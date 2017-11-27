#include "SkynetTerrainAnalyser.h"

#include "Types.h"
#include "Heap.h"
#include "MapUtil.h"
#include "UnitTracker.h"
#include "DrawBuffer.h"
#include "Window.h"

#include <thread>
#include <atomic>
#include <queue>
#include <array>
#include <fstream>
#include <filesystem>

// Increment if anything changes in the algorithm, or in the file format
const unsigned int data_version_number = 6;

SkynetTerrainAnalyser::SkynetTerrainAnalyser( Core & core )
	: TerrainAnalyserInterface( core )
	, MessageListener<UnitDiscover, UnitMorphRenegade, UnitDestroy>( getUnitTracker() )
	, m_map_size( BWAPI::Broodwar->mapWidth() * 4, BWAPI::Broodwar->mapHeight() * 4 )
{
	m_processed_static_data.m_map_size = m_map_size;
	m_processed_dynamic_data.m_map_size = m_map_size;
	m_processing_dynamic_data.m_map_size = m_map_size;

	core.registerUpdateProcess( 1.0f, [this]() { update(); } );
}

void SkynetTerrainAnalyser::update()
{
	if( !m_processed_static_data.m_analysed )
	{
		if( (isDebugging( Debug::RegionAnalysis ) && isDebugging( Debug::StaticData )) || !tryLoadData( m_processed_static_data ) )
		{
			m_processed_static_data.process( getResources(), isDebugging( Debug::RegionAnalysis ) && isDebugging( Debug::StaticData ) );
			saveData( m_processed_static_data );
		}

		if( !tryLoadData( m_processed_dynamic_data ) )
		{
			m_processed_dynamic_data.process( getResources(), false );
		}

		postMessage<StaticTerrainAnalysed>();
		postMessage<DynamicTerrainAnalysed>();
	}

	if( isDebugging( Debug::Default ) && BWAPI::Broodwar->getKeyState( BWAPI::Key( '0' ) ) )
		++m_reprocess_request;

	//checkDynamicData();

	if( isDebugging( Debug::Default ) )
	{
		bool debugging_static_data = isDebugging( Debug::StaticData );
		auto & data = debugging_static_data ? m_processed_static_data : m_processed_dynamic_data;

		bool show_region = BWAPI::Broodwar->getKeyState( BWAPI::Key( '1' ) ) && data.m_analysed;
		bool show_connectivity = BWAPI::Broodwar->getKeyState( BWAPI::Key( '2' ) ) && data.m_analysed;
		bool show_clearance = BWAPI::Broodwar->getKeyState( BWAPI::Key( '3' ) ) && data.m_analysed;
		bool show_regions = BWAPI::Broodwar->getKeyState( BWAPI::Key( '4' ) );
		bool show_chokepoints = BWAPI::Broodwar->getKeyState( BWAPI::Key( '5' ) );
		bool show_base_locations = BWAPI::Broodwar->getKeyState( BWAPI::Key( '6' ) );

		WalkPosition mouse_tile( BWAPI::Broodwar->getMousePosition() + BWAPI::Broodwar->getScreenPosition() );
		if( mouse_tile.isValid() )
		{
			if( show_region )
			{
				Region region = getRegion( mouse_tile, debugging_static_data );
				if( region )
				{
					WalkPosition top_left( BWAPI::Broodwar->getScreenPosition() );
					WalkPosition bottom_right( top_left + WalkPosition( Position( 640, 480 ) ) );
					if( bottom_right.x > m_map_size.x ) bottom_right.x = m_map_size.x;
					if( bottom_right.y > m_map_size.y ) bottom_right.y = m_map_size.y;

					MapUtil::forEachPosition( top_left, bottom_right, [this, region, debugging_static_data]( WalkPosition pos )
					{
						if( getRegion( pos, debugging_static_data ) == region )
							BWAPI::Broodwar->drawBoxMap( pos.x * 8, pos.y * 8, pos.x * 8 + 7, pos.y * 8 + 7, Colors::Red );
					} );
				}
			}
			else if( show_connectivity )
			{
				int current_connectivity = data.m_tile_connectivity[mouse_tile];

				WalkPosition top_left( BWAPI::Broodwar->getScreenPosition() );
				WalkPosition bottom_right( top_left + WalkPosition( Position( 640, 480 ) ) );
				if( bottom_right.x > m_map_size.x ) bottom_right.x = m_map_size.x;
				if( bottom_right.y > m_map_size.y ) bottom_right.y = m_map_size.y;

				MapUtil::forEachPosition( top_left, bottom_right, [&data, current_connectivity]( WalkPosition pos )
				{
					if( data.m_tile_connectivity[pos] == current_connectivity )
						BWAPI::Broodwar->drawBoxMap( pos.x * 8, pos.y * 8, pos.x * 8 + 7, pos.y * 8 + 7, Colors::Red );
				} );

				if( data.m_connectivity_to_small_obstacles[current_connectivity] )
					BWAPI::Broodwar->drawTextMouse( 8, -8, "Small Obstacle" );
			}

			if( show_clearance )
				BWAPI::Broodwar->drawCircleMap( Position( mouse_tile ), data.m_tile_clearance[mouse_tile], Colors::Red );
		}

		if( show_regions )
		{
			for( auto region : data.m_regions )
				region->draw( Colors::Blue );
		}

		if( show_chokepoints )
		{
			for( auto chokepoint : data.m_chokepoints )
				chokepoint->draw( Colors::Red );
		}

		if( show_base_locations )
		{
			for( auto base_location : data.m_base_locations )
				base_location->draw( Colors::Green );
		}

		BWAPI::Broodwar->drawTextScreen( 5, 5, "Terrain Analysis Walkability: %.2f", data.m_walkability_time_seconds );
		BWAPI::Broodwar->drawTextScreen( 5, 15, "Terrain Analysis Connectivity: %.2f", data.m_connectivity_time_seconds );
		BWAPI::Broodwar->drawTextScreen( 5, 25, "Terrain Analysis Clearance: %.2f", data.m_clearance_time_seconds );
		BWAPI::Broodwar->drawTextScreen( 5, 35, "Terrain Analysis Regions: %.2f", data.m_regions_time_seconds );

		if( m_async_future.valid() )
			BWAPI::Broodwar->drawTextScreen( 5, 45, "Analysing Terrain" );
	}
}

