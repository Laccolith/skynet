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

SkynetTerrainAnalyser::SkynetTerrainAnalyser( Core & core )
	: TerrainAnalyserInterface( core )
	, m_map_size( BWAPI::Broodwar->mapWidth() * 4, BWAPI::Broodwar->mapHeight() * 4 )
{
	core.registerUpdateProcess( 1.0f, [this]() { update(); } );
}

void SkynetTerrainAnalyser::update()
{
	if( !m_processed_data.m_analysed )
	{
		if( !tryLoadData() )
		{
			process( m_processed_data, getResources() );
			saveData();
		}
		
		postMessage<TerrainAnalysed>();
	}

	if( isDebugging( Debug::Default ) && BWAPI::Broodwar->getKeyState( BWAPI::Key( '6' ) ) )
		++m_reprocess_request;

	checkData();

	if( isDebugging( Debug::Default ) )
	{
		bool show_connectivity = BWAPI::Broodwar->getKeyState( BWAPI::Key( '1' ) );
		bool show_clearance = BWAPI::Broodwar->getKeyState( BWAPI::Key( '2' ) );
		bool show_regions = BWAPI::Broodwar->getKeyState( BWAPI::Key( '3' ) );
		bool show_chokepoints = BWAPI::Broodwar->getKeyState( BWAPI::Key( '4' ) );
		bool show_base_locations = BWAPI::Broodwar->getKeyState( BWAPI::Key( '5' ) );

		WalkPosition mouse_tile( BWAPI::Broodwar->getMousePosition() + BWAPI::Broodwar->getScreenPosition() );
		if( mouse_tile.isValid() )
		{
			if( show_connectivity )
			{
				int current_connectivity = m_processed_data.m_tile_connectivity[mouse_tile];

				WalkPosition top_left( BWAPI::Broodwar->getScreenPosition() );
				WalkPosition bottom_right( top_left + WalkPosition( Position( 640, 480 ) ) );
				if( bottom_right.x > m_map_size.x ) bottom_right.x = m_map_size.x;
				if( bottom_right.y > m_map_size.y ) bottom_right.y = m_map_size.y;

				MapUtil::forEachPosition( top_left, bottom_right, [this, current_connectivity]( WalkPosition pos )
				{
					if( m_processed_data.m_tile_connectivity[pos] == current_connectivity )
						BWAPI::Broodwar->drawBoxMap( pos.x * 8, pos.y * 8, pos.x * 8 + 7, pos.y * 8 + 7, Colors::Red );
				} );

				if( m_processed_data.m_connectivity_to_small_obstacles[current_connectivity] )
					BWAPI::Broodwar->drawTextMouse( 8, -8, "Small Obstacle" );
			}

			if( show_clearance )
				BWAPI::Broodwar->drawCircleMap( Position( mouse_tile ), m_processed_data.m_tile_clearance[mouse_tile], Colors::Red );
		}

		if( show_regions )
		{
			for( auto region : m_processed_data.m_regions )
				region->draw( Colors::Blue );
		}

		if( show_chokepoints )
		{
			for( auto chokepoint : m_processed_data.m_chokepoints )
				chokepoint->draw( Colors::Red );
		}

		if( show_base_locations )
		{
			for( auto base_location : m_processed_data.m_base_locations )
				base_location->draw( Colors::Green );
		}

		BWAPI::Broodwar->drawTextScreen( 5, 5, "Terrain Analysis Connectivity: %.2f", m_processed_data.m_connectivity_time_seconds );
		BWAPI::Broodwar->drawTextScreen( 5, 15, "Terrain Analysis Clearance: %.2f", m_processed_data.m_clearance_time_seconds );
		BWAPI::Broodwar->drawTextScreen( 5, 25, "Terrain Analysis Regions: %.2f", m_processed_data.m_regions_time_seconds );

		if( m_async_future.valid() )
			BWAPI::Broodwar->drawTextScreen( 5, 35, "Analysing Terrain" );
	}
}

