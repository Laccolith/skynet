#pragma once

#include "Types.h"

#include <set>
#include <functional>

namespace MapUtil
{
	TilePosition spiralSearch( TilePosition startLocation, std::function<bool( TilePosition )> testFunction, int maxRadius = 0 );
	std::set<TilePosition> floodFill( TilePosition start, std::function<bool( TilePosition )> testFunction, std::set<TilePosition> targets = std::set<TilePosition>(), std::set<TilePosition> ignoreTiles = std::set<TilePosition>() );
	std::map<WalkPosition, int> walkSearch( WalkPosition start, std::function<bool( WalkPosition )> testFunction, std::set<WalkPosition> targets = std::set<WalkPosition>(), std::set<WalkPosition> ignoreTiles = std::set<WalkPosition>() );

	bool isAnyVisible( TilePosition location, UnitType type );
	bool isAllVisible( TilePosition location, UnitType type );

	bool isTileWalkable( TilePosition location );

	int getDistance( const Position &pos1, const UnitType type1, const Position &pos2, const UnitType type2 );
	int getDistance( const Position &pos1, const UnitType type1, const Position &pos2 );

	template <typename T, typename P>
	void forEachPositionInLine( T start_pos, T end_pos, P p, bool continue_past_end = false )
	{
		int x0 = start_pos.x;
		int y0 = start_pos.y;

		int x1 = end_pos.x;
		int y1 = end_pos.y;

		int dx = abs( x1 - x0 );
		int dy = abs( y1 - y0 );

		int sx = x0 < x1 ? 1 : -1;
		int sy = y0 < y1 ? 1 : -1;

		int error = dx - dy;

		while( true )
		{
			if( p( T( x0, y0 ) ) )
				break;

			if( !continue_past_end && x0 == x1 && y0 == y1 )
				break;

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

	template <typename T, int Scale, typename F>
	void forEachPosition( BWAPI::Point<T, Scale> top_left, BWAPI::Point<T, Scale> bottom_right, F f )
	{
		for( int x = top_left.x; x < bottom_right.x; ++x )
		{
			for( int y = top_left.y; y < bottom_right.y; ++y )
			{
				f( BWAPI::Point<T, Scale>( x, y ) );
			}
		}
	}
}