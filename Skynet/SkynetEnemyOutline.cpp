#include "SkynetEnemyOutline.h"

#include "SkynetTerritoryTracker.h"

SkynetEnemyOutline::SkynetEnemyOutline( SkynetTerritoryTracker & territory_tracker, int id, const EnemyTerritory & territory, std::vector<WalkPosition> positions )
	: m_territory_tracker( territory_tracker )
	, m_id( id )
	, m_territory( territory )
	, m_positions ( std::move( positions ) )
{
}

bool SkynetEnemyOutline::isOnOutline( WalkPosition walk_position ) const
{
	return m_territory_tracker.getEnemyOutline( walk_position ) == this;
}
