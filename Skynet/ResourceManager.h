#pragma once

#include "CoreModule.h"

class ResourceManagerInterface : public CoreModule
{
public:
	ResourceManagerInterface( Core & core ) : CoreModule( core, "ResourceManager" ) {}

	virtual void reserveTaskMinerals( int time, int amount ) = 0;
	virtual void reserveTaskGas( int time, int amount ) = 0;
	virtual void reserveTaskSupply( int time, int amount ) = 0;

	virtual int earliestMineralAvailability( int amount ) const = 0;
	virtual int earliestGasAvailability( int amount ) const = 0;
	virtual int earliestSupplyAvailability( int amount ) const = 0;
};