std::pair<Chokepoint, Chokepoint> SkynetTerrainAnalyser::getTravelChokepoints( Region start, Region end, bool static_data ) const
{
	std::pair<Chokepoint, Chokepoint> current_shortest{ nullptr, nullptr };
	if( start == end )
		return current_shortest;

	int shortest_distance = max_distance;

	auto & data = static_data ? m_processed_static_data : m_processed_dynamic_data;

	if( start->getConnectivity() == end->getConnectivity() )
	{
		for( Chokepoint start_chokepoint : start->getChokepoints() )
		{
			for( Chokepoint end_chokepoint : end->getChokepoints() )
			{
				int distance_between_chokepoints = data.m_chokepoint_distances[Position( start_chokepoint->getID(), end_chokepoint->getID() )];
				if( distance_between_chokepoints == max_distance )
					continue;

				if( distance_between_chokepoints < shortest_distance )
				{
					shortest_distance = distance_between_chokepoints;
					current_shortest.first = start_chokepoint;
					current_shortest.second = end_chokepoint;
				}
			}
		}
	}

	return current_shortest;
}

int SkynetTerrainAnalyser::getGroundDistance( WalkPosition start, WalkPosition end, bool static_data ) const
{
	Region start_region = getRegion( start, static_data );
	Region end_region = getRegion( end, static_data );

	if( !start_region || !end_region )
	{
		BWAPI::Broodwar->setLocalSpeed( -1 );

		if( !start_region )
		{
			getDrawBuffer().drawMap<BufferedBox>( max_time, Position( start ), Position( start ) + Position( 7, 7 ), Colors::Red );
			getDrawBuffer().drawMap<BufferedBox>( max_time, Position( end ), Position( end ) + Position( 7, 7 ), Colors::Green );
			getDrawBuffer().drawMap<BufferedLine>( max_time, Position( start ), Position( end ), Colors::Red );
		}
		else
		{
			getDrawBuffer().drawMap<BufferedBox>( max_time, Position( start ), Position( start ) + Position( 7, 7 ), Colors::Green );
			getDrawBuffer().drawMap<BufferedBox>( max_time, Position( end ), Position( end ) + Position( 7, 7 ), Colors::Red );
			getDrawBuffer().drawMap<BufferedLine>( max_time, Position( start ), Position( end ), Colors::Red );
		}

		return start.getApproxDistance( end );
	}

	if( start_region == end_region )
		return start.getApproxDistance( end );

	int shortest_distance = max_distance;

	auto & data = static_data ? m_processed_static_data : m_processed_dynamic_data;

	if( start_region->getConnectivity() == end_region->getConnectivity() )
	{
		for( Chokepoint start_chokepoint : start_region->getChokepoints() )
		{
			int distance_to_start_chokepoint = start.getApproxDistance( start_chokepoint->getCenter() );

			for( Chokepoint end_chokepoint : end_region->getChokepoints() )
			{
				int distance_between_chokepoints = data.m_chokepoint_distances[Position( start_chokepoint->getID(), end_chokepoint->getID() )];
				if( distance_between_chokepoints == max_distance )
					continue;

				int distance_to_end_chokepoint = end.getApproxDistance( end_chokepoint->getCenter() );

				int total_distance = distance_to_start_chokepoint + distance_between_chokepoints + distance_to_end_chokepoint;
				if( total_distance < shortest_distance )
				{
					shortest_distance = total_distance;
				}
			}
		}
	}

	return shortest_distance;
}

void SkynetTerrainAnalyser::Data::process( UnitGroup resources, bool debug_region )
{
	calculateWalkability();
	calculateConnectivity();
	calculateClearance();
	calculateRegions( debug_region );

	if( !resources.empty() )
	{
		createBases( resources );
	}

	m_analysed = true;
}

void SkynetTerrainAnalyser::Data::calculateWalkability()
{
	auto start_time = std::chrono::system_clock::now();

	m_tile_clearance.resize( m_map_size.x, m_map_size.y, m_map_size.x + m_map_size.y );
	m_tile_to_closest_obstacle.resize( m_map_size.x, m_map_size.y );

	for( int x = 0; x < m_map_size.x; ++x )
	{
		for( int y = 0; y < m_map_size.y; ++y )
		{
			WalkPosition pos( x, y );

			if( !BWAPI::Broodwar->isWalkable( pos ) )
			{
				m_tile_clearance[pos] = 0;
				m_tile_to_closest_obstacle[pos] = pos;
			}
			else if( x == 0 || y == 0 || x == m_map_size.x - 1 || y == m_map_size.y - 1 )
			{
				m_tile_clearance[pos] = 10;

				m_tile_to_closest_obstacle[pos] = WalkPosition(
					(x == 0 ? -1 : (x == m_map_size.x - 1 ? m_map_size.x : x)),
					(y == 0 ? -1 : (y == m_map_size.y - 1 ? m_map_size.y : y)) );
			}
		}
	}

	for( auto & building : m_buildings_cache )
	{
		UnitType unit_type = building.first;
		TilePosition tile_position = building.second;

		Position position = Position( tile_position ) + Position( unit_type.tileSize() ) / 2;

		int top = (position.y - unit_type.dimensionUp()) / WALKPOSITION_SCALE;
		int left = (position.x - unit_type.dimensionLeft()) / WALKPOSITION_SCALE;
		int bottom = (position.y + unit_type.dimensionDown()) / WALKPOSITION_SCALE;
		int right = (position.x + unit_type.dimensionRight()) / WALKPOSITION_SCALE;

		for( int x = left; x < right; ++x )
		{
			for( int y = top; y < bottom; ++y )
			{
				WalkPosition pos( x, y );

				m_tile_clearance[pos] = 0;
				m_tile_to_closest_obstacle[pos] = pos;
			}
		}
	}

	std::chrono::duration<float> elapsed_seconds = std::chrono::system_clock::now() - start_time;
	m_walkability_time_seconds = elapsed_seconds.count();
}

