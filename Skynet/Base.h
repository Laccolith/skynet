#pragma once

#include "Types.h"

#include "UnitGroup.h"

class BaseInterface
{
public:
	virtual const TilePosition &getCenterBuildLocation() const = 0;
	virtual const Position &getCenterLocation() const = 0;

	virtual const UnitGroup &getMinerals() const = 0;
	virtual const UnitGroup &getGeysers() const = 0;
	virtual const UnitGroup &getRefineries() const = 0;
};