#pragma once

#include "CoreAccess.h"
#include "Types.h"

#include <limits>

class SkynetTaskRequirement
{
public:
	virtual int getEarliestTime( CoreAccess & access ) = 0;
	virtual void reserveTime( CoreAccess & access, int time ) = 0;
	virtual void freeReserved( CoreAccess & access ) = 0;

	virtual ~SkynetTaskRequirement() {}
};

class SkynetTaskRequirementUnit
{
public:
	virtual int getReserveEarliestTime( CoreAccess & access, int current_earliest_time ) = 0;
	virtual void freeReserved( CoreAccess & access ) = 0;

	virtual Unit getChosenUnit() const = 0;

	virtual ~SkynetTaskRequirementUnit() {}
};
