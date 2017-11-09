#pragma once

#include "LazyLogic.h"

#include "Types.h"

namespace LazyQuery
{
	Value<double> randomUnary();

	Value<int> numStartPositions();

	Value<Race> enemyRace();
	Value<int> numEnemies();

	Value<int> playerUnitCount( UnitType unit_type );
	Value<int> enemyUnitCount( UnitType unit_type );

	Condition isResearching( TechType tech_type );
	Condition hasResearching( TechType tech_type );
	Condition isUpgrading( UpgradeType upgrade_type, int level = 1 );
	Value<int> getUpgradeLevel( UpgradeType upgrade_type );
}