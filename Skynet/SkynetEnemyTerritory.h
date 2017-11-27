#pragma once

#include "EnemyTerritory.h"

class SkynetTerritoryTracker;
class SkynetEnemyTerritory : public EnemyTerritory
{
public:
	SkynetEnemyTerritory( SkynetTerritoryTracker & territory_tracker, int id, WalkPosition center );

	WalkPosition getCenter() const override { return m_center; }
	const std::vector<EnemyOutline*> getOutlines() const override { return m_outlines; }

	bool isInside( WalkPosition walk_position ) const override;

private:
	SkynetTerritoryTracker & m_territory_tracker;

	int m_id;

	WalkPosition m_center;
	std::vector<EnemyOutline*> m_outlines;
};