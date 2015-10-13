#include "SkynetTerrainAnalyser.h"

#include "Skynet.h"
#include "Heap.h"
#include "MapUtil.h"
#include "UnitTracker.h"

#include <thread>
#include <atomic>
#include <queue>

SkynetTerrainAnalyser::SkynetTerrainAnalyser( Access & access )
	: TerrainAnalyserInterface( access )
	, m_map_size( BWAPI::Broodwar->mapWidth() * 4, BWAPI::Broodwar->mapHeight() * 4 )
{
	getSkynet().registerUpdateProcess( 1.0f, [this](){ update(); } );
}

void SkynetTerrainAnalyser::update()
{
	if( !m_data.m_analysed )
	{
		Process( m_map_size, m_data, getResources() );
		postMessage<TerrainAnalysed>();
	}

	if( isDebugging() && BWAPI::Broodwar->getKeyState( BWAPI::Key( '6' ) ) )
		++m_reprocess_request;

	check_data();

	if( isDebugging() )
	{
		bool show_connectivity = BWAPI::Broodwar->getKeyState( BWAPI::Key('1') );
		bool show_clearnace = BWAPI::Broodwar->getKeyState( BWAPI::Key('2') );
		bool show_regions = BWAPI::Broodwar->getKeyState( BWAPI::Key('3') );
		bool show_chokepoints = BWAPI::Broodwar->getKeyState( BWAPI::Key('4') );
		bool show_base_locations = BWAPI::Broodwar->getKeyState( BWAPI::Key('5') );

		WalkPosition mouse_tile( BWAPI::Broodwar->getMousePosition() + BWAPI::Broodwar->getScreenPosition() );
		if( mouse_tile.isValid() )
		{
			if( show_connectivity )
			{
				int current_connectivity = m_data.m_tile_connectivity[mouse_tile];

				WalkPosition top_left( BWAPI::Broodwar->getScreenPosition() );
				WalkPosition bottom_right( top_left + std::min( WalkPosition( Position( 640, 480 ) ), m_map_size ) );

				MapUtil::forEachPosition( top_left, bottom_right, [this, current_connectivity]( WalkPosition pos )
				{
					if( m_data.m_tile_connectivity[pos] == current_connectivity )
						BWAPI::Broodwar->drawBoxMap( pos.x * 8, pos.y * 8, pos.x * 8 + 7, pos.y * 8 + 7, Colors::Red );
				} );

				if( m_data.m_connectivity_to_small_obstacles[current_connectivity] )
					BWAPI::Broodwar->drawTextMouse( 8, -8, "Small Obstacle" );
			}

			if( show_clearnace )
				BWAPI::Broodwar->drawCircleMap( mouse_tile.x * 8, mouse_tile.y * 8, m_data.m_tile_clearance[mouse_tile], Colors::Red );
		}

		if( show_regions )
		{
			for( auto region : m_data.m_regions )
				region->draw( Colors::Blue );
		}

		if( show_chokepoints )
		{
			for( auto chokepoint : m_data.m_chokepoints )
				chokepoint->draw( Colors::Red );
		}

		if( show_base_locations )
		{
			for( auto base_location : m_data.m_base_locations )
				base_location->draw( Colors::Green );
		}

		if( m_async_future.valid() )
			BWAPI::Broodwar->drawTextScreen( 10, 10, "Analysing Terrain" );
	}
}

SkynetTerrainAnalyser::Process::Process( WalkPosition map_size, Data & data, const UnitGroup & resources )
	: m_map_size( map_size )
	, m_data( data )
	, m_resources( resources )
{
	calculateConnectivity();
	calculateWalkTileClearance();
	calculateRegions();
	createBases();

	m_data.m_analysed = true;
}