void SkynetTerrainAnalyser::Data::calculateConnectivity()
{
	auto start_time = std::chrono::system_clock::now();

	m_tile_connectivity.resize( m_map_size.x, m_map_size.y, -1 );

	std::atomic<int> connectivity_counter( 0 );

	auto calculate_connectivity = [this, &connectivity_counter]( bool walkable_tiles )
	{
		for( int x = 0; x < m_map_size.x; ++x )
		{
			for( int y = 0; y < m_map_size.y; ++y )
			{
				WalkPosition pos( x, y );

				if( m_tile_connectivity[pos] != -1 ) continue;
				if( (m_tile_clearance[pos] == 0) != walkable_tiles ) continue;

				int region_area = 0;
				int region_connectivity = connectivity_counter++;
				bool touches_edge = false;

				m_tile_connectivity[pos] = region_connectivity;

				std::queue<WalkPosition> unvisited_tiles;
				unvisited_tiles.push( pos );

				while( !unvisited_tiles.empty() )
				{
					const WalkPosition &tile = unvisited_tiles.front();

					++region_area;

					WalkPosition pos_north( tile.x, tile.y - 1 );
					if( tile.y <= 0 ) touches_edge = true;
					else if( (m_tile_clearance[pos_north] == 0) == walkable_tiles && m_tile_connectivity[pos_north] == -1 )
					{
						unvisited_tiles.push( pos_north );
						m_tile_connectivity[pos_north] = region_connectivity;
					}

					WalkPosition pos_east( tile.x + 1, tile.y );
					if( tile.x >= m_map_size.x - 1 ) touches_edge = true;
					else if( (m_tile_clearance[pos_east] == 0) == walkable_tiles && m_tile_connectivity[pos_east] == -1 )
					{
						unvisited_tiles.push( pos_east );
						m_tile_connectivity[pos_east] = region_connectivity;
					}

					WalkPosition pos_south( tile.x, tile.y + 1 );
					if( tile.y >= m_map_size.y - 1 ) touches_edge = true;
					else if( (m_tile_clearance[pos_south] == 0) == walkable_tiles && m_tile_connectivity[pos_south] == -1 )
					{
						unvisited_tiles.push( pos_south );
						m_tile_connectivity[pos_south] = region_connectivity;
					}

					WalkPosition pos_west( tile.x - 1, tile.y );
					if( tile.x <= 0 ) touches_edge = true;
					else if( (m_tile_clearance[pos_west] == 0) == walkable_tiles && m_tile_connectivity[pos_west] == -1 )
					{
						unvisited_tiles.push( pos_west );
						m_tile_connectivity[pos_west] = region_connectivity;
					}

					unvisited_tiles.pop();
				}

				if( !walkable_tiles )
				{
					m_connectivity_to_small_obstacles.resize( region_connectivity + 1, false );
					m_connectivity_to_small_obstacles[region_connectivity] = region_area < 200 && !touches_edge;
				}
			}
		}
	};

	std::thread thread( calculate_connectivity, true );
	calculate_connectivity( false );
	thread.join();

	m_connectivity_to_small_obstacles.resize( connectivity_counter + 1, false );

	std::chrono::duration<float> elapsed_seconds = std::chrono::system_clock::now() - start_time;
	m_connectivity_time_seconds = elapsed_seconds.count();
}

void SkynetTerrainAnalyser::Data::calculateClearance()
{
	auto start_time = std::chrono::system_clock::now();

	std::queue<std::pair<WalkPosition, int>> queued_tiles;

	for( int x = 0; x < m_map_size.x; ++x )
	{
		for( int y = 0; y < m_map_size.y; ++y )
		{
			WalkPosition pos( x, y );
			int clearance = m_tile_clearance[pos];

			if( clearance <= 10 )
			{
				if( !m_connectivity_to_small_obstacles[m_tile_connectivity[pos]] )
					queued_tiles.emplace( pos, clearance );
			}
		}
	}

	auto visit_direction = [this, &queued_tiles]( int distance, WalkPosition current_obstacle, WalkPosition next_pos )
	{
		if( distance < m_tile_clearance[next_pos] )
		{
			m_tile_clearance[next_pos] = distance;
			m_tile_to_closest_obstacle[next_pos] = current_obstacle;
			queued_tiles.emplace( next_pos, distance );
		}
	};

	while( !queued_tiles.empty() )
	{
		const WalkPosition current_tile = queued_tiles.front().first;
		const int next_distance = queued_tiles.front().second + 10;
		const int next_distance_diag = next_distance + 4;
		queued_tiles.pop();

		const WalkPosition current_obstacle = m_tile_to_closest_obstacle[current_tile];

		const int west = current_tile.x - 1;
		const int north = current_tile.y - 1;
		const int east = current_tile.x + 1;
		const int south = current_tile.y + 1;

		const bool can_go_west = west >= 0;
		const bool can_go_north = north >= 0;
		const bool can_go_east = east < m_map_size.x;
		const bool can_go_south = south < m_map_size.y;

		if( can_go_west )
			visit_direction( next_distance, current_obstacle, WalkPosition( west, current_tile.y ) );

		if( can_go_north )
			visit_direction( next_distance, current_obstacle, WalkPosition( current_tile.x, north ) );

		if( can_go_east )
			visit_direction( next_distance, current_obstacle, WalkPosition( east, current_tile.y ) );

		if( can_go_south )
			visit_direction( next_distance, current_obstacle, WalkPosition( current_tile.x, south ) );

		if( can_go_west && can_go_north )
			visit_direction( next_distance_diag, current_obstacle, WalkPosition( west, north ) );

		if( can_go_east && can_go_south )
			visit_direction( next_distance_diag, current_obstacle, WalkPosition( east, south ) );

		if( can_go_east && can_go_north )
			visit_direction( next_distance_diag, current_obstacle, WalkPosition( east, north ) );

		if( can_go_west && can_go_south )
			visit_direction( next_distance_diag, current_obstacle, WalkPosition( west, south ) );
	}

	std::chrono::duration<float> elapsed_seconds = std::chrono::system_clock::now() - start_time;
	m_clearance_time_seconds = elapsed_seconds.count();
}

struct RegionTileData
{
	WalkPosition last_minima = WalkPositions::None;
	unsigned int children_flags = 0;

	void set_child( int i )
	{
		children_flags |= 1 << i;
	}

	bool has_child( int i ) const
	{
		return (children_flags >> i) & 1;
	}
};

