#pragma once

#include "CoreAccess.h"
#include "Types.h"

#include <limits>

const int requirement_max_time = std::numeric_limits<int>::max();

class SkynetTaskRequirement
{
public:
	virtual int getEarliestTime( CoreAccess & access ) = 0;
	virtual void reserveTime( CoreAccess & access, int time ) = 0;
};

class SkynetTaskRequirementUnit
{
public:
	virtual int getReserveEarliestTime( int current_earliest_time, int & travel_time ) = 0;

	virtual Unit getChosenUnit() = 0;
};
