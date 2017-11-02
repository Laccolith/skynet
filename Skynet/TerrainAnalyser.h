#pragma once

#include "CoreModule.h"

#include "Messaging.h"
#include "Types.h"

#include <vector>

struct TerrainAnalysed
{
};

class TerrainAnalyserInterface : public CoreModule, public MessageReporter<TerrainAnalysed>
{
public:
	TerrainAnalyserInterface( Core & core ) : CoreModule( core, "TerrainAnalyser" ) {}

	virtual const std::vector<Region> & getRegions() const = 0;
	virtual const std::vector<Chokepoint> & getChokepoints() const = 0;
	virtual const std::vector<BaseLocation> & getBaseLocations() const = 0;

	virtual Region getRegion( WalkPosition pos ) const = 0;
	virtual int getClearance( WalkPosition pos ) const = 0;
	virtual int getConnectivity( WalkPosition pos ) const = 0;
	virtual WalkPosition getClosestObstacle( WalkPosition pos ) const = 0;

	virtual int getGroundDistance( WalkPosition start, WalkPosition end ) const = 0;
};