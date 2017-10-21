#pragma once

#include "ResourceTracker.h"

class SkynetResourceTracker : public ResourceTrackerInterface
{
public:
	SkynetResourceTracker( Access & access );

private:
	DEFINE_DEBUGGING_INTERFACE( Default );
};