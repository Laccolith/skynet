#pragma once

#include "SkynetInterface.h"

#include "VectorUnique.h"
#include "Messager.h"

struct TerrainAnalysePreFinalise
{
};

struct TerrainAnalyseComplete
{
};

class TerrainAnalyserInterface : public SkynetInterface, public MessageReporter<TerrainAnalysePreFinalise, TerrainAnalyseComplete>
{
public:
	TerrainAnalyserInterface( Access & access ) : SkynetInterface( access, "TerrainAnalyser" ) {}

	virtual const std::vector<Region> &getRegions() const = 0;
	virtual const std::vector<Chokepoint> &getChokepoints() const = 0;

	virtual Region getRegion( WalkPosition pos ) const = 0;
	virtual int getClearance( WalkPosition pos ) const = 0;
	virtual int getConnectivity( WalkPosition pos ) const = 0;
};