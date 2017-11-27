#pragma once

#include "CoreModule.h"

#include "EnemyTerritory.h"
#include "EnemyOutline.h"

class TerritoryTrackerInterface : public CoreModule
{
public:
	TerritoryTrackerInterface( Core & core ) : CoreModule( core, "TerritoryTracker" ) {}

	virtual const std::vector<const EnemyTerritory*>& getEnemyTerritories() const = 0;

	virtual const EnemyTerritory* getEnemyTerritory( WalkPosition position ) const = 0;
	virtual const EnemyOutline* getEnemyOutline( WalkPosition position ) const = 0;
};