void SkynetTerrainAnalyser::Data::calculateRegions( bool debug_region )
{
	auto start_time = std::chrono::system_clock::now();

	static std::unique_ptr<Window> debug_window;
	if( debug_region )
	{
		debug_window = std::make_unique<Window>( "Calculating Regions", m_map_size.x, m_map_size.y );

		for( int x = 0; x < m_map_size.x; ++x )
		{
			for( int y = 0; y < m_map_size.y; ++y )
			{
				if( m_tile_clearance[WalkPosition( x, y )] == 0 )
				{
					if( m_connectivity_to_small_obstacles[m_tile_connectivity[WalkPosition( x, y )]] )
						debug_window->addBox( x, y, x + 1, y + 1, Colors::Blue );
					else
						debug_window->addBox( x, y, x + 1, y + 1, Colors::Grey );
				}
			}
		}
	}

	m_tile_to_region.resize( m_map_size.x, m_map_size.y, nullptr );

	RectangleArray<RegionTileData, WALKPOSITION_SCALE> tile_data( m_map_size.x, m_map_size.y );
	RectangleArray<SkynetChokepoint *, WALKPOSITION_SCALE> tile_to_chokepoint( m_map_size.x, m_map_size.y, nullptr );
	std::vector<std::unique_ptr<SkynetChokepoint>> narrow_chokepoints;

	auto comp = []( const std::pair<WalkPosition, int> & first, const std::pair<WalkPosition, int> & second )
	{
		return first.second < second.second;
	};

	std::priority_queue<std::pair<WalkPosition, int>, std::vector<std::pair<WalkPosition, int>>, decltype(comp)> unvisited_tiles( comp );

	while( true )
	{
		int current_region_clearance = 0;
		WalkPosition current_region_tile;

		// Find the tile with the largest clearance to start from
		for( int x = 0; x < m_map_size.x; ++x )
		{
			for( int y = 0; y < m_map_size.y; ++y )
			{
				WalkPosition pos( x, y );

				if( m_tile_to_region[pos] || tile_to_chokepoint[pos] )
					continue;

				const int local_maxima_value = m_tile_clearance[pos];
				if( local_maxima_value > current_region_clearance )
				{
					current_region_clearance = local_maxima_value;
					current_region_tile = pos;
				}
			}
		}

		// No tile found, calculation is complete
		if( current_region_clearance == 0 )
			break;

		tile_data.fill();

		// This will be the start of our region
		int region_id = m_region_storage.size();
		m_region_storage.emplace_back( std::make_unique<SkynetRegion>( region_id, current_region_tile, current_region_clearance, m_tile_connectivity[current_region_tile] ) );
		SkynetRegion *current_region = m_region_storage.back().get();
		m_regions.push_back( current_region );

		// Start the algorithm from this tile
		unvisited_tiles.emplace( current_region_tile, current_region_clearance );
		tile_data[current_region_tile].last_minima = current_region_tile;

		// Count the number of tiles we touch along the way
		int region_size = 0;

		// While we still have tiles to visit
		while( !unvisited_tiles.empty() )
		{
			WalkPosition current_tile = unvisited_tiles.top().first;
			int current_tile_clearance = unvisited_tiles.top().second;
			unvisited_tiles.pop();

			if( tile_data[current_tile].last_minima == WalkPositions::None )
				continue;

			// If this tile belongs to an existing chokepoint
			auto existing_chokepoint = tile_to_chokepoint[current_tile];
			if( existing_chokepoint )
			{
				// Chokepoint already has a second region
				if( existing_chokepoint->getRegions().second )
				{
					// It is not the current region, this shouldn't happen
					if( existing_chokepoint->getRegions().second != current_region )
					{
						//LOGMESSAGEWARNING("Touched a choke saved to anouther region");

						if( debug_window )
						{
							debug_window->addBox( current_tile.x, current_tile.y, current_tile.x + 1, current_tile.y + 1, Colors::Red );
						}
					}
				}
				// This chokepoint wasn't created by the current region, add it as the other side
				else if( existing_chokepoint->getRegions().first != current_region )
				{
					if( existing_chokepoint->getID() >= 0 )
						current_region->addChokepoint( existing_chokepoint );

					existing_chokepoint->setRegion2( current_region );
				}

				continue;
			}

			if( !m_tile_to_region[current_tile] )
			{
				++region_size;
				m_tile_to_region[current_tile] = current_region;
			}
			else if( m_tile_to_region[current_tile] != current_region )
			{
				//LOGMESSAGEWARNING("2 regions possibly connected without a choke");
				if( debug_window )
				{
					debug_window->addBox( current_tile.x, current_tile.y, current_tile.x + 1, current_tile.y + 1, Colors::Red );
				}

				continue;
			}

			WalkPosition last_minima = tile_data[current_tile].last_minima;
			const int last_minima_size = m_tile_clearance[last_minima];

			// Test change in clearance between current position and region position
			bool found_chokepoint = false;
			if( last_minima_size < int( float( current_region_clearance )*0.90f ) && last_minima_size < int( float( current_tile_clearance )*0.80f ) )
				found_chokepoint = true;

			// TODO: else if the terrain height has changed, tweak the above values

			if( found_chokepoint )
			{
				const int min_distance = 32;
				if( (abs( current_region_tile.x - last_minima.x ) + abs( current_region_tile.y - last_minima.y )) < min_distance )
					found_chokepoint = false;
				else if( (abs( current_tile.x - last_minima.x ) + abs( current_tile.y - last_minima.y )) < min_distance )
					found_chokepoint = false;
				else if( current_tile_clearance < 120 )
					found_chokepoint = false;
			}

			// A chokepoint has been found
			if( found_chokepoint )
			{
				// Find the 2 positions either side of this new chokepoint
				std::pair<WalkPosition, WalkPosition> choke_sides = findChokePoint( last_minima );

				// Create the chokepoint
				SkynetChokepoint *current_chokepoint = nullptr;

				// Don't create permanent chokepoints if they are too narrow and cannot be moved through by any unit
				bool is_narrow_chokepoint = last_minima_size < 22;
				if( !is_narrow_chokepoint )
				{
					int chokepoint_id = m_chokepoint_storage.size();
					m_chokepoint_storage.emplace_back( std::make_unique<SkynetChokepoint>( chokepoint_id, last_minima, choke_sides.first, choke_sides.second, last_minima_size ) );
					current_chokepoint = m_chokepoint_storage.back().get();
					m_chokepoints.push_back( current_chokepoint );

					current_region->addChokepoint( current_chokepoint );
				}
				else
				{
					narrow_chokepoints.emplace_back( std::make_unique<SkynetChokepoint>( -1, last_minima, choke_sides.first, choke_sides.second, last_minima_size ) );
					current_chokepoint = narrow_chokepoints.back().get();
				}

				// The first region is the region that created it
				current_chokepoint->setRegion1( current_region );

				// For every tile between the chokepoint sides
				std::queue<WalkPosition> choke_children;
				auto add_choke_children = [this, &region_size, &tile_to_chokepoint, &choke_children, &current_region, &current_chokepoint, is_narrow_chokepoint]( WalkPosition line_pos )
				{
					if( line_pos.x >= 0 && line_pos.y >= 0 && line_pos.x < m_map_size.x && line_pos.y < m_map_size.y && m_tile_clearance[line_pos] != 0 )
					{
						++region_size;
						tile_to_chokepoint[line_pos] = current_chokepoint;
						m_tile_to_region[line_pos] = current_region;
						choke_children.push( line_pos );

						if( debug_window )
						{
							debug_window->addBox( line_pos.x, line_pos.y, line_pos.x + 1, line_pos.y + 1, is_narrow_chokepoint ? Colors::Purple : Colors::Orange );
						}
					}

					return false;
				};

				MapUtil::forEachPositionInLine( choke_sides.first, last_minima, add_choke_children );
				MapUtil::forEachPositionInLine( last_minima, choke_sides.second, add_choke_children );

				// Remove any tiles that are after the chokepoint as they are now cut off
				while( !choke_children.empty() )
				{
					--region_size;
					auto current_tile = choke_children.front();
					choke_children.pop();

					if( tile_to_chokepoint[current_tile] != current_chokepoint )
					{
						m_tile_to_region[current_tile] = nullptr;
						tile_data[current_tile].last_minima = WalkPositions::None;
					}

					for( int i = 0; i < 4; ++i )
					{
						if( tile_data[current_tile].has_child( i ) )
						{
							WalkPosition next_tile(
								(i == 0 ? current_tile.x - 1 : (i == 1 ? current_tile.x + 1 : current_tile.x)),
								(i == 2 ? current_tile.y - 1 : (i == 3 ? current_tile.y + 1 : current_tile.y)) );
							choke_children.push( next_tile );
						}
					}

					tile_data[current_tile].children_flags = 0;
				}
			}
			// A chokepoint was not found for this current position
			else
			{
				// Set this tile as the minima if it is smaller
				if( m_tile_clearance[current_tile] < last_minima_size )
					last_minima = current_tile;

				// For every adjacent tile
				for( int i = 0; i < 4; ++i )
				{
					WalkPosition next_tile(
						(i == 0 ? current_tile.x - 1 : (i == 1 ? current_tile.x + 1 : current_tile.x)),
						(i == 2 ? current_tile.y - 1 : (i == 3 ? current_tile.y + 1 : current_tile.y)) );

					// If it's within the bounds of the map
					if( next_tile.x < 0 || next_tile.y < 0 || next_tile.x >= m_map_size.x || next_tile.y >= m_map_size.y )
						continue;

					// If it is not an edge
					if( m_tile_clearance[next_tile] == 0 )
						continue;

					if( tile_data[next_tile].last_minima == WalkPositions::None )
					{
						tile_data[next_tile].last_minima = last_minima;
						tile_data[current_tile].set_child( i );

						unvisited_tiles.emplace( next_tile, m_tile_clearance[next_tile] );
					}
				}
			}
		}

		current_region->setSize( region_size );
	}

	// It's possible for a chokepoint to not have a second region if the same region ended up on it's other side, delete these chokepoints
	m_chokepoint_storage.erase( std::remove_if( m_chokepoint_storage.begin(), m_chokepoint_storage.end(), [this]( auto & test_chokepoint )
	{
		if( !test_chokepoint->getRegions().second )
		{
			static_cast<SkynetRegion*>(test_chokepoint->getRegions().first)->removeChokepoint( test_chokepoint.get() );
			m_chokepoints.erase( std::find( m_chokepoints.begin(), m_chokepoints.end(), test_chokepoint.get() ) );
			return true;
		}

		return false;
	} ), m_chokepoint_storage.end() );

	// Set final ids now that the dead chokepoints
	int id = 0;
	for( auto & chokepoint : m_chokepoint_storage )
	{
		chokepoint->setID( id++ );
	}

	// Calculate the ground distance for traveling between chokepoints
	m_chokepoint_distances.resize( m_chokepoints.size(), m_chokepoints.size(), max_distance );

	for( Chokepoint start_chokepoint : m_chokepoints )
	{
		for( Chokepoint end_chokepoint : m_chokepoints )
		{
			Position distance_position{ start_chokepoint->getID(), end_chokepoint->getID() };

			if( start_chokepoint == end_chokepoint )
				m_chokepoint_distances[distance_position] = 0;
			else if( start_chokepoint->getRegions().first->getConnectivity() != end_chokepoint->getRegions().first->getConnectivity() )
				continue;
			else if( start_chokepoint->getRegions().first == end_chokepoint->getRegions().first
				|| start_chokepoint->getRegions().first == end_chokepoint->getRegions().second
				|| start_chokepoint->getRegions().second == end_chokepoint->getRegions().first
				|| start_chokepoint->getRegions().second == end_chokepoint->getRegions().second )
				m_chokepoint_distances[distance_position] = start_chokepoint->getCenter().getApproxDistance( end_chokepoint->getCenter() );
			else
				m_chokepoint_distances[distance_position] = calculateShortestDistance( start_chokepoint, end_chokepoint );
		}
	}

	std::chrono::duration<float> elapsed_seconds = std::chrono::system_clock::now() - start_time;
	m_regions_time_seconds = elapsed_seconds.count();
}

