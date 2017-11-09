#include "LazyQuery.h"

#include "UnitTracker.h"
#include "PlayerTracker.h"

#include <random>

Value<double> LazyQuery::randomUnary()
{
	std::random_device rd;
	std::mt19937 gen( rd() );
	std::uniform_real_distribution<> dis( 0.0, 1.0 );

	return Value<double>( [num = dis(gen)]( CoreAccess & access ) -> double
	{
		return num;
	} );
}

Value<int> LazyQuery::numStartPositions()
{
	return Value<int>( []( CoreAccess & access ) -> int
	{
		return BWAPI::Broodwar->getStartLocations().size();
	} );
}

Value<Race> LazyQuery::enemyRace()
{
	return Value<Race>( []( CoreAccess & access ) -> bool
	{
		return access.getPlayerTracker().getEnemyPlayer()->getRace();
	} );
}

Value<int> LazyQuery::numEnemies()
{
	return Value<int>( []( CoreAccess & access ) -> int
	{
		return access.getPlayerTracker().getLocalPlayer()->getEnemies().size();
	} );
}

Value<int> LazyQuery::playerUnitCount( UnitType unit_type )
{
	return Value<int>( [unit_type]( CoreAccess & access ) -> int
	{
		return access.getUnitTracker().getAllUnits( access.getPlayerTracker().getLocalPlayer() ).size();
	} );
}

Value<int> LazyQuery::enemyUnitCount( UnitType unit_type )
{
	return Value<int>( [unit_type]( CoreAccess & access ) -> int
	{
		return access.getUnitTracker().getAllUnits( access.getPlayerTracker().getEnemyPlayer() ).size();
	} );
}

Condition LazyQuery::isResearching( TechType tech_type )
{
	return Condition( [tech_type]( CoreAccess & access ) -> bool
	{
		return BWAPI::Broodwar->self()->isResearching( tech_type ) || BWAPI::Broodwar->self()->hasResearched( tech_type );
	} );
}

Condition LazyQuery::hasResearching( TechType tech_type )
{
	return Condition( [tech_type]( CoreAccess & access ) -> bool
	{
		return BWAPI::Broodwar->self()->hasResearched( tech_type );
	} );
}

Condition LazyQuery::isUpgrading( UpgradeType upgrade_type, int level )
{
	return Condition( [upgrade_type, level]( CoreAccess & access ) -> bool
	{
		return BWAPI::Broodwar->self()->isUpgrading( upgrade_type ) && (level == 1 || BWAPI::Broodwar->self()->getUpgradeLevel( upgrade_type ) == level - 1);
	} );
}

Value<int> LazyQuery::getUpgradeLevel( UpgradeType upgrade_type )
{
	return Value<int>( [upgrade_type]( CoreAccess & access ) -> int
	{
		return BWAPI::Broodwar->self()->getUpgradeLevel( upgrade_type );
	} );
}