void SkynetTerrainAnalyser::Process::calculateConnectivity()
{
	m_data.m_tile_connectivity.resize( m_map_size.x, m_map_size.y, -1 );

	std::atomic<int> connectivity_counter;

	auto calculate_connectivity = [this, &connectivity_counter]( bool walkable_tiles )
	{
		for( int x = 0; x < m_map_size.x; ++x )
		{
			for( int y = 0; y < m_map_size.y; ++y )
			{
				WalkPosition pos( x, y );

				if( m_data.m_tile_connectivity[pos] != -1 ) continue;
				if( BWAPI::Broodwar->isWalkable( pos ) != walkable_tiles ) continue;

				int region_area = 0;
				int region_connectivity = connectivity_counter++;

				m_data.m_tile_connectivity[pos] = region_connectivity;

				std::queue<WalkPosition> unvisited_tiles;
				unvisited_tiles.push( pos );
				
				while( !unvisited_tiles.empty() )
				{
					const WalkPosition &tile = unvisited_tiles.front();

					++region_area;

					WalkPosition pos_north( tile.x, tile.y - 1 );
					if( tile.y > 0 && BWAPI::Broodwar->isWalkable( pos_north ) == walkable_tiles && m_data.m_tile_connectivity[pos_north] == -1 )
					{
						unvisited_tiles.push( pos_north );
						m_data.m_tile_connectivity[pos_north] = region_connectivity;
					}

					WalkPosition pos_east( tile.x + 1, tile.y );
					if( tile.x < m_map_size.x - 1 && BWAPI::Broodwar->isWalkable( pos_east ) == walkable_tiles && m_data.m_tile_connectivity[pos_east] == -1 )
					{
						unvisited_tiles.push( pos_east );
						m_data.m_tile_connectivity[pos_east] = region_connectivity;
					}

					WalkPosition pos_south( tile.x, tile.y + 1 );
					if( tile.y < m_map_size.y - 1 && BWAPI::Broodwar->isWalkable( pos_south ) == walkable_tiles && m_data.m_tile_connectivity[pos_south] == -1 )
					{
						unvisited_tiles.push( pos_south );
						m_data.m_tile_connectivity[pos_south] = region_connectivity;
					}

					WalkPosition pos_west( tile.x - 1, tile.y );
					if( tile.x > 0 && BWAPI::Broodwar->isWalkable( pos_west ) == walkable_tiles && m_data.m_tile_connectivity[pos_west] == -1 )
					{
						unvisited_tiles.push( pos_west );
						m_data.m_tile_connectivity[pos_west] = region_connectivity;
					}

					unvisited_tiles.pop();
				}

				if( !walkable_tiles )
				{
					m_data.m_connectivity_to_small_obstacles.resize( region_connectivity + 1, false );
					m_data.m_connectivity_to_small_obstacles[region_connectivity] = region_area < 200;
				}
			}
		}
	};

	std::thread thread( calculate_connectivity, true );
	calculate_connectivity( false );
	thread.join();

	m_data.m_connectivity_to_small_obstacles.resize( connectivity_counter + 1, false );
}

