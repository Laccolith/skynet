#pragma once

#include "SkynetInterface.h"

#include "SmartEnum.h"

SMART_ENUM( UnitAccessType, int,
	Dead,
	Lost,
	Guess,
	Prediction,
	Partial,
	Full
	);

class Unit
{
public:
	virtual TilePosition getTilePosition() const = 0;
	virtual TilePosition getTilePosition( int in_frames_time ) const = 0;
	virtual WalkPosition getWalkPosition() const = 0;
	virtual WalkPosition getWalkPosition( int in_frames_time ) const = 0;
	virtual Position getPosition() const = 0;
	virtual Position getPosition( int in_frames_time ) const = 0;

	virtual Position getTargetPosition() const = 0;
};

class UnitTrackerInterface : public SkynetInterface
{
public:
	UnitTrackerInterface( Access & access ) : SkynetInterface( access ) {}

	virtual void update() = 0;
};