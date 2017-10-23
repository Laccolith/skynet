#pragma once

#include "SkynetTaskRequirement.h"

#include "UnitGroup.h"

class SkynetTaskRequirementUnitType : public SkynetTaskRequirementUnit
{
public:
	SkynetTaskRequirementUnitType( UnitType unit_type, int duration = max_time, Position starting_position = Positions::None, Position ending_position = Positions::None );
	SkynetTaskRequirementUnitType( UnitType unit_type, Position starting_position = Positions::None, Position ending_position = Positions::None );

	int getReserveEarliestTime( CoreAccess & access, int current_earliest_time, int & travel_time ) override;

	Unit getChosenUnit() const override;

private:
	UnitType m_unit_type;
	int m_duration;
	Position m_starting_position;
	Position m_ending_position;

	Unit m_chosen_unit = nullptr;

	int chooseUnit( CoreAccess & access, int current_earliest_time, int & travel_time, const UnitGroup & applicable_units );
};