void SkynetTerrainAnalyser::process( Data & data, UnitGroup resources )
{
	calculateConnectivity( data );
	calculateClearance( data );
	calculateRegions( data );
	createBases( data, resources );

	data.m_analysed = true;
}

void SkynetTerrainAnalyser::calculateConnectivity( Data & data )
{
	auto start_time = std::chrono::system_clock::now();

	data.m_tile_connectivity.resize( m_map_size.x, m_map_size.y, -1 );

	std::atomic<int> connectivity_counter( 0 );

	auto calculate_connectivity = [this, &connectivity_counter, &data]( bool walkable_tiles )
	{
		for( int x = 0; x < m_map_size.x; ++x )
		{
			for( int y = 0; y < m_map_size.y; ++y )
			{
				WalkPosition pos( x, y );

				if( data.m_tile_connectivity[pos] != -1 ) continue;
				if( BWAPI::Broodwar->isWalkable( pos ) != walkable_tiles ) continue;

				int region_area = 0;
				int region_connectivity = connectivity_counter++;
				bool touches_edge = false;

				data.m_tile_connectivity[pos] = region_connectivity;

				std::queue<WalkPosition> unvisited_tiles;
				unvisited_tiles.push( pos );

				while( !unvisited_tiles.empty() )
				{
					const WalkPosition &tile = unvisited_tiles.front();

					++region_area;

					WalkPosition pos_north( tile.x, tile.y - 1 );
					if( tile.y <= 0 ) touches_edge = true;
					else if( BWAPI::Broodwar->isWalkable( pos_north ) == walkable_tiles && data.m_tile_connectivity[pos_north] == -1 )
					{
						unvisited_tiles.push( pos_north );
						data.m_tile_connectivity[pos_north] = region_connectivity;
					}

					WalkPosition pos_east( tile.x + 1, tile.y );
					if( tile.x >= m_map_size.x - 1 ) touches_edge = true;
					else if( BWAPI::Broodwar->isWalkable( pos_east ) == walkable_tiles && data.m_tile_connectivity[pos_east] == -1 )
					{
						unvisited_tiles.push( pos_east );
						data.m_tile_connectivity[pos_east] = region_connectivity;
					}

					WalkPosition pos_south( tile.x, tile.y + 1 );
					if( tile.y >= m_map_size.y - 1 ) touches_edge = true;
					else if( BWAPI::Broodwar->isWalkable( pos_south ) == walkable_tiles && data.m_tile_connectivity[pos_south] == -1 )
					{
						unvisited_tiles.push( pos_south );
						data.m_tile_connectivity[pos_south] = region_connectivity;
					}

					WalkPosition pos_west( tile.x - 1, tile.y );
					if( tile.x <= 0 ) touches_edge = true;
					else if( BWAPI::Broodwar->isWalkable( pos_west ) == walkable_tiles && data.m_tile_connectivity[pos_west] == -1 )
					{
						unvisited_tiles.push( pos_west );
						data.m_tile_connectivity[pos_west] = region_connectivity;
					}

					unvisited_tiles.pop();
				}

				if( !walkable_tiles )
				{
					data.m_connectivity_to_small_obstacles.resize( region_connectivity + 1, false );
					data.m_connectivity_to_small_obstacles[region_connectivity] = region_area < 200 && !touches_edge;
				}
			}
		}
	};

	std::thread thread( calculate_connectivity, true );
	calculate_connectivity( false );
	thread.join();

	data.m_connectivity_to_small_obstacles.resize( connectivity_counter + 1, false );

	std::chrono::duration<float> elapsed_seconds = std::chrono::system_clock::now() - start_time;
	data.m_connectivity_time_seconds = elapsed_seconds.count();
}