void SkynetTerrainAnalyser::Process::calculateWalkTileClearance()
{
	m_data.m_tile_clearance.resize( m_map_size.x, m_map_size.y, -1 );
	m_data.m_tile_to_closest_obstacle.resize( m_map_size.x, m_map_size.y );

	Heap<WalkPosition, int> unvisited_tiles( true );
	for( int x = 0; x < m_map_size.x; ++x )
	{
		for( int y = 0; y < m_map_size.y; ++y )
		{
			WalkPosition pos( x, y );

			if( !BWAPI::Broodwar->isWalkable( pos ) )
			{
				m_data.m_tile_clearance[pos] = 0;
				m_data.m_tile_to_closest_obstacle[pos] = pos;

				if( !m_data.m_connectivity_to_small_obstacles[m_data.m_tile_connectivity[pos]] )
					unvisited_tiles.set( pos, 0 );
			}
			else if( x == 0 || y == 0 || x == m_map_size.x - 1 || y == m_map_size.y - 1 )
			{
				m_data.m_tile_clearance[pos] = 10;

				m_data.m_tile_to_closest_obstacle[pos] = WalkPosition(
					(x == 0 ? -1 : (x == m_map_size.x - 1 ? m_map_size.x : x)),
					(y == 0 ? -1 : (y == m_map_size.y - 1 ? m_map_size.y : y)) );

				unvisited_tiles.set( pos, 10 );
			}
		}
	}

	auto visit_direction = [this, &unvisited_tiles]( int distance, WalkPosition current_obstacle, WalkPosition next_pos )
	{
		if( m_data.m_tile_clearance[next_pos] == -1 || distance < m_data.m_tile_clearance[next_pos] )
		{
			m_data.m_tile_clearance[next_pos] = distance;
			m_data.m_tile_to_closest_obstacle[next_pos] = current_obstacle;
			unvisited_tiles.set( next_pos, distance );
		}
	};

	while( !unvisited_tiles.empty() )
	{
		const WalkPosition tile = unvisited_tiles.top().first;
		const int distance = unvisited_tiles.top().second + 10;
		const int diag_distance = distance + 4;
		unvisited_tiles.pop();

		const WalkPosition current_obstacle = m_data.m_tile_to_closest_obstacle[tile];

		const int west = tile.x - 1;
		const int north = tile.y - 1;
		const int east = tile.x + 1;
		const int south = tile.y + 1;

		const bool can_go_west = west >= 0;
		const bool can_go_north = north >= 0;
		const bool can_go_east = east < m_map_size.x;
		const bool can_go_south = south < m_map_size.y;

		if( can_go_west )
			visit_direction( distance, current_obstacle, WalkPosition( west, tile.y ) );

		if( can_go_north )
			visit_direction( distance, current_obstacle, WalkPosition( tile.x, north ) );

		if( can_go_east )
			visit_direction( distance, current_obstacle, WalkPosition( east, tile.y ) );

		if( can_go_south )
			visit_direction( distance, current_obstacle, WalkPosition( tile.x, south ) );

		if( can_go_west && can_go_north )
			visit_direction( diag_distance, current_obstacle, WalkPosition( west, north ) );

		if( can_go_east && can_go_south )
			visit_direction( diag_distance, current_obstacle, WalkPosition( east, south ) );

		if( can_go_east && can_go_north )
			visit_direction( diag_distance, current_obstacle, WalkPosition( east, north ) );

		if( can_go_west && can_go_south )
			visit_direction( diag_distance, current_obstacle, WalkPosition( west, south ) );
	}
}