std::pair<WalkPosition, WalkPosition> SkynetTerrainAnalyser::Data::findChokePoint( WalkPosition center ) const
{
	const WalkPosition side1 = m_tile_to_closest_obstacle[center];

	if( side1 == center )
		return std::make_pair( side1, side1 );

	WalkPositionFloat side_1_direction( side1 - center );
	normalise( side_1_direction );

	int x0 = side1.x;
	int y0 = side1.y;

	int x1 = center.x;
	int y1 = center.y;

	int dx = abs( x1 - x0 );
	int dy = abs( y1 - y0 );

	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;

	x0 = x1;
	y0 = y1;

	int error = dx - dy;

	while( true )
	{
		WalkPosition pos( x0, y0 );
		if( x0 < 0 || y0 < 0 || x0 >= m_map_size.x || y0 >= m_map_size.y || m_tile_clearance[pos] == 0 )
			return std::make_pair( side1, pos );

		WalkPosition side2 = m_tile_to_closest_obstacle[pos];

		WalkPositionFloat side_2_direction( side2 - center );
		normalise( side_2_direction );

		float dot = dotProduct( side_2_direction, side_1_direction );
		float angle = acos( dot );
		if( angle > 2.0f )
			return std::make_pair( side1, side2 );

		int e2 = error * 2;
		if( e2 > -dy )
		{
			error -= dy;
			x0 += sx;
		}
		if( e2 < dx )
		{
			error += dx;
			y0 += sy;
		}
	}
}

