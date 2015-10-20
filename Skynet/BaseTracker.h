#pragma once

#include "SkynetInterface.h"

class BaseTrackerInterface : public SkynetInterface
{
public:
	BaseTrackerInterface( Access &access ) : SkynetInterface( access, "BaseTracker" ) {}

	virtual const std::vector<Base> &getAllBases() const = 0;

	virtual Base getBase( TilePosition possition ) const = 0;
};