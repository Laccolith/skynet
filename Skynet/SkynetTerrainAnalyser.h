#pragma once

#include "TerrainAnalyserInterface.h"

class SkynetTerrainAnalyser : public TerrainAnalyserInterface
{
public:
	SkynetTerrainAnalyser( Access & access );

	virtual void update();
};