int SkynetTerrainAnalyser::Data::calculateShortestDistance( Chokepoint start_chokepoint, Chokepoint end_chokepoint ) const
{
	Heap<Chokepoint, int> open_chokepoints{ true };
	std::set<Chokepoint> closed_chokepoints;
	std::map<Chokepoint, int> chokepoint_distances;

	open_chokepoints.push( std::make_pair( start_chokepoint, 0 ) );

	while( !open_chokepoints.empty() )
	{
		Chokepoint current_chokepoint = open_chokepoints.top().first;
		int current_distance = open_chokepoints.top().second;

		if( current_chokepoint == end_chokepoint )
			return current_distance;

		chokepoint_distances[current_chokepoint] = current_distance;

		open_chokepoints.pop();
		closed_chokepoints.insert( current_chokepoint );

		for( Chokepoint connected_chokepoint : current_chokepoint->getRegions().first->getChokepoints() )
		{
			if( current_chokepoint == connected_chokepoint || closed_chokepoints.count( connected_chokepoint ) > 0 )
				continue;

			int connection_distance = current_distance + current_chokepoint->getCenter().getApproxDistance( connected_chokepoint->getCenter() );

			if( !open_chokepoints.contains( connected_chokepoint ) )
			{
				open_chokepoints.push( std::make_pair( connected_chokepoint, connection_distance ) );
			}
			else if( open_chokepoints.get( connected_chokepoint ) > connection_distance )
			{
				open_chokepoints.set( connected_chokepoint, connection_distance );
			}
		}

		for( Chokepoint connected_chokepoint : current_chokepoint->getRegions().second->getChokepoints() )
		{
			if( current_chokepoint == connected_chokepoint || closed_chokepoints.count( connected_chokepoint ) > 0 )
				continue;

			int connection_distance = current_distance + current_chokepoint->getCenter().getApproxDistance( connected_chokepoint->getCenter() );

			if( !open_chokepoints.contains( connected_chokepoint ) )
			{
				open_chokepoints.push( std::make_pair( connected_chokepoint, connection_distance ) );
			}
			else if( open_chokepoints.get( connected_chokepoint ) > connection_distance )
			{
				open_chokepoints.set( connected_chokepoint, connection_distance );
			}
		}
	}

	return max_distance;
}

void SkynetTerrainAnalyser::Data::createBases( const UnitGroup & resources )
{
	//Group them into clusters
	std::vector<UnitGroup> resource_clusters = resources.getClusters( 260, 3 );

	for( const UnitGroup &resource_cluster : resource_clusters )
	{
		TilePosition base_location = TilePositions::Invalid;
		int best_rating = m_map_size.x * m_map_size.y;
		MapUtil::spiralSearch( TilePosition( resource_cluster.getCenter() ), [&resource_cluster, &base_location, best_rating]( TilePosition location ) mutable -> bool
		{
			for( int x = location.x; x < location.x + UnitTypes::Protoss_Nexus.tileWidth(); ++x )
			{
				for( int y = location.y; y < location.y + UnitTypes::Protoss_Nexus.tileHeight(); ++y )
				{
					if( x < 0 && y < 0 && x >= BWAPI::Broodwar->mapWidth() && y >= BWAPI::Broodwar->mapHeight() )
						return false;

					if( !BWAPI::Broodwar->isBuildable( x, y ) )
						return false;
				}
			}

			int distance_to_resources = 0;
			for( Unit resource : resource_cluster )
			{
				const UnitType &resource_type = resource->getType();
				const TilePosition &resource_tile_position = resource->getTilePosition();

				if( resource_tile_position.x > location.x - (resource_type == UnitTypes::Resource_Mineral_Field ? 5 : 7) &&
					resource_tile_position.y > location.y - (resource_type == UnitTypes::Resource_Mineral_Field ? 4 : 5) &&
					resource_tile_position.x < location.x + 7 &&
					resource_tile_position.y < location.y + 6 )
				{
					return false;
				}

				const Position &resource_position = resource->getPosition();

				int tx = location.x * 32 + 64;
				int ty = location.y * 32 + 48;

				int u_left = resource_position.x - resource_type.dimensionLeft();
				int u_top = resource_position.y - resource_type.dimensionUp();
				int u_right = resource_position.x + resource_type.dimensionRight() + 1;
				int u_bottom = resource_position.y + resource_type.dimensionDown() + 1;

				int targ_left = tx - UnitTypes::Protoss_Nexus.dimensionLeft();
				int targ_top = ty - UnitTypes::Protoss_Nexus.dimensionUp();
				int targ_right = tx + UnitTypes::Protoss_Nexus.dimensionRight() + 1;
				int targ_bottom = ty + UnitTypes::Protoss_Nexus.dimensionDown() + 1;

				int x_dist = u_left - targ_right;
				if( x_dist < 0 )
				{
					x_dist = targ_left - u_right;
					if( x_dist < 0 )
						x_dist = 0;
				}

				int y_dist = u_top - targ_bottom;
				if( y_dist < 0 )
				{
					y_dist = targ_top - u_bottom;
					if( y_dist < 0 )
						y_dist = 0;
				}

				distance_to_resources += Position( 0, 0 ).getApproxDistance( Position( x_dist, y_dist ) );
			}

			if( distance_to_resources < best_rating )
			{
				best_rating = distance_to_resources;
				base_location = location;
			}

			return false;
		}, 18 );

		if( !base_location.isValid() )
		{
			continue;
		}

		bool added_to_other = false;
		for( auto &other_base_location : m_base_location_storage )
		{
			int dx = abs( base_location.x - other_base_location->getBuildLocation().x );
			int dy = abs( base_location.y - other_base_location->getBuildLocation().y );

			if( dx <= 4 && dy <= 3 )
			{
				other_base_location->addResources( resource_cluster );
				added_to_other = true;
				break;
			}
		}

		if( !added_to_other )
		{
			SkynetRegion *base_region = m_tile_to_region[WalkPosition( base_location )];
			m_base_location_storage.emplace_back( std::make_unique<SkynetBaseLocation>( base_location, base_region, resource_cluster ) );
			m_base_locations.push_back( m_base_location_storage.back().get() );
			base_region->addBase( m_base_locations.back() );
		}
	}
}

