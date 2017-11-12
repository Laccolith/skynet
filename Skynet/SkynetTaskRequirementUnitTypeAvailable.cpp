#include "SkynetTaskRequirementUnitTypeAvailable.h"

#include "UnitTracker.h"
#include "UnitManager.h"
#include "PlayerTracker.h"

SkynetTaskRequirementUnitTypeAvailable::SkynetTaskRequirementUnitTypeAvailable( UnitType unit_type )
	: m_unit_type( unit_type )
{
}

int SkynetTaskRequirementUnitTypeAvailable::getEarliestTime( CoreAccess & access )
{
	int earliest_time = max_time;
	
	for( Unit unit : access.getUnitTracker().getAllUnits( m_unit_type, access.getPlayerTracker().getLocalPlayer() ) )
	{
		if( unit->isCompleted() )
			return 0;

		int completed_time = unit->getTimeTillCompleted();
		if( earliest_time > completed_time )
			earliest_time = completed_time;
	}

	int ealiest_output_time = access.getUnitManager().earliestUnitOutputTime( m_unit_type );
	if( earliest_time > ealiest_output_time )
		earliest_time = ealiest_output_time;

	return earliest_time;
}
