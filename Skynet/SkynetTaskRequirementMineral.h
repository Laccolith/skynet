#pragma once

#include "SkynetTaskRequirement.h"

class SkynetTaskRequirementMineral : public SkynetTaskRequirement
{
public:
	SkynetTaskRequirementMineral( int ammount );

	int getEarliestTime() override;

private:
	int m_ammount;
};
