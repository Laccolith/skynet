#pragma once

#include "SkynetTaskRequirement.h"

#include "Types.h"

class SkynetTaskRequirementUnitTypeAvailable : public SkynetTaskRequirement
{
public:
	SkynetTaskRequirementUnitTypeAvailable( UnitType unit_type );

	int getEarliestTime( CoreAccess & access ) override;
	void reserveTime( CoreAccess & access, int time ) override {}
	void freeReserved( CoreAccess & access ) override {}

private:
	UnitType m_unit_type;
};