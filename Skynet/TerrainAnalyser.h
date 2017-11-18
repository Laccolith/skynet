#pragma once

#include "CoreModule.h"

#include "Messaging.h"
#include "Types.h"

#include <vector>

struct StaticTerrainAnalysed
{
};

struct DynamicTerrainAnalysed
{
};

class TerrainAnalyserInterface : public CoreModule, public MessageReporter<StaticTerrainAnalysed, DynamicTerrainAnalysed>
{
public:
	TerrainAnalyserInterface( Core & core ) : CoreModule( core, "TerrainAnalyser" ) {}

	virtual const std::vector<Region> & getRegions( bool static_data ) const = 0;
	virtual const std::vector<Chokepoint> & getChokepoints( bool static_data ) const = 0;
	virtual const std::vector<BaseLocation> & getBaseLocations( bool static_data ) const = 0;

	virtual Region getRegion( WalkPosition pos, bool static_data ) const = 0;
	virtual int getClearance( WalkPosition pos, bool static_data ) const = 0;
	virtual int getConnectivity( WalkPosition pos, bool static_data ) const = 0;
	virtual WalkPosition getClosestObstacle( WalkPosition pos, bool static_data ) const = 0;

	virtual std::pair<Chokepoint, Chokepoint> getTravelChokepoints( Region start, Region end, bool static_data ) const = 0;
	virtual int getGroundDistance( WalkPosition start, WalkPosition end, bool static_data ) const = 0;
};