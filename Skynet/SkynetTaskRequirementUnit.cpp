#include "SkynetTaskRequirementUnit.h"

#include "PlayerTracker.h"
#include "UnitTracker.h"
#include "UnitManager.h"

SkynetTaskRequirementUnitType::SkynetTaskRequirementUnitType( UnitType unit_type, int duration, Position starting_position, Position ending_position )
	: m_unit_type( unit_type )
	, m_duration( duration )
	, m_starting_position( starting_position )
	, m_ending_position( ending_position )
{
}

SkynetTaskRequirementUnitType::SkynetTaskRequirementUnitType( UnitType unit_type, Position starting_position, Position ending_position )
	: m_unit_type( unit_type )
	, m_duration( max_time )
	, m_starting_position( starting_position )
	, m_ending_position( ending_position )
{
}

int SkynetTaskRequirementUnitType::getReserveEarliestTime( CoreAccess & access, int current_earliest_time, int & travel_time )
{
	const UnitGroup & applicable_units = access.getUnitTracker().getAllUnits( m_unit_type, access.getPlayerTracker().getLocalPlayer() );

	if( m_starting_position == Positions::None )
		return chooseUnit( access, current_earliest_time, travel_time, applicable_units );

	UnitGroup sorted_units = applicable_units;
	sorted_units.sortByDistance( m_starting_position );

	return chooseUnit( access, current_earliest_time, travel_time, sorted_units );
}

Unit SkynetTaskRequirementUnitType::getChosenUnit() const
{
	return m_chosen_unit;
}

int SkynetTaskRequirementUnitType::chooseUnit( CoreAccess & access, int current_earliest_time, int & travel_time, const UnitGroup & applicable_units )
{
	m_chosen_unit = nullptr;

	int best_time = max_time;

	for( Unit unit : applicable_units )
	{
		// TODO: Allow choosing none existant units, but need to create a dependency to the task creating it
		// So that it doesn't cycle wether it can use or not
		// Still only choose it if it would be faster then using an existing unit
		if( !unit->exists() )
			continue;

		if( m_starting_position != Positions::None )
		{
			int earliest_time = access.getUnitManager().getAvailableTime( unit, current_earliest_time, m_duration, travel_time, m_starting_position, m_ending_position );
			if( earliest_time < best_time )
			{
				m_chosen_unit = unit;
				best_time = earliest_time;
			}
		}
		else
		{
			int earliest_time = access.getUnitManager().getAvailableTime( unit, current_earliest_time, m_duration );
			if( earliest_time < best_time )
			{
				m_chosen_unit = unit;
				best_time = earliest_time;
			}
		}

		if( best_time == current_earliest_time )
			break;
	}

	if( m_chosen_unit )
	{
		current_earliest_time = std::max( current_earliest_time, best_time );

		int end_time = (current_earliest_time > (max_time - m_duration)) ? max_time : current_earliest_time + m_duration;

		access.getUnitManager().reserveTaskUnit( m_chosen_unit, current_earliest_time, end_time, m_starting_position, m_ending_position );
	}

	return current_earliest_time;
}
