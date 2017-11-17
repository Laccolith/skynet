#pragma once

#include "CoreModule.h"

class TerritoryTrackerInterface : public CoreModule
{
public:
	TerritoryTrackerInterface( Core & core ) : CoreModule( core, "TerritoryTracker" ) {}
};
