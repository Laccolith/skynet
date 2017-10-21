#pragma once

#include "ResourceTracker.h"

class SkynetResourceTracker : public ResourceTrackerInterface
{
public:
	SkynetResourceTracker( Core & core );

private:
	DEFINE_DEBUGGING_INTERFACE( Default );
};