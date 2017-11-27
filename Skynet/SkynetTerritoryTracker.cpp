#include "SkynetTerritoryTracker.h"

#include "BaseTracker.h"
#include "PlayerTracker.h"
#include "UnitTracker.h"
#include "MapUtil.h"

#include "Window.h"

#include <queue>
#include <thread>

SkynetTerritoryTracker::SkynetTerritoryTracker( Core & core )
	: TerritoryTrackerInterface( core )
	, m_territories( BWAPI::Broodwar->mapWidth() * 4, BWAPI::Broodwar->mapHeight() * 4, -1 )
	, m_outlines( BWAPI::Broodwar->mapWidth() * 4, BWAPI::Broodwar->mapHeight() * 4, -1 )
{
	core.registerUpdateProcess( 4.0f, [this]() { update(); } );

	setDebugging( Debug::Default, true );
	//setDebugging( Debug::Analysis, true );
}

const EnemyTerritory * SkynetTerritoryTracker::getEnemyTerritory( WalkPosition position ) const
{
	int id = m_territories[position];
	if( id == -1 )
		return nullptr;

	return m_enemy_territories[id];
}

const EnemyOutline* SkynetTerritoryTracker::getEnemyOutline( WalkPosition position ) const
{
	int id = m_outlines[position];
	if( id == -1 )
		return nullptr;

	return m_enemy_outlines[id];
}

