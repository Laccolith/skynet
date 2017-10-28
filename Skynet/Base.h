#pragma once

#include "Types.h"

#include "UnitGroup.h"

class BaseInterface
{
public:
	virtual Position getCenterPosition() const = 0;
	virtual TilePosition getBuildPosition() const = 0;
	virtual BaseLocation getLocation() const = 0;
	virtual Region getRegion() const = 0;

	virtual const UnitGroup &getMinerals() const = 0;
	virtual const UnitGroup &getGeysers() const = 0;
	virtual const UnitGroup &getRefineries() const = 0;

	virtual Unit getResourceDepot() const = 0;
	virtual bool isActive( int frame_time = 0 ) const = 0;

	virtual bool isStartLocation() const = 0;

	virtual const UnitGroup &getBuildings() const = 0;

	virtual Player getPlayer() const = 0;

	virtual bool isEnemyBase() const = 0;
	virtual bool isMyBase() const = 0;
	virtual bool isAllyBase() const = 0;
	virtual bool isContested() const = 0;

	virtual void draw() const = 0;

	virtual ~BaseInterface() {}
};