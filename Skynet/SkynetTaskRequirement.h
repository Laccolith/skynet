#pragma once

#include "Types.h"

#include <limits>

const int requirement_max_time = std::numeric_limits<int>::max();

class SkynetTaskRequirement
{
public:
	virtual int getEarliestTime() = 0;
};

class SkynetTaskRequirementUnit
{
public:
	virtual int getEarliestTime( int current_earliest_time, int & travel_time ) = 0;

	virtual Unit getChosenUnit() = 0;
};
