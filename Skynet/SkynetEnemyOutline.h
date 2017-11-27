#pragma once

#include "EnemyOutline.h"

class SkynetTerritoryTracker;
class SkynetEnemyOutline : public EnemyOutline
{
public:
	SkynetEnemyOutline( SkynetTerritoryTracker & territory_tracker, int id, const EnemyTerritory & territory, std::vector<WalkPosition> positions );

	const EnemyTerritory & getTerritory() const override { return m_territory; }
	const std::vector<WalkPosition> & getPositions() const override { return m_positions; }

	bool isOnOutline( WalkPosition walk_position ) const override;

private:
	SkynetTerritoryTracker & m_territory_tracker;

	int m_id;

	const EnemyTerritory & m_territory;
	std::vector<WalkPosition> m_positions;
};