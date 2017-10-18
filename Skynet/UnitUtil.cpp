#include "UnitUtil.h"

#include <set>

bool UnitUtil::hasAddon( UnitType type )
{
	return(type == UnitTypes::Terran_Command_Center
		|| type == UnitTypes::Terran_Factory
		|| type == UnitTypes::Terran_Starport
		|| type == UnitTypes::Terran_Science_Facility);
}

bool UnitUtil::isArmyUnit( UnitType type )
{
	if( type.isBuilding() || type.isSpell() || type.isWorker() || type.supplyProvided() != 0 )
		return false;

	if( type == UnitTypes::Zerg_Egg )
		return false;
	if( type == UnitTypes::Protoss_Interceptor )
		return false;
	if( type == UnitTypes::Terran_Vulture_Spider_Mine )
		return false;
	if( type == UnitTypes::Zerg_Larva )
		return false;
	if( type == UnitTypes::Protoss_Scarab )
		return false;
	if( type == UnitTypes::Protoss_Observer )
		return false;

	return true;
}

bool UnitUtil::isStaticDefense( UnitType type )
{
	return(type == UnitTypes::Protoss_Photon_Cannon
		|| type == UnitTypes::Zerg_Creep_Colony
		|| type == UnitTypes::Zerg_Spore_Colony
		|| type == UnitTypes::Zerg_Sunken_Colony
		|| type == UnitTypes::Terran_Bunker
		|| type == UnitTypes::Terran_Missile_Turret);
}

bool UnitUtil::unitProducesGround( UnitType type )
{
	static std::set<UnitType> unit_data;
	if( unit_data.empty() )
	{
		for( UnitType type : UnitTypes::allUnitTypes() )
		{
			if( !type.isFlyer() && type.whatBuilds().first.isBuilding() )
				unit_data.insert( type.whatBuilds().first );
		}
	}

	return unit_data.count( type ) != 0;
}