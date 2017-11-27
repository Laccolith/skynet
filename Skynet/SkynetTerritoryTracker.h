#pragma once

#include "TerritoryTracker.h"

#include "SkynetEnemyTerritory.h"
#include "SkynetEnemyOutline.h"
#include "RectangleArray.h"

class SkynetTerritoryTracker : public TerritoryTrackerInterface
{
public:
	SkynetTerritoryTracker( Core & core );

	const std::vector<const EnemyTerritory*>& getEnemyTerritories() const override { return m_enemy_territories; }

	const EnemyTerritory* getEnemyTerritory( WalkPosition position ) const override;
	const EnemyOutline* getEnemyOutline( WalkPosition position ) const override;

	void update();

private:
	std::vector<const EnemyTerritory*> m_enemy_territories;
	std::vector<std::unique_ptr<SkynetEnemyTerritory>> m_enemy_territories_storage;

	std::vector<const EnemyOutline*> m_enemy_outlines;
	std::vector<std::unique_ptr<SkynetEnemyOutline>> m_enemy_outlines_storage;

	RectangleArray<int, WALKPOSITION_SCALE> m_territories;
	RectangleArray<int, WALKPOSITION_SCALE> m_outlines;

	DEFINE_DEBUGGING_INTERFACE( Default, Analysis );
};