void SkynetTerritoryTracker::update()
{
	m_enemy_territories.clear();
	m_enemy_territories_storage.clear();
	m_enemy_outlines.clear();
	m_enemy_outlines_storage.clear();
	m_territories.fill( -1 );
	m_outlines.fill( -1 );

	// TODO: None of this currently considers whether tiles are walkable
	// They should be skipped completely so that they break up territory and outlines

	Position map_size( BWAPI::Broodwar->mapWidth() * 32, BWAPI::Broodwar->mapHeight() * 32 );
	Position walk_map_size( BWAPI::Broodwar->mapWidth() * 4, BWAPI::Broodwar->mapHeight() * 4 );

	RectangleArray<bool, WALKPOSITION_SCALE> tile_danger( walk_map_size.x, walk_map_size.y );
	RectangleArray<bool, WALKPOSITION_SCALE> tile_edge( walk_map_size.x, walk_map_size.y );

	RectangleArray<bool, WALKPOSITION_SCALE> tile_visited( walk_map_size.x, walk_map_size.y );

	static std::unique_ptr<Window> debug_window;

	if( isDebugging( Debug::Analysis ) )
	{
		if( !debug_window )
			debug_window = std::make_unique<Window>( "Danger Zone", map_size.x, map_size.y );

		debug_window->clear();
	}
	else
		debug_window.reset();

	std::queue<WalkPosition> unvisited_enemy_tiles;

	// For through all enemy units and mark what positions they are a danger to
	for( Unit unit : getUnitTracker().getAllUnits( getPlayerTracker().getEnemyPlayer() ) )
	{
		UnitType unit_type = unit->getType();
		Position unit_pos = unit->getPosition();

		if( unit_type.groundWeapon() == WeaponTypes::None )
			continue;

		WalkPosition unit_walk_pos = unit->getWalkPosition();

		if( tile_danger[unit_walk_pos] )
			unvisited_enemy_tiles.push( unit_walk_pos );

		int range = unit_type.groundWeapon().maxRange();

		if( unit_type == UnitTypes::Terran_Vulture_Spider_Mine )
			range = unit_type.seekRange();

		int walk_range = range / 8;

		int unit_left = unit_pos.x - unit_type.dimensionLeft();
		int unit_top = unit_pos.y - unit_type.dimensionUp();
		int unit_right = unit_pos.x + unit_type.dimensionRight();
		int unit_bottom = unit_pos.y + unit_type.dimensionDown();

		int walk_unit_left = unit_left / 8;
		int walk_unit_top = unit_top / 8;
		int walk_unit_right = unit_right / 8;
		int walk_unit_bottom = unit_bottom / 8;

		for( int x = walk_unit_left - walk_range - 1; x < walk_unit_right + walk_range + 1; ++x )
		{
			for( int y = walk_unit_top - walk_range - 1; y < walk_unit_bottom + walk_range + 1; ++y )
			{
				if( x < 0 || y < 0 || x >= walk_map_size.x || y >= walk_map_size.y )
					continue;

				WalkPosition walk_target_pos{ x, y };
				if( tile_danger[walk_target_pos] )
					continue;

				Position target_pos{ walk_target_pos };

				int left = target_pos.x - 1;
				int top = target_pos.y - 1;
				int right = target_pos.x + 1;
				int bottom = target_pos.y + 1;

				int xDist = unit_left - right;
				if( xDist < 0 )
				{
					xDist = left - unit_right;
					if( xDist < 0 )
						xDist = 0;
				}

				int yDist = unit_top - bottom;
				if( yDist < 0 )
				{
					yDist = top - unit_bottom;
					if( yDist < 0 )
						yDist = 0;
				}

				// Spider mines seem to lock on within a square
				if( unit_type == UnitTypes::Terran_Vulture_Spider_Mine || Positions::Origin.getApproxDistance( Position( xDist, yDist ) ) <= range )
				{
					tile_danger[walk_target_pos] = true;
				}
			}
		}
	}

	auto for_each_neighbour = [walk_map_size]( WalkPosition tile, auto func )
	{
		if( tile.x > 0 )
			func( WalkPosition( tile.x - 1, tile.y ) );

		if( tile.y > 0 )
			func( WalkPosition( tile.x, tile.y - 1 ) );

		if( tile.x < walk_map_size.x - 1 )
			func( WalkPosition( tile.x + 1, tile.y ) );

		if( tile.y < walk_map_size.y - 1 )
			func( WalkPosition( tile.x, tile.y + 1 ) );
	};

	auto for_each_other_neighbour = [walk_map_size]( WalkPosition tile, auto func )
	{
		if( tile.x > 0 && tile.y > 0 )
			func( WalkPosition( tile.x - 1, tile.y - 1 ) );

		if( tile.x > 0 && tile.y < walk_map_size.y - 1 )
			func( WalkPosition( tile.x - 1, tile.y + 1 ) );

		if( tile.x < walk_map_size.x - 1 && tile.y > 0 )
			func( WalkPosition( tile.x + 1, tile.y - 1 ) );

		if( tile.x < walk_map_size.x - 1 && tile.y < walk_map_size.y - 1 )
			func( WalkPosition( tile.x + 1, tile.y + 1 ) );
	};

	// Go through all dangerous tiles, grouping them up
	while( !unvisited_enemy_tiles.empty() )
	{
		WalkPosition starting_tile = unvisited_enemy_tiles.front();
		unvisited_enemy_tiles.pop();

		if( m_territories[starting_tile] != -1 )
			continue;

		int current_id = m_enemy_territories.size();

		std::queue<WalkPosition> connected_tiles;
		connected_tiles.push( starting_tile );
		m_territories[starting_tile] = current_id;

		int number_of_tiles = 0;
		WalkPosition average_position = starting_tile;

		while( !connected_tiles.empty() )
		{
			WalkPosition connected_tile = connected_tiles.front();
			connected_tiles.pop();

			for_each_neighbour( connected_tile, [this, current_id, &connected_tiles, &average_position, &number_of_tiles, &tile_danger]( WalkPosition next_tile )
			{
				if( tile_danger[next_tile] && m_territories[next_tile] == -1 )
				{
					connected_tiles.push( next_tile );
					m_territories[next_tile] = current_id;
					average_position += next_tile;
					++number_of_tiles;
				}
			} );
		}

		average_position /= number_of_tiles;

		// TODO: If this tiles isn't in the territory, spiral outward looking for the first one that is

		m_enemy_territories_storage.push_back( std::make_unique<SkynetEnemyTerritory>( *this, current_id, average_position ) );
		m_enemy_territories.push_back( m_enemy_territories_storage.back().get() );
	}

	std::queue<WalkPosition> unvisited_tiles;

	// Add all tiles that we have claim to as the starting positions for the search
	for( Unit unit : getUnitTracker().getAllUnits( getPlayerTracker().getLocalPlayer() ) )
	{
		if( unit->getType().isFlyer() )
			continue;

		WalkPosition unit_walk_pos = unit->getWalkPosition();

		if( tile_danger[unit_walk_pos] )
			continue;

		if( !tile_visited[unit_walk_pos] )
		{
			unvisited_tiles.push( unit_walk_pos );
			tile_visited[unit_walk_pos] = true;
		}
	}

	std::queue<WalkPosition> uncategorised_edges;

	// Spread outwards from the friendly positions, finding the edge tiles between the enemy territories
	while( !unvisited_tiles.empty() )
	{
		WalkPosition tile = unvisited_tiles.front();
		unvisited_tiles.pop();

		for_each_neighbour( tile, [tile, &tile_visited, &unvisited_tiles, &uncategorised_edges, &tile_danger, &tile_edge]( WalkPosition next_tile )
		{
			if( tile_danger[next_tile] )
			{
				if( !tile_edge[tile] )
				{
					tile_edge[tile] = true;
					uncategorised_edges.push( tile );
				}
			}
			else if( !tile_visited[next_tile] )
			{
				unvisited_tiles.push( next_tile );
				tile_visited[next_tile] = true;
			}
		} );
	}

	// Go through all tiles and mark the ones that are in the corners of edge tiles also as edges to form a single connected line
	for( int x = 0; x < walk_map_size.x; ++x )
	{
		for( int y = 0; y < walk_map_size.y; ++y )
		{
			WalkPosition tile{ x, y };

			if( !tile_danger[tile] && !tile_edge[tile] )
			{
				int num_edges = 0;
				for_each_neighbour( tile, [&num_edges, &tile_edge]( WalkPosition next_tile )
				{
					if( tile_edge[next_tile] )
						++num_edges;
				} );

				int num_danger_edges = 0;
				for_each_other_neighbour( tile, [&num_danger_edges, &tile_danger]( WalkPosition next_tile )
				{
					if( tile_danger[next_tile] )
						++num_danger_edges;
				} );

				if( num_edges == 2 && num_danger_edges == 1 )
				{
					tile_edge[tile] = true;
					uncategorised_edges.push( tile );
				}
			}

			if( debug_window && !tile_edge[tile] && tile_danger[tile] )
			{
				debug_window->addBox( Position( x * 8, y * 8 ), Position( x * 8 + 8, y * 8 + 8 ), Colors::Red );
			}
		}
	}

	// Create the outlines out of connected edges by stepping along them
	while( !uncategorised_edges.empty() )
	{
		WalkPosition tile = uncategorised_edges.front();
		uncategorised_edges.pop();

		if( m_outlines[tile] != -1 )
			continue;

		WalkPosition connected_edge_1 = WalkPositions::None;
		WalkPosition connected_edge_2 = WalkPositions::None;

		auto one_step = [&]( WalkPosition tile )
		{
			for_each_neighbour( tile, [&]( WalkPosition neighbour_tile )
			{
				if( !tile_edge[neighbour_tile] || m_outlines[neighbour_tile] != -1 )
					return;

				if( tile.x != neighbour_tile.x )
				{
					bool pass_1 = (neighbour_tile.y < walk_map_size.y - 1) && !tile_danger[WalkPosition( neighbour_tile.x, neighbour_tile.y + 1 )];
					bool pass_2 = (neighbour_tile.y > 0) && !tile_danger[WalkPosition( neighbour_tile.x, neighbour_tile.y - 1 )];

					if( !pass_1 && !pass_2 )
						return;
				}
				else
				{
					bool pass_1 = (neighbour_tile.x < walk_map_size.x - 1) && !tile_danger[WalkPosition( neighbour_tile.x + 1, neighbour_tile.y )];
					bool pass_2 = (neighbour_tile.x > 0) && !tile_danger[WalkPosition( neighbour_tile.x - 1, neighbour_tile.y )];

					if( !pass_1 && !pass_2 )
						return;
				}

				if( connected_edge_1 == WalkPositions::None )
					connected_edge_1 = neighbour_tile;
				else if( connected_edge_2 == WalkPositions::None )
					connected_edge_2 = neighbour_tile;
				else
				{
					//TODO: Logic error
					volatile int i = 0;
				}
			} );
		};

		one_step( tile );

		if( connected_edge_1 == WalkPositions::None || connected_edge_2 == WalkPositions::None )
			continue;

		const EnemyTerritory* enemy_territory = nullptr;
		for_each_neighbour( tile, [this, &enemy_territory]( WalkPosition neighbour_tile )
		{
			enemy_territory = getEnemyTerritory( neighbour_tile );
		} );

		if( !enemy_territory )
			continue;

		int outline_id = m_enemy_outlines.size();

		m_outlines[tile] = outline_id;
		if( debug_window )
			debug_window->addBox( Position( tile.x * 8, tile.y * 8 ), Position( tile.x * 8 + 8, tile.y * 8 + 8 ), Colors::Orange );

		std::vector<WalkPosition> edge_points;
		edge_points.push_back( tile );
		WalkPosition average_position = tile;

		auto update_direction = [&]()
		{
			for(;;)
			{
				WalkPosition next_tile = connected_edge_1;

				edge_points.push_back( next_tile );
				average_position += next_tile;

				if( debug_window )
					debug_window->addBox( Position( next_tile.x * 8, next_tile.y * 8 ), Position( next_tile.x * 8 + 8, next_tile.y * 8 + 8 ), Colors::Orange );

				m_outlines[next_tile] = outline_id;
				connected_edge_1 = WalkPositions::None;
				one_step( next_tile );

				if( connected_edge_1 == WalkPositions::None )
					break;
			}
		};

		update_direction();

		if( m_outlines[connected_edge_2] == -1 )
		{
			std::reverse( edge_points.begin(), edge_points.end() );

			connected_edge_1 = connected_edge_2;
			update_direction();
		}

		m_enemy_outlines_storage.push_back( std::make_unique<SkynetEnemyOutline>( *this, outline_id, *enemy_territory, std::move( edge_points ) ) );
		m_enemy_outlines.push_back( m_enemy_outlines_storage.back().get() );
	}

	if( debug_window )
	{
		for( Unit unit : getUnitTracker().getAllUnits( getPlayerTracker().getEnemyPlayer() ) )
		{
			UnitType unit_type = unit->getType();
			Position unit_pos = unit->getPosition();

			debug_window->addBox( Position( unit_pos.x - unit_type.dimensionLeft(), unit_pos.y - unit_type.dimensionUp() ), Position( unit_pos.x + unit_type.dimensionRight(), unit_pos.y + unit_type.dimensionDown() ), Colors::Blue );
		}
	}

	if( isDebugging( Debug::Default ) )
	{
		bool show_hover = BWAPI::Broodwar->getKeyState( BWAPI::Key( '1' ) );
		bool show_all = !show_hover;

		int show_territory = -1;
		int show_outline = -1;

		if( show_hover )
		{
			WalkPosition mouse_tile( BWAPI::Broodwar->getMousePosition() + BWAPI::Broodwar->getScreenPosition() );
			if( mouse_tile.isValid() )
			{
				show_territory = m_territories[mouse_tile];
				show_outline = m_outlines[mouse_tile];
			}
		}

		WalkPosition top_left( BWAPI::Broodwar->getScreenPosition() );
		WalkPosition bottom_right( top_left + WalkPosition( Position( 640, 480 ) ) );

		if( bottom_right.x > walk_map_size.x ) bottom_right.x = walk_map_size.x;
		if( bottom_right.y > walk_map_size.y ) bottom_right.y = walk_map_size.y;

		MapUtil::forEachPosition( top_left, bottom_right, [this, show_all, show_territory, show_outline]( WalkPosition pos )
		{
			if( show_all )
			{
				if( m_outlines[pos] != -1 )
				{
					BWAPI::Broodwar->drawBoxMap( pos.x * 8, pos.y * 8, pos.x * 8 + 7, pos.y * 8 + 7, Colors::Orange );
				}
				else if( m_territories[pos] != -1 )
				{
					BWAPI::Broodwar->drawBoxMap( pos.x * 8, pos.y * 8, pos.x * 8 + 7, pos.y * 8 + 7, Colors::Red );
				}
			}
			else if( show_territory != -1 && show_territory == m_territories[pos] )
			{
				BWAPI::Broodwar->drawBoxMap( pos.x * 8, pos.y * 8, pos.x * 8 + 7, pos.y * 8 + 7, Colors::Red );
			}
			else if( show_outline != -1 && show_outline == m_outlines[pos] )
			{
				BWAPI::Broodwar->drawBoxMap( pos.x * 8, pos.y * 8, pos.x * 8 + 7, pos.y * 8 + 7, Colors::Orange );
			}
		} );
	}
}
