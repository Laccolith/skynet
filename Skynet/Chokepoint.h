#pragma once

#include "Types.h"

class ChokepointInterface
{
public:
	virtual const WalkPosition &getCenter() const = 0;
	virtual const std::pair<WalkPosition, WalkPosition> &getSides() const = 0;
	virtual int getClearance() const = 0;

	virtual const std::pair<Region, Region> &getRegions() const = 0;

	virtual void draw( Color color ) const = 0;
};