#pragma once

#include "SkynetTaskRequirement.h"

class SkynetTaskRequirementSupply : public SkynetTaskRequirement
{
public:
	SkynetTaskRequirementSupply( int amount );

	int getEarliestTime( CoreAccess & access ) override;
	void reserveTime( CoreAccess & access, int time ) override;

private:
	int m_amount;
};
