#include "SkynetEnemyTerritory.h"

#include "SkynetTerritoryTracker.h"

SkynetEnemyTerritory::SkynetEnemyTerritory( SkynetTerritoryTracker & territory_tracker, int id, WalkPosition center )
	: m_territory_tracker( territory_tracker )
	, m_id( id )
	, m_center( center )
{
}

bool SkynetEnemyTerritory::isInside( WalkPosition walk_position ) const
{
	return m_territory_tracker.getEnemyTerritory( walk_position ) == this;
}