void SkynetTerrainAnalyser::Process::calculateRegions()
{
	m_data.m_tile_to_region.resize( m_map_size.x, m_map_size.y, nullptr );
	std::map<WalkPosition, SkynetChokepoint *> choke_tiles;

	while( true )
	{
		int current_region_clearance = 0;
		WalkPosition current_region_tile;

		for( int x = 0; x < m_map_size.x; ++x )
		{
			for( int y = 0; y < m_map_size.y; ++y )
			{
				WalkPosition pos( x, y );

				if( m_data.m_tile_to_region[pos] )
					continue;

				const int local_maxima_value = m_data.m_tile_clearance[pos];
				if( local_maxima_value > current_region_clearance )
				{
					current_region_clearance = local_maxima_value;
					current_region_tile = pos;
				}
			}
		}

		if( current_region_clearance == 0 )
			break;

		m_data.m_region_storage.emplace_back( std::make_unique<SkynetRegion>( current_region_tile, current_region_clearance, m_data.m_tile_connectivity[current_region_tile] ) );
		SkynetRegion *current_region = m_data.m_region_storage.back().get();
		m_data.m_regions.push_back( current_region );

		std::map<WalkPosition, WalkPosition> tile_to_last_minima;
		std::map<WalkPosition, std::vector<WalkPosition>> tile_to_children;

		Heap<WalkPosition, int> unvisited_tiles;

		unvisited_tiles.set( current_region_tile, current_region_clearance );
		tile_to_last_minima[current_region_tile] = current_region_tile;

		while( !unvisited_tiles.empty() )
		{
			WalkPosition current_tile = unvisited_tiles.top().first;
			int current_tile_clearance = unvisited_tiles.top().second;
			unvisited_tiles.pop();

			if( choke_tiles.count( current_tile ) != 0 )
			{
				if( choke_tiles[current_tile]->getRegions().second && choke_tiles[current_tile]->getRegions().second != current_region )
				{
					//DrawBuffer::Instance().drawBufferedBox(BWAPI::CoordinateType::Map, currentTile.x * 8, currentTile.y * 8, currentTile.x * 8 + 8, currentTile.y * 8 + 8, 999999, BWAPI::Colors::Red);
					//LOGMESSAGEWARNING("Touched a choke saved to anouther region");
				}
				else if( choke_tiles[current_tile]->getRegions().first != current_region )
				{
					current_region->addChokepoint( choke_tiles[current_tile] );
					choke_tiles[current_tile]->setRegion2( current_region );
				}

				continue;
			}

			if( m_data.m_tile_to_region[current_tile] )
			{
				//LOGMESSAGEWARNING("2 regions possibly connected without a choke");
				continue;
			}

			WalkPosition last_minima = tile_to_last_minima[current_tile];
			const int choke_size = m_data.m_tile_clearance[last_minima];

			bool found_chokepoint = false;
			if( choke_size < int( float( current_region_clearance )*0.90f ) && choke_size < int( float( current_tile_clearance )*0.80f ) )
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

			if( found_chokepoint )
			{
				std::pair<WalkPosition, WalkPosition> choke_sides = findChokePoint( last_minima );

				m_data.m_chokepoint_storage.emplace_back( std::make_unique<SkynetChokepoint>( last_minima, choke_sides.first, choke_sides.second, choke_size ) );
				SkynetChokepoint *current_chokepoint = m_data.m_chokepoint_storage.back().get();
				m_data.m_chokepoints.push_back( current_chokepoint );

				current_chokepoint->setRegion1( current_region );
				current_region->addChokepoint( current_chokepoint );

				std::set<WalkPosition> choke_children;
				MapUtil::forEachPositionInLine( choke_sides.second, choke_sides.first, [this, &tile_to_children, &current_region_tile, &choke_tiles, &choke_children, &current_chokepoint]( WalkPosition line_pos )
				{
					if( line_pos.x >= 0 && line_pos.y >= 0 && line_pos.x < m_map_size.x && line_pos.y < m_map_size.y && m_data.m_tile_clearance[line_pos] != 0 && !m_data.m_tile_to_region[line_pos] )
					{
						tile_to_children[current_region_tile].push_back( line_pos );
						choke_tiles[line_pos] = current_chokepoint;
						choke_children.insert( line_pos );
					}

					return false;
				} );

				while( !choke_children.empty() )
				{
					auto current_tile = choke_children.begin();

					tile_to_last_minima.erase( *current_tile );
					unvisited_tiles.erase( *current_tile );

					for( auto next_tile : tile_to_children[*current_tile] )
						choke_children.insert( next_tile );

					tile_to_children.erase( *current_tile );

					choke_children.erase( current_tile );
				}
			}
			else
			{
				if( m_data.m_tile_clearance[current_tile] < choke_size )
					last_minima = current_tile;

				for( int i = 0; i < 4; ++i )
				{
					WalkPosition next_tile(
						(i == 0 ? current_tile.x - 1 : (i == 1 ? current_tile.x + 1 : current_tile.x)),
						(i == 2 ? current_tile.y - 1 : (i == 3 ? current_tile.y + 1 : current_tile.y)) );

					if( next_tile.x < 0 || next_tile.y < 0 || next_tile.x >= m_map_size.x || next_tile.y >= m_map_size.y )
						continue;

					if( m_data.m_tile_clearance[next_tile] == 0 )
						continue;

					if( tile_to_last_minima.count( next_tile ) == 0 )
					{
						tile_to_last_minima[next_tile] = last_minima;
						tile_to_children[current_tile].push_back( next_tile );

						unvisited_tiles.set( next_tile, m_data.m_tile_clearance[next_tile] );
					}
				}
			}
		}

		std::set<WalkPosition> tile_steps;
		tile_steps.insert( current_region_tile );
		int region_size = 1;

		while( !tile_steps.empty() )
		{
			auto current_tile = tile_steps.begin();
			++region_size;

			for( auto next_tile : tile_to_children[*current_tile] )
				tile_steps.insert( next_tile );

			m_data.m_tile_to_region[*current_tile] = current_region;

			tile_steps.erase( current_tile );
		}

		current_region->setSize( region_size );
	}
}

