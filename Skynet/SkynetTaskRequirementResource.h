#pragma once

#include "SkynetTaskRequirement.h"

class SkynetTaskRequirementMineral : public SkynetTaskRequirement
{
public:
	SkynetTaskRequirementMineral( int amount );

	int getEarliestTime( CoreAccess & access ) override;
	void reserveTime( CoreAccess & access, int time ) override;
	void freeReserved( CoreAccess & access ) override;

private:
	int m_amount;
	bool m_reserved = false;
};

class SkynetTaskRequirementGas : public SkynetTaskRequirement
{
public:
	SkynetTaskRequirementGas( int amount );

	int getEarliestTime( CoreAccess & access ) override;
	void reserveTime( CoreAccess & access, int time ) override;
	void freeReserved( CoreAccess & access ) override;

private:
	int m_amount;
	bool m_reserved = false;
};

class SkynetTaskRequirementSupply : public SkynetTaskRequirement
{
public:
	SkynetTaskRequirementSupply( int amount );

	int getEarliestTime( CoreAccess & access ) override;
	void reserveTime( CoreAccess & access, int time ) override;
	void freeReserved( CoreAccess & access ) override;

private:
	int m_amount;
	bool m_reserved = false;
};
