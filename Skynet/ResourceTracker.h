#pragma once

#include "SkynetInterface.h"

class ResourceTrackerInterface : public SkynetInterface
{
public:
	ResourceTrackerInterface( Access &access ) : SkynetInterface( access, "ResourceTracker" ) {}
};