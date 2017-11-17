#pragma once

#include "TerritoryTracker.h"

class SkynetTerritoryTracker : public TerritoryTrackerInterface
{
public:
	SkynetTerritoryTracker( Core & core );

	void update();
};
