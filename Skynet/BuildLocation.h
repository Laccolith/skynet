#pragma once

#include "Types.h"

class BuildLocation
{
public:
	virtual int calculatePosition( int earliest_time ) = 0;
	virtual void reservePosition( int time ) = 0;
	virtual void freeReservation() = 0;

	virtual Position getPosition() const = 0;
	virtual TilePosition getTilePosition() const = 0;

	virtual ~BuildLocation() {}
};