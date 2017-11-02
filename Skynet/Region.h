#pragma once

#include "Types.h"

#include "VectorUnique.h"

class RegionInterface
{
public:
	virtual int getID() const = 0;

	virtual WalkPosition getCenter() const = 0;
	virtual int getClearance() const = 0;
	virtual int getConnectivity() const = 0;
	virtual int getSize() const = 0;

	virtual const VectorUnique<Chokepoint> &getChokepoints() const = 0;
	virtual const VectorUnique<TilePosition> &getChokepointTiles() const = 0;
	virtual const VectorUnique<BaseLocation> &getBaseLocations() const = 0;

	bool isConnected( Region other ) const { return getConnectivity() == other->getConnectivity(); }

	virtual bool isValid() const = 0;

	virtual void draw( Color color ) const = 0;
};