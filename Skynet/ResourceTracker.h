#pragma once

#include "CoreModule.h"

class ResourceTrackerInterface : public CoreModule
{
public:
	ResourceTrackerInterface( Core & core ) : CoreModule( core, "ResourceTracker" ) {}
};