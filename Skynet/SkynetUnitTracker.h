#pragma once

#include "UnitTrackerInterface.h"

class SkynetUnitTracker : public UnitTrackerInterface
{
public:
	SkynetUnitTracker( Access & access );

	virtual void update();
};