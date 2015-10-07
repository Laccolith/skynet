#pragma once

#include "SkynetInterface.h"

class TerrainAnalyserInterface : public SkynetInterface
{
public:
	TerrainAnalyserInterface( Access & access ) : SkynetInterface( access ) {}

	virtual void update() = 0;
};