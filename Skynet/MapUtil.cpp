#include "MapUtil.h"

bool MapUtil::isAnyVisible( TilePosition location, UnitType type )
{
	for( int x = location.x; x < location.x + type.tileWidth(); ++x )
	{
		for( int y = location.y; y < location.y + type.tileHeight(); ++y )
		{
			if( BWAPI::Broodwar->isVisible( x, y ) )
				return true;
		}
	}

	return false;
}

bool MapUtil::isAllVisible( TilePosition location, UnitType type )
{
	for( int x = location.x; x < location.x + type.tileWidth(); ++x )
	{
		for( int y = location.y; y < location.y + type.tileHeight(); ++y )
		{
			if( !BWAPI::Broodwar->isVisible( x, y ) )
				return false;
		}
	}

	return true;
}

bool MapUtil::isTileWalkable( TilePosition location )
{
	for( int nx = location.x * 4; nx < location.x * 4 + 4; ++nx )
	{
		for( int ny = location.y * 4; ny < location.y * 4 + 4; ++ny )
		{
			if( !BWAPI::Broodwar->isWalkable( nx, ny ) )
				return false;
		}
	}

	return true;
}

int MapUtil::getDistance( const Position &pos1, const UnitType type1, const Position &pos2, const UnitType type2 )
{
	const int left = pos1.x - type1.dimensionLeft();
	const int top = pos1.y - type1.dimensionUp();
	const int right = pos1.x + type1.dimensionRight() + 1;
	const int bottom = pos1.y + type1.dimensionDown() + 1;

	const int targLeft = pos2.x - type2.dimensionLeft();
	const int targTop = pos2.y - type2.dimensionUp();
	const int targRight = pos2.x + type2.dimensionRight() + 1;
	const int targBottom = pos2.y + type2.dimensionDown() + 1;

	int xDist = left - targRight;
	if( xDist < 0 )
	{
		xDist = targLeft - right;
		if( xDist < 0 )
			xDist = 0;
	}

	int yDist = top - targBottom;
	if( yDist < 0 )
	{
		yDist = targTop - bottom;
		if( yDist < 0 )
			yDist = 0;
	}

	return Position( 0, 0 ).getApproxDistance( Position( xDist, yDist ) );
}

int MapUtil::getDistance( const Position &pos1, const UnitType type1, const Position &pos2 )
{
	const int left = pos1.x - type1.dimensionLeft();
	const int top = pos1.y - type1.dimensionUp();
	const int right = pos1.x + type1.dimensionRight() + 1;
	const int bottom = pos1.y + type1.dimensionDown() + 1;

	int xDist = left - (pos2.x + 1);
	if( xDist < 0 )
	{
		xDist = pos2.x - right;
		if( xDist < 0 )
			xDist = 0;
	}

	int yDist = top - (pos2.y + 1);
	if( yDist < 0 )
	{
		yDist = pos2.y - bottom;
		if( yDist < 0 )
			yDist = 0;
	}

	return Position( 0, 0 ).getApproxDistance( Position( xDist, yDist ) );
}

TilePosition MapUtil::spiralSearch( TilePosition startLocation, std::function<bool( TilePosition )> testFunction, int maxRadius )
{
	int x = startLocation.x;
	int y = startLocation.y;

	int length = 1;
	int j = 0;
	bool first = true;
	int dx = 0;
	int dy = 1;

	if( maxRadius == 0 )
		maxRadius = std::max( BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight() );

	while( length < maxRadius )
	{
		if( x >= 0 && x < BWAPI::Broodwar->mapWidth() && y >= 0 && y < BWAPI::Broodwar->mapHeight() )
		{
			TilePosition location( x, y );
			if( testFunction( location ) )
				return location;
		}

		x = x + dx;
		y = y + dy;

		//count how many steps we take in this direction
		++j;
		if( j == length ) //if we've reached the end, its time to turn
		{
			//reset step counter
			j = 0;

			//Spiral out. Keep going.
			if( !first )
				++length; //increment step counter if needed

						  //first=true for every other turn so we spiral out at the right rate
			first = !first;

			//turn counter clockwise 90 degrees:
			if( dx == 0 )
			{
				dx = dy;
				dy = 0;
			}
			else
			{
				dy = -dx;
				dx = 0;
			}
		}
		//Spiral out. Keep going.
	}

	return TilePositions::None;
}

std::set<TilePosition> MapUtil::floodFill( TilePosition start, std::function<bool( TilePosition )> testFunction, std::set<TilePosition> targets, std::set<TilePosition> ignoreTiles )
{
	bool noTargets = targets.empty();

	std::set<TilePosition> remainingTiles;
	remainingTiles.insert( start );

	while( !remainingTiles.empty() )
	{
		if( !noTargets && targets.empty() )
			return targets;

		TilePosition tile = *remainingTiles.begin();

		ignoreTiles.insert( tile );
		remainingTiles.erase( tile );

		if( !tile.isValid() )
			continue;

		if( !testFunction( tile ) )
			continue;

		if( noTargets )
			targets.insert( tile );
		else
			targets.erase( tile );

		if( ignoreTiles.count( TilePosition( tile.x + 1, tile.y ) ) == 0 )
			remainingTiles.insert( TilePosition( tile.x + 1, tile.y ) );
		if( ignoreTiles.count( TilePosition( tile.x - 1, tile.y ) ) == 0 )
			remainingTiles.insert( TilePosition( tile.x - 1, tile.y ) );
		if( ignoreTiles.count( TilePosition( tile.x, tile.y + 1 ) ) == 0 )
			remainingTiles.insert( TilePosition( tile.x, tile.y + 1 ) );
		if( ignoreTiles.count( TilePosition( tile.x, tile.y - 1 ) ) == 0 )
			remainingTiles.insert( TilePosition( tile.x, tile.y - 1 ) );
	}

	return targets;
}

std::map<WalkPosition, int> MapUtil::walkSearch( WalkPosition start, std::function<bool( WalkPosition )> testFunction, std::set<WalkPosition> targets, std::set<WalkPosition> ignoreTiles )
{
	std::map<WalkPosition, int> returnDistances;

	/*int mapWidth = BWAPI::Broodwar->mapWidth() * 4;
	int mapHeight = BWAPI::Broodwar->mapHeight() * 4;

	Heap<WalkPosition, int> openTiles( true );
	std::map<WalkPosition, int> gmap;
	std::set<WalkPosition> closedTiles;

	openTiles.push( std::make_pair( start, 0 ) );
	gmap[start] = 0;

	int maxhvalue = std::max( mapHeight, mapWidth );

	while( !openTiles.empty() )
	{
		WalkPosition p = openTiles.top().first;

		int fvalue = openTiles.top().second;
		int gvalue = gmap[p];

		openTiles.pop();
		closedTiles.insert( p );
		returnDistances[p] = gvalue;

		for( int i = 0; i < 4; ++i )
		{
			int x = i == 0 ? 1 : i == 1 ? -1 : 0;
			int y = i == 2 ? 1 : i == 3 ? -1 : 0;
			WalkPosition tile( p.x + x, p.y + y );

			if( tile.x < 0 || tile.y < 0 || tile.x >= mapWidth || tile.y >= mapHeight )
				continue;

			if( closedTiles.find( tile ) != closedTiles.end() )
				continue;

			if( !testFunction( tile ) )
				continue;

			int g = gvalue + 1;

			int f = g;
			if( gmap.find( tile ) == gmap.end() || gmap[tile] > g )
			{
				gmap[tile] = g;
				openTiles.set( tile, f );
			}
		}
	}*/

	return returnDistances;
}