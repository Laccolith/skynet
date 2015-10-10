#pragma once

#include "SkynetInterface.h"

#include "VectorUnique.h"
#include "Messaging.h"

struct TerrainAnalyseComplete
{
};

class TerrainAnalyserInterface : public SkynetInterface, public MessageReporter<TerrainAnalyseComplete>
{
public:
	TerrainAnalyserInterface( Access & access ) : SkynetInterface( access, "TerrainAnalyser" ) {}

	virtual const std::vector<Region> &getRegions() const = 0;
	virtual const std::vector<Chokepoint> &getChokepoints() const = 0;
	virtual const std::vector<BaseLocation> &getBaseLocations() const = 0;

	virtual Region getRegion( WalkPosition pos ) const = 0;
	virtual int getClearance( WalkPosition pos ) const = 0;
	virtual int getConnectivity( WalkPosition pos ) const = 0;
	virtual WalkPosition getClosestObstacle( WalkPosition pos ) const = 0;
};