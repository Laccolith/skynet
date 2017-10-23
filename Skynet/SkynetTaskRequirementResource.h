#pragma once

#include "SkynetTaskRequirement.h"

class SkynetTaskRequirementMineral : public SkynetTaskRequirement
{
public:
	SkynetTaskRequirementMineral( int amount );

	int getEarliestTime( CoreAccess & access ) override;
	void reserveTime( CoreAccess & access, int time ) override;

private:
	int m_amount;
};

class SkynetTaskRequirementGas : public SkynetTaskRequirement
{
public:
	SkynetTaskRequirementGas( int amount );

	int getEarliestTime( CoreAccess & access ) override;
	void reserveTime( CoreAccess & access, int time ) override;

private:
	int m_amount;
};

class SkynetTaskRequirementSupply : public SkynetTaskRequirement
{
public:
	SkynetTaskRequirementSupply( int amount );

	int getEarliestTime( CoreAccess & access ) override;
	void reserveTime( CoreAccess & access, int time ) override;

private:
	int m_amount;
};