UnitGroup SkynetTerrainAnalyser::getResources()
{
	UnitGroup resources = getUnitTracker().getGeysers();
	for( Unit mineral : getUnitTracker().getMinerals() )
	{
		if( mineral->getResources() > 200 )
			resources.insert( mineral, true );
	}

	return resources;
}

void SkynetTerrainAnalyser::checkDynamicData()
{
	m_old_regions.clear();
	m_old_chokepoints.clear();
	m_old_base_locations.clear();

	if( m_async_future.valid() )
	{
		if( m_async_future.wait_for( std::chrono::milliseconds( 2 ) ) == std::future_status::ready )
		{
			m_old_regions = std::move( m_processed_dynamic_data.m_region_storage );
			for( auto & region : m_old_regions ) region->markInvalid();

			m_old_chokepoints = std::move( m_processed_dynamic_data.m_chokepoint_storage );
			for( auto & chokepoint : m_old_chokepoints ) chokepoint->markInvalid();

			m_old_base_locations = std::move( m_processed_dynamic_data.m_base_location_storage );
			for( auto & base_location : m_old_base_locations ) base_location->markInvalid();

			m_async_future.get();

			m_processed_dynamic_data = std::move( m_processing_dynamic_data );
			postMessage<DynamicTerrainAnalysed>();
		}
	}

	if( !m_async_future.valid() && m_processed_dynamic_data.m_request < m_reprocess_request )
	{
		m_processing_dynamic_data.m_buildings_cache.clear();

		for( Unit building : m_buildings )
		{
			if( !building->isLifted() )
				m_processing_dynamic_data.m_buildings_cache.emplace_back( building->getType(), building->getTilePosition() );
		}

		m_processing_dynamic_data.m_request = m_reprocess_request;

		m_async_future = std::async( std::launch::async, [this]()
		{
			m_processing_dynamic_data.process( UnitGroup(), isDebugging( Debug::RegionAnalysis ) && !isDebugging( Debug::StaticData ) );
		} );
	}
}

template <typename T>
void writeData( std::ofstream & file, T value )
{
	file.write( (const char *) &value, sizeof( T ) );
}

template <typename T>
T readData( std::ifstream & file )
{
	T value;
	file.read( (char *) &value, sizeof( T ) );
	return value;
}

bool SkynetTerrainAnalyser::tryLoadData( Data & data )
{
	std::string map_hash = BWAPI::Broodwar->mapHash();
	std::string directory = "bwapi-data/Skynet/TerrainAnalysis/";

	std::ifstream file( directory + map_hash + ".dat", std::ios_base::binary );

	if( !file.good() )
		return false;

	unsigned int version_number = readData<unsigned int>( file );

	if( data_version_number != version_number )
		return false;

	WalkPosition map_size = readData<WalkPosition>( file );

	if( map_size != m_map_size )
		return false;

	unsigned int num_regions = readData<unsigned int>( file );
	for( unsigned int i = 0; i < num_regions; ++i )
	{
		WalkPosition pos = readData<WalkPosition>( file );
		int clearance = readData<int>( file );
		int connectivity = readData<int>( file );
		int size = readData<int>( file );

		data.m_region_storage.emplace_back( std::make_unique<SkynetRegion>( i, pos, clearance, connectivity ) );
		data.m_region_storage.back()->setSize( size );
		data.m_regions.push_back( data.m_region_storage.back().get() );
	}

	unsigned int num_chokepoints = readData<unsigned int>( file );
	for( unsigned int i = 0; i < num_chokepoints; ++i )
	{
		WalkPosition center = readData<WalkPosition>( file );
		WalkPosition side1 = readData<WalkPosition>( file );
		WalkPosition side2 = readData<WalkPosition>( file );
		int clearance = readData<int>( file );
		unsigned int region_id_1 = readData<unsigned int>( file );
		unsigned int region_id_2 = readData<unsigned int>( file );

		data.m_chokepoint_storage.emplace_back( std::make_unique<SkynetChokepoint>( i, center, side1, side2, clearance ) );
		data.m_chokepoints.push_back( data.m_chokepoint_storage.back().get() );

		data.m_chokepoint_storage.back()->setRegion1( data.m_regions[region_id_1] );
		data.m_region_storage[region_id_1]->addChokepoint( data.m_chokepoints.back() );

		data.m_chokepoint_storage.back()->setRegion2( data.m_regions[region_id_2] );
		data.m_region_storage[region_id_2]->addChokepoint( data.m_chokepoints.back() );
	}

	data.m_chokepoint_distances.resize( num_chokepoints, num_chokepoints, max_distance );
	for( int i = 0; i < (int) num_chokepoints; ++i )
	{
		for( int j = 0; j < (int) num_chokepoints; ++j )
		{
			data.m_chokepoint_distances[Position( i, j )] = readData<int>( file );
		}
	}

	unsigned int num_base_locations = readData<unsigned int>( file );
	for( unsigned int i = 0; i < num_base_locations; ++i )
	{
		TilePosition build_location = readData<TilePosition>( file );
		unsigned int region_id = readData<unsigned int>( file );

		UnitGroup resouces;

		/*unsigned int num_resources = readData<unsigned int>( file );
		for( unsigned int j = 0; j < num_resources; ++j )
		{
		resouces.insert( getUnitTracker().getUnit( BWAPI::Broodwar->getUnit( readData<int>( file ) ) ), true );
		}*/

		data.m_base_location_storage.emplace_back( std::make_unique<SkynetBaseLocation>( build_location, data.m_regions[region_id], resouces ) );
		data.m_base_locations.push_back( data.m_base_location_storage.back().get() );
		data.m_region_storage[region_id]->addBase( data.m_base_locations.back() );
	}

	unsigned int connectivity_count = readData<unsigned int>( file );
	data.m_connectivity_to_small_obstacles.resize( connectivity_count );
	for( unsigned int i = 0; i < connectivity_count; ++i )
	{
		data.m_connectivity_to_small_obstacles[i] = readData<bool>( file );
	}

	data.m_tile_to_region.resize( m_map_size.x, m_map_size.y, nullptr );
	data.m_tile_clearance.resize( m_map_size.x, m_map_size.y, m_map_size.x + m_map_size.y );
	data.m_tile_connectivity.resize( m_map_size.x, m_map_size.y, -1 );
	data.m_tile_to_closest_obstacle.resize( m_map_size.x, m_map_size.y );

	for( int x = 0; x < m_map_size.x; ++x )
	{
		for( int y = 0; y < m_map_size.y; ++y )
		{
			WalkPosition pos( x, y );

			unsigned int region_id = readData<unsigned int>( file );
			data.m_tile_to_region[pos] = region_id < data.m_region_storage.size() ? data.m_region_storage[region_id].get() : nullptr;
			data.m_tile_clearance[pos] = readData<int>( file );
			data.m_tile_connectivity[pos] = readData<int>( file );
			data.m_tile_to_closest_obstacle[pos] = readData<WalkPosition>( file );
		}
	}

	for( const UnitGroup &resource_cluster : getResources().getClusters( 260, 3 ) )
	{
		TilePosition resouce_tile_position( resource_cluster.getCenter() );

		SkynetBaseLocation* best_base_location = nullptr;
		int best_distance = m_map_size.x * m_map_size.y;

		for( auto & base_location : data.m_base_location_storage )
		{
			int distance = base_location->getBuildLocation().getApproxDistance( resouce_tile_position );
			if( distance < best_distance )
			{
				best_distance = distance;
				best_base_location = base_location.get();
			}
		}

		if( best_base_location )
		{
			best_base_location->addResources( resource_cluster );
		}
	}

	data.m_analysed = true;

	return true;
}

