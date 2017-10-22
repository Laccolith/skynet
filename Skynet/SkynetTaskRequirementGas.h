#pragma once

#include "SkynetTaskRequirement.h"

class SkynetTaskRequirementGas : public SkynetTaskRequirement
{
public:
	SkynetTaskRequirementGas( int amount );

	int getEarliestTime( CoreAccess & access ) override;
	void reserveTime( CoreAccess & access, int time ) override;

private:
	int m_amount;
};
