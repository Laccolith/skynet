#include "SkynetTerritoryTracker.h"

#include "BaseTracker.h"
#include "PlayerTracker.h"
#include "UnitTracker.h"
#include "RectangleArray.h"
#include "MapUtil.h"

#include "Window.h"

#include <queue>
#include <thread>

SkynetTerritoryTracker::SkynetTerritoryTracker( Core & core )
	: TerritoryTrackerInterface( core )
{
	core.registerUpdateProcess( 4.0f, [this]() { update(); } );
}

void SkynetTerritoryTracker::update()
{
	Position map_size( BWAPI::Broodwar->mapWidth() * 32, BWAPI::Broodwar->mapHeight() * 32 );
	Position walk_map_size( BWAPI::Broodwar->mapWidth() * 4, BWAPI::Broodwar->mapHeight() * 4 );

	static RectangleArray<bool, WALKPOSITION_SCALE> tile_danger( walk_map_size.x, walk_map_size.y );
	static RectangleArray<bool, WALKPOSITION_SCALE> tile_edge( walk_map_size.x, walk_map_size.y );

	static bool ran = false;

	if( !ran )
	{
		ran = true;

		RectangleArray<bool, WALKPOSITION_SCALE> tile_visited( walk_map_size.x, walk_map_size.y );

		static std::unique_ptr<Window> debug_window = std::make_unique<Window>( "Danger Zone", map_size.x, map_size.y );

		for( Unit unit : getUnitTracker().getAllUnits( getPlayerTracker().getEnemyPlayer() ) )
		{
			UnitType unit_type = unit->getType();
			Position unit_pos = unit->getPosition();

			if( unit_type.groundWeapon() == WeaponTypes::None )
				continue;

			int range = unit_type.groundWeapon().maxRange();

			if( unit_type == UnitTypes::Terran_Vulture_Spider_Mine )
				range = unit_type.seekRange();

			WalkPosition unit_walk_pos = unit->getWalkPosition();
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

		std::queue<WalkPosition> unvisited_tiles;

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

		auto for_each_neightbour = [walk_map_size]( WalkPosition tile, auto func )
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

		auto for_each_other_neightbour = [walk_map_size]( WalkPosition tile, auto func )
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

		std::queue<WalkPosition> uncategorised_edges;

		while( !unvisited_tiles.empty() )
		{
			const WalkPosition &tile = unvisited_tiles.front();

			for_each_neightbour( tile, [tile, &tile_visited, &unvisited_tiles, &uncategorised_edges]( WalkPosition next_tile )
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

			unvisited_tiles.pop();
		}

		for( int x = 0; x < walk_map_size.x; ++x )
		{
			for( int y = 0; y < walk_map_size.y; ++y )
			{
				WalkPosition tile{ x, y };

				if( !tile_danger[tile] && !tile_edge[tile] )
				{
					int num_edges = 0;
					for_each_neightbour( tile, [&num_edges]( WalkPosition next_tile )
					{
						if( tile_edge[next_tile] )
							++num_edges;
					} );

					int num_danger_edges = 0;
					for_each_other_neightbour( tile, [&num_danger_edges]( WalkPosition next_tile )
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
				
				if( tile_edge[tile] )
				{
					//debug_window->addBox( Position( x * 8, y * 8 ), Position( x * 8 + 8, y * 8 + 8 ), Colors::Orange );
				}
				else if( tile_danger[tile] )
				{
					debug_window->addBox( Position( x * 8, y * 8 ), Position( x * 8 + 8, y * 8 + 8 ), Colors::Red );
				}
			}
		}

		RectangleArray<bool, WALKPOSITION_SCALE> categorised_edges( walk_map_size.x, walk_map_size.y );

		while( !uncategorised_edges.empty() )
		{
			WalkPosition tile = uncategorised_edges.front();
			uncategorised_edges.pop();

			if( categorised_edges[tile] )
				continue;

			WalkPosition connected_edge_1 = WalkPositions::None;
			WalkPosition connected_edge_2 = WalkPositions::None;

			auto one_step = [&]( WalkPosition tile )
			{
				for_each_neightbour( tile, [&]( WalkPosition neighbour_tile )
				{
					if( !tile_edge[neighbour_tile] || categorised_edges[neighbour_tile] )
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

			categorised_edges[tile] = true;
			debug_window->addBox( Position( tile.x * 8, tile.y * 8 ), Position( tile.x * 8 + 8, tile.y * 8 + 8 ), Colors::Orange );

			WalkPosition first_corner = WalkPositions::None;
			WalkPosition last_corner = WalkPositions::None;

			std::vector<WalkPosition> edge_points;

			auto update_direction = [&]()
			{
				for(;;)
				{
					WalkPosition next_tile = connected_edge_1;

					debug_window->addBox( Position( next_tile.x * 8, next_tile.y * 8 ), Position( next_tile.x * 8 + 8, next_tile.y * 8 + 8 ), Colors::Orange );

					categorised_edges[next_tile] = true;
					connected_edge_1 = WalkPositions::None;
					one_step( next_tile );

					if( connected_edge_1 == WalkPositions::None )
					{
						if( first_corner == WalkPositions::None )
							first_corner = next_tile;

						break;
					}
				}
			};

			update_direction();

			if( !categorised_edges[connected_edge_2] )
			{
				last_corner = first_corner;
				connected_edge_1 = connected_edge_2;
				update_direction();
			}
		}

		for( Unit unit : getUnitTracker().getAllUnits( getPlayerTracker().getEnemyPlayer() ) )
		{
			UnitType unit_type = unit->getType();
			Position unit_pos = unit->getPosition();

			debug_window->addBox( Position( unit_pos.x - unit_type.dimensionLeft(), unit_pos.y - unit_type.dimensionUp() ), Position( unit_pos.x + unit_type.dimensionRight(), unit_pos.y + unit_type.dimensionDown() ), Colors::Blue );
		}
	}

	WalkPosition top_left( BWAPI::Broodwar->getScreenPosition() );
	WalkPosition bottom_right( top_left + WalkPosition( Position( 640, 480 ) ) );

	if( bottom_right.x > walk_map_size.x ) bottom_right.x = walk_map_size.x;
	if( bottom_right.y > walk_map_size.y ) bottom_right.y = walk_map_size.y;

	MapUtil::forEachPosition( top_left, bottom_right, []( WalkPosition pos )
	{
		if( tile_edge[pos] )
		{
			BWAPI::Broodwar->drawBoxMap( pos.x * 8, pos.y * 8, pos.x * 8 + 7, pos.y * 8 + 7, Colors::Orange );
		}
		else if( tile_danger[pos] )
		{
			BWAPI::Broodwar->drawBoxMap( pos.x * 8, pos.y * 8, pos.x * 8 + 7, pos.y * 8 + 7, Colors::Red );
		}
	} );
}