void SkynetTerrainAnalyser::saveData( Data & data )
{
	std::string map_hash = BWAPI::Broodwar->mapHash();
	std::string directory = "bwapi-data/Skynet/TerrainAnalysis/";

	std::experimental::filesystem::path fs_dir( directory );
	if( !std::experimental::filesystem::exists( fs_dir ) )
	{
		std::experimental::filesystem::create_directories( fs_dir );
	}

	std::map<Region, unsigned int> region_ids;
	unsigned int region_id_counter = 0;
	for( auto & region : data.m_region_storage )
	{
		region_ids[region.get()] = region_id_counter;
		++region_id_counter;
	}

	std::ofstream file( directory + map_hash + ".dat", std::ios_base::trunc | std::ios_base::binary );

	writeData( file, data_version_number );
	writeData( file, m_map_size );

	writeData( file, (unsigned int) data.m_region_storage.size() );
	for( auto & region : data.m_region_storage )
	{
		writeData( file, region->getCenter() );
		writeData( file, region->getClearance() );
		writeData( file, region->getConnectivity() );
		writeData( file, region->getSize() );
	}

	writeData( file, (unsigned int) data.m_chokepoint_storage.size() );
	for( auto & chokepoint : data.m_chokepoint_storage )
	{
		writeData( file, chokepoint->getCenter() );
		writeData( file, chokepoint->getSides().first );
		writeData( file, chokepoint->getSides().second );
		writeData( file, chokepoint->getClearance() );
		writeData( file, region_ids[chokepoint->getRegions().first] );
		writeData( file, region_ids[chokepoint->getRegions().second] );
	}

	for( int i = 0, i_end = data.m_chokepoint_storage.size(); i < i_end; ++i )
	{
		for( int j = 0, j_end = data.m_chokepoint_storage.size(); j < j_end; ++j )
		{
			writeData( file, data.m_chokepoint_distances[Position( i, j )] );
		}
	}

	writeData( file, (unsigned int) data.m_base_location_storage.size() );
	for( auto & base_location : data.m_base_location_storage )
	{
		writeData( file, base_location->getBuildLocation() );
		writeData( file, region_ids[base_location->getRegion()] );

		/*writeData( file, (unsigned int) base_location->getStaticMinerals().size() + (unsigned int) base_location->getStaticGeysers().size() );
		for( auto mineral : base_location->getStaticMinerals() )
		{
		writeData( file, mineral->getBWAPIUnit()->getID() );
		}

		for( auto geyser : base_location->getStaticGeysers() )
		{
		writeData( file, geyser->getBWAPIUnit()->getID() );
		}*/
	}

	writeData( file, (unsigned int) data.m_connectivity_to_small_obstacles.size() );
	for( bool small_obstacle : data.m_connectivity_to_small_obstacles )
	{
		writeData( file, small_obstacle );
	}

	unsigned int null_region_id = (unsigned int) data.m_region_storage.size();
	for( int x = 0; x < m_map_size.x; ++x )
	{
		for( int y = 0; y < m_map_size.y; ++y )
		{
			WalkPosition pos( x, y );

			Region region = data.m_tile_to_region[pos];
			writeData( file, region ? region_ids[region] : null_region_id );
			writeData( file, data.m_tile_clearance[pos] );
			writeData( file, data.m_tile_connectivity[pos] );
			writeData( file, data.m_tile_to_closest_obstacle[pos] );
		}
	}
}

void SkynetTerrainAnalyser::notify( const UnitDiscover & message )
{
	if( message.unit->getType().isBuilding() )
	{
		m_buildings.insert( message.unit );
		++m_reprocess_request;
	}
}

void SkynetTerrainAnalyser::notify( const UnitMorphRenegade & message )
{
	if( message.isMorph() )
	{
		if( message.last_type.isBuilding() )
		{
			m_buildings.remove( message.unit );
			++m_reprocess_request;
		}
		if( message.unit->getType().isBuilding() )
		{
			m_buildings.insert( message.unit );
			++m_reprocess_request;
		}
	}
}

void SkynetTerrainAnalyser::notify( const UnitDestroy & message )
{
	if( message.unit->getType().isBuilding() )
	{
		m_buildings.remove( message.unit );
		++m_reprocess_request;
	}
}