std::pair<WalkPosition, WalkPosition> SkynetTerrainAnalyser::Process::findChokePoint( WalkPosition center ) const
{
	const WalkPosition side1 = m_data.m_tile_to_closest_obstacle[center];

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

		WalkPosition side2 = m_data.m_tile_to_closest_obstacle[pos];

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

void SkynetTerrainAnalyser::Process::createBases()
{
	//Group them into clusters
	std::vector<UnitGroup> resource_clusters = m_resources.getClusters( 260, 3 );

	for( const UnitGroup &resource_cluster : resource_clusters )
	{
		TilePosition base_location;
		int best_rating;
		MapUtil::spiralSearch( TilePosition( resource_cluster.getCenter() ), [&resource_cluster, &base_location, best_rating] ( TilePosition location ) mutable -> bool
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

		bool added_to_other = false;
		for( auto &other_base_location : m_data.m_base_location_storage )
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
			SkynetRegion *base_region = m_data.m_tile_to_region[WalkPosition(base_location)];
			m_data.m_base_location_storage.emplace_back( std::make_unique<SkynetBaseLocation>( base_location, base_region, resource_cluster ) );
			m_data.m_base_locations.push_back( m_data.m_base_location_storage.back().get() );
			base_region->addBase( m_data.m_base_locations.back() );
		}
	}
}

UnitGroup SkynetTerrainAnalyser::getResources()
{
	UnitGroup resources = getUnitTracker().getGeysers();
	for( Unit mineral : getUnitTracker().getMinerals() )
	{
		if( mineral->getResources() > 200 )
			resources.insert( mineral );
	}

	return resources;
}

void SkynetTerrainAnalyser::check_data()
{
	m_old_regions.clear();
	m_old_chokepoints.clear();
	m_old_base_locations.clear();

	if( m_async_future.valid() )
	{
		if( m_async_future.wait_for( std::chrono::milliseconds( 2 ) ) == std::future_status::ready )
		{
			m_old_regions = std::move( m_data.m_region_storage );
			for( auto & region : m_old_regions ) region->markInvalid();

			m_old_chokepoints = std::move( m_data.m_chokepoint_storage );
			for( auto & chokepoint : m_old_chokepoints ) chokepoint->markInvalid();

			m_old_base_locations = std::move( m_data.m_base_location_storage );
			for( auto & base_location : m_old_base_locations ) base_location->markInvalid();

			m_data = std::move( *m_async_future.get() );
			postMessage<TerrainAnalysed>();
		}
	}

	if( !m_async_future.valid() && m_data.m_request < m_reprocess_request )
	{
		UnitGroup resources = getResources();
		int request = m_reprocess_request;
		auto map_size = m_map_size;
		m_async_future = std::async( std::launch::async, [resources, request, map_size] () -> std::unique_ptr<Data>
		{
			auto new_data = std::make_unique<Data>();
			new_data->m_request = request;

			Process( map_size, *new_data, resources );

			return new_data;
		} );
	}
}