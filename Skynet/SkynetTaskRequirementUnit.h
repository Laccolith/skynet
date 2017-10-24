#pragma once

#include "SkynetTaskRequirement.h"

#include "UnitGroup.h"

class SkynetTaskRequirementUnitType : public SkynetTaskRequirementUnit
{
public:
	SkynetTaskRequirementUnitType( UnitType unit_type, int duration, Position starting_position = Positions::None, Position ending_position = Positions::None );
	SkynetTaskRequirementUnitType( UnitType unit_type, Position starting_position = Positions::None, Position ending_position = Positions::None );

	int getReserveEarliestTime( CoreAccess & access, int current_earliest_time ) override;
	void freeReserved( CoreAccess & access ) override;

	Unit getChosenUnit() const override;

private:
	UnitType m_unit_type;
	int m_duration;
	Position m_starting_position;
	Position m_ending_position;

	Unit m_chosen_unit = nullptr;

	int chooseUnit( CoreAccess & access, int current_earliest_time, const UnitGroup & applicable_units );
};

class SkynetTaskRequirementUnitSpecific : public SkynetTaskRequirementUnit
{
public:
	SkynetTaskRequirementUnitSpecific( Unit unit, int duration, Position starting_position = Positions::None, Position ending_position = Positions::None );
	SkynetTaskRequirementUnitSpecific( Unit unit, Position starting_position = Positions::None, Position ending_position = Positions::None );

	int getReserveEarliestTime( CoreAccess & access, int current_earliest_time ) override;
	void freeReserved( CoreAccess & access ) override;

	Unit getChosenUnit() const override;

private:
	Unit m_unit;
	int m_duration;
	Position m_starting_position;
	Position m_ending_position;

	bool m_is_reserved = false;
};
