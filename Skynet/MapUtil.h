#pragma once

#include "Types.h"

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
	void forEachTileInLine( T startPos, T endPos, bool continuePastEnd, P p )
	{
		int x0 = startPos.x;
		int y0 = startPos.y;

		int x1 = endPos.x;
		int y1 = endPos.y;

		int dx = abs( x1 - x0 );
		int dy = abs( y1 - y0 );

		int sx = x0 < x1 ? 1 : -1;
		int sy = y0 < y1 ? 1 : -1;

		int error = dx - dy;

		for( ;;)
		{
			if( p( x0, y0 ) )
				break;

			if( !continuePastEnd && x0 == x1 && y0 == y1 )
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
}