void SkynetTerrainAnalyser::calculateClearance( Data & data )
{
	auto start_time = std::chrono::system_clock::now();

	data.m_tile_clearance.resize( m_map_size.x, m_map_size.y, m_map_size.x + m_map_size.y );
	data.m_tile_to_closest_obstacle.resize( m_map_size.x, m_map_size.y );

	std::queue<std::pair<WalkPosition, int>> queued_tiles;

	for( int x = 0; x < m_map_size.x; ++x )
	{
		for( int y = 0; y < m_map_size.y; ++y )
		{
			WalkPosition pos( x, y );

			if( !BWAPI::Broodwar->isWalkable( pos ) )
			{
				data.m_tile_clearance[pos] = 0;
				data.m_tile_to_closest_obstacle[pos] = pos;

				if( !data.m_connectivity_to_small_obstacles[data.m_tile_connectivity[pos]] )
					queued_tiles.emplace( pos, 0 );
			}
			else if( x == 0 || y == 0 || x == m_map_size.x - 1 || y == m_map_size.y - 1 )
			{
				data.m_tile_clearance[pos] = 10;

				data.m_tile_to_closest_obstacle[pos] = WalkPosition(
					(x == 0 ? -1 : (x == m_map_size.x - 1 ? m_map_size.x : x)),
					(y == 0 ? -1 : (y == m_map_size.y - 1 ? m_map_size.y : y)) );

				queued_tiles.emplace( pos, 10 );
			}
		}
	}

	auto visit_direction = [this, &queued_tiles, &data]( int distance, WalkPosition current_obstacle, WalkPosition next_pos )
	{
		if( distance < data.m_tile_clearance[next_pos] )
		{
			data.m_tile_clearance[next_pos] = distance;
			data.m_tile_to_closest_obstacle[next_pos] = current_obstacle;
			queued_tiles.emplace( next_pos, distance );
		}
	};

	while( !queued_tiles.empty() )
	{
		const WalkPosition current_tile = queued_tiles.front().first;
		const int next_distance = queued_tiles.front().second + 10;
		const int next_distance_diag = next_distance + 4;
		queued_tiles.pop();

		const WalkPosition current_obstacle = data.m_tile_to_closest_obstacle[current_tile];

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
	data.m_clearance_time_seconds = elapsed_seconds.count();
}

struct RegionTileData
{
	WalkPosition last_minima = WalkPositions::None;
	unsigned int children_flags = 0;

	void set_child( int i ) { children_flags |= (i + 1); }
	bool has_child( int i ) const { return (children_flags & (i + 1)) != 0; }
};

void SkynetTerrainAnalyser::calculateRegions( Data & data )
{
	auto start_time = std::chrono::system_clock::now();

	static std::unique_ptr<Window> debug_window;
	if( isDebugging( Debug::RegionAnalysis ) )
	{
		debug_window = std::make_unique<Window>( "Calculating Regions", m_map_size.x * 8, m_map_size.y * 8 );

		for( int x = 0; x < m_map_size.x; ++x )
		{
			for( int y = 0; y < m_map_size.y; ++y )
			{
				if( data.m_tile_clearance[WalkPosition( x, y )] == 0 )
					debug_window->addBox( x * 8, y * 8, x * 8 + 8, y * 8 + 8, Colors::Red );
			}
		}
	}

	data.m_tile_to_region.resize( m_map_size.x, m_map_size.y, nullptr );

	RectangleArray<RegionTileData, WALKPOSITION_SCALE> tile_data( m_map_size.x, m_map_size.y );
	RectangleArray<SkynetChokepoint *, WALKPOSITION_SCALE> tile_to_chokepoint( m_map_size.x, m_map_size.y, nullptr );

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

				if( data.m_tile_to_region[pos] || tile_to_chokepoint[pos] )
					continue;

				const int local_maxima_value = data.m_tile_clearance[pos];
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
		data.m_region_storage.emplace_back( std::make_unique<SkynetRegion>( current_region_tile, current_region_clearance, data.m_tile_connectivity[current_region_tile] ) );
		SkynetRegion *current_region = data.m_region_storage.back().get();
		data.m_regions.push_back( current_region );

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
				if( debug_window )
					debug_window->addBox( current_tile.x * 8, current_tile.y * 8, current_tile.x * 8, current_tile.y * 8, Colors::Purple );

				// Chokepoint already has a second region
				if( existing_chokepoint->getRegions().second )
				{
					// It is not the current region, this shouldn't happen
					if( existing_chokepoint->getRegions().second != current_region )
					{
						//DrawBuffer::Instance().drawBufferedBox(BWAPI::CoordinateType::Map, currentTile.x * 8, currentTile.y * 8, currentTile.x * 8 + 8, currentTile.y * 8 + 8, 999999, BWAPI::Colors::Red);
						//LOGMESSAGEWARNING("Touched a choke saved to anouther region");
					}
				}
				// This chokepoint wasn't created by the current region, add it as the other side
				else if( existing_chokepoint->getRegions().first != current_region )
				{
					current_region->addChokepoint( existing_chokepoint );
					existing_chokepoint->setRegion2( current_region );
				}

				continue;
			}

			if( !data.m_tile_to_region[current_tile] )
			{
				++region_size;
				data.m_tile_to_region[current_tile] = current_region;
			}
			else if( data.m_tile_to_region[current_tile] != current_region )
			{
				//LOGMESSAGEWARNING("2 regions possibly connected without a choke");
				continue;
			}

			WalkPosition last_minima = tile_data[current_tile].last_minima;
			const int last_minima_size = data.m_tile_clearance[last_minima];

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
				std::pair<WalkPosition, WalkPosition> choke_sides = findChokePoint( last_minima, data );

				// Create the chokepoint
				data.m_chokepoint_storage.emplace_back( std::make_unique<SkynetChokepoint>( last_minima, choke_sides.first, choke_sides.second, last_minima_size ) );
				SkynetChokepoint *current_chokepoint = data.m_chokepoint_storage.back().get();
				data.m_chokepoints.push_back( current_chokepoint );

				// The first region is the region that created it
				current_chokepoint->setRegion1( current_region );
				current_region->addChokepoint( current_chokepoint );

				// For every tile between the chokepoint sides
				std::queue<WalkPosition> choke_children;
				auto add_choke_children = [this, &region_size, &tile_to_chokepoint, &choke_children, &current_chokepoint, &data]( WalkPosition line_pos )
				{
					if( line_pos.x >= 0 && line_pos.y >= 0 && line_pos.x < m_map_size.x && line_pos.y < m_map_size.y && data.m_tile_clearance[line_pos] != 0 )
					{
						++region_size;
						tile_to_chokepoint[line_pos] = current_chokepoint;
						choke_children.push( line_pos );
					}

					return false;
				};

				MapUtil::forEachPositionInLine( choke_sides.first, last_minima, add_choke_children );
				MapUtil::forEachPositionInLine( last_minima, choke_sides.second, add_choke_children );

				if( debug_window )
				{
					debug_window->addLine( choke_sides.first.x * 8, choke_sides.first.y * 8, last_minima.x * 8, last_minima.y * 8, 8, Colors::Orange );
					debug_window->addLine( last_minima.x * 8, last_minima.y * 8, choke_sides.second.x * 8, choke_sides.second.y * 8, 8, Colors::Orange );
				}

				// Remove any tiles that are after the chokepoint as they are now cut off
				while( !choke_children.empty() )
				{
					--region_size;
					auto current_tile = choke_children.front();
					choke_children.pop();

					data.m_tile_to_region[current_tile] = nullptr;
					tile_data[current_tile].last_minima = WalkPositions::None;

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
				if( data.m_tile_clearance[current_tile] < last_minima_size )
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
					if( data.m_tile_clearance[next_tile] == 0 )
						continue;

					if( tile_data[next_tile].last_minima == WalkPositions::None )
					{
						tile_data[next_tile].last_minima = last_minima;
						tile_data[current_tile].set_child( i );

						unvisited_tiles.emplace( next_tile, data.m_tile_clearance[next_tile] );
					}
				}
			}
		}

		current_region->setSize( region_size );
	}

	std::chrono::duration<float> elapsed_seconds = std::chrono::system_clock::now() - start_time;
	data.m_regions_time_seconds = elapsed_seconds.count();
}

std::pair<WalkPosition, WalkPosition> SkynetTerrainAnalyser::findChokePoint( WalkPosition center, Data & data ) const
{
	const WalkPosition side1 = data.m_tile_to_closest_obstacle[center];

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
		if( x0 < 0 || y0 < 0 || x0 >= m_map_size.x || y0 >= m_map_size.y || !BWAPI::Broodwar->isWalkable( pos ) )
			return std::make_pair( side1, pos );

		WalkPosition side2 = data.m_tile_to_closest_obstacle[pos];

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

void SkynetTerrainAnalyser::createBases( Data & data, const UnitGroup & resources )
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
		for( auto &other_base_location : data.m_base_location_storage )
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
			SkynetRegion *base_region = data.m_tile_to_region[WalkPosition( base_location )];
			data.m_base_location_storage.emplace_back( std::make_unique<SkynetBaseLocation>( base_location, base_region, resource_cluster ) );
			data.m_base_locations.push_back( data.m_base_location_storage.back().get() );
			base_region->addBase( data.m_base_locations.back() );
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

void SkynetTerrainAnalyser::checkData()
{
	m_old_regions.clear();
	m_old_chokepoints.clear();
	m_old_base_locations.clear();

	if( m_async_future.valid() )
	{
		if( m_async_future.wait_for( std::chrono::milliseconds( 2 ) ) == std::future_status::ready )
		{
			m_old_regions = std::move( m_processed_data.m_region_storage );
			for( auto & region : m_old_regions ) region->markInvalid();

			m_old_chokepoints = std::move( m_processed_data.m_chokepoint_storage );
			for( auto & chokepoint : m_old_chokepoints ) chokepoint->markInvalid();

			m_old_base_locations = std::move( m_processed_data.m_base_location_storage );
			for( auto & base_location : m_old_base_locations ) base_location->markInvalid();

			m_processed_data = std::move( *m_async_future.get() );
			postMessage<TerrainAnalysed>();
		}
	}

	if( !m_async_future.valid() && m_processed_data.m_request < m_reprocess_request )
	{
		UnitGroup resources = getResources();
		int request = m_reprocess_request;
		m_async_future = std::async( std::launch::async, [this, resources, request]() -> std::unique_ptr<Data>
		{
			auto new_data = std::make_unique<Data>();
			new_data->m_request = request;

			process( *new_data, resources );

			return new_data;
		} );
	}
}

const unsigned int data_version_number = 1;

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

bool SkynetTerrainAnalyser::tryLoadData()
{
#if defined(_DEBUG)
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

		m_processed_data.m_region_storage.emplace_back( std::make_unique<SkynetRegion>( pos, clearance, connectivity ) );
		m_processed_data.m_region_storage.back()->setSize( size );
		m_processed_data.m_regions.push_back( m_processed_data.m_region_storage.back().get() );
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

		m_processed_data.m_chokepoint_storage.emplace_back( std::make_unique<SkynetChokepoint>( center, side1, side2, clearance ) );
		m_processed_data.m_chokepoints.push_back( m_processed_data.m_chokepoint_storage.back().get() );

		m_processed_data.m_chokepoint_storage.back()->setRegion1( m_processed_data.m_regions[region_id_1] );
		m_processed_data.m_region_storage[region_id_1]->addChokepoint( m_processed_data.m_chokepoints.back() );

		m_processed_data.m_chokepoint_storage.back()->setRegion2( m_processed_data.m_regions[region_id_2] );
		m_processed_data.m_region_storage[region_id_2]->addChokepoint( m_processed_data.m_chokepoints.back() );
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

		m_processed_data.m_base_location_storage.emplace_back( std::make_unique<SkynetBaseLocation>( build_location, m_processed_data.m_regions[region_id], resouces ) );
		m_processed_data.m_base_locations.push_back( m_processed_data.m_base_location_storage.back().get() );
		m_processed_data.m_region_storage[region_id]->addBase( m_processed_data.m_base_locations.back() );
	}

	unsigned int connectivity_count = readData<unsigned int>( file );
	m_processed_data.m_connectivity_to_small_obstacles.resize( connectivity_count );
	for( unsigned int i = 0; i < connectivity_count; ++i )
	{
		m_processed_data.m_connectivity_to_small_obstacles[i] = readData<bool>( file );
	}

	m_processed_data.m_tile_to_region.resize( m_map_size.x, m_map_size.y, nullptr );
	m_processed_data.m_tile_clearance.resize( m_map_size.x, m_map_size.y, m_map_size.x + m_map_size.y );
	m_processed_data.m_tile_connectivity.resize( m_map_size.x, m_map_size.y, -1 );
	m_processed_data.m_tile_to_closest_obstacle.resize( m_map_size.x, m_map_size.y );

	for( int x = 0; x < m_map_size.x; ++x )
	{
		for( int y = 0; y < m_map_size.y; ++y )
		{
			WalkPosition pos( x, y );

			m_processed_data.m_tile_to_region[pos] = m_processed_data.m_region_storage[readData<unsigned int>( file )].get();
			m_processed_data.m_tile_clearance[pos] = readData<int>( file );
			m_processed_data.m_tile_connectivity[pos] = readData<int>( file );
			m_processed_data.m_tile_to_closest_obstacle[pos] = readData<WalkPosition>( file );
		}
	}

	for( const UnitGroup &resource_cluster : getResources().getClusters( 260, 3 ) )
	{
		TilePosition resouce_tile_position( resource_cluster.getCenter() );

		SkynetBaseLocation* best_base_location = nullptr;
		int best_distance = m_map_size.x * m_map_size.y;

		for( auto & base_location : m_processed_data.m_base_location_storage )
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

	m_processed_data.m_analysed = true;

	return true;
#else
	return false;
#endif
	
}

void SkynetTerrainAnalyser::saveData()
{
#if defined(_DEBUG)
	std::string map_hash = BWAPI::Broodwar->mapHash();
	std::string directory = "bwapi-data/Skynet/TerrainAnalysis/";

	std::experimental::filesystem::path fs_dir( directory );
	if( !std::experimental::filesystem::exists( fs_dir ) )
	{
		std::experimental::filesystem::create_directories( fs_dir );
	}

	std::map<Region, unsigned int> region_ids;
	unsigned int region_id_counter = 0;
	for( auto & region : m_processed_data.m_region_storage )
	{
		region_ids[region.get()] = region_id_counter;
		++region_id_counter;
	}

	std::ofstream file( directory + map_hash + ".dat", std::ios_base::trunc | std::ios_base::binary );

	writeData( file, data_version_number );
	writeData( file, m_map_size );

	writeData( file, (unsigned int) m_processed_data.m_region_storage.size() );
	for( auto & region : m_processed_data.m_region_storage )
	{
		writeData( file, region->getCenter() );
		writeData( file, region->getClearance() );
		writeData( file, region->getConnectivity() );
		writeData( file, region->getSize() );
	}

	writeData( file, (unsigned int) m_processed_data.m_chokepoint_storage.size() );
	for( auto & chokepoint : m_processed_data.m_chokepoint_storage )
	{
		writeData( file, chokepoint->getCenter() );
		writeData( file, chokepoint->getSides().first );
		writeData( file, chokepoint->getSides().second );
		writeData( file, chokepoint->getClearance() );
		writeData( file, region_ids[chokepoint->getRegions().first] );
		writeData( file, region_ids[chokepoint->getRegions().second] );
	}

	writeData( file, (unsigned int) m_processed_data.m_base_location_storage.size() );
	for( auto & base_location : m_processed_data.m_base_location_storage )
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

	writeData( file, (unsigned int) m_processed_data.m_connectivity_to_small_obstacles.size() );
	for( bool small_obstacle : m_processed_data.m_connectivity_to_small_obstacles )
	{
		writeData( file, small_obstacle );
	}

	for( int x = 0; x < m_map_size.x; ++x )
	{
		for( int y = 0; y < m_map_size.y; ++y )
		{
			WalkPosition pos( x, y );

			writeData( file, region_ids[m_processed_data.m_tile_to_region[pos]] );
			writeData( file, m_processed_data.m_tile_clearance[pos] );
			writeData( file, m_processed_data.m_tile_connectivity[pos] );
			writeData( file, m_processed_data.m_tile_to_closest_obstacle[pos] );
		}
	}
#endif
}
