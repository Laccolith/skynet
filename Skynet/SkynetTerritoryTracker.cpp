#include "SkynetTerritoryTracker.h"

#include "BaseTracker.h"

SkynetTerritoryTracker::SkynetTerritoryTracker( Core & core )
	: TerritoryTrackerInterface( core )
{
	core.registerUpdateProcess( 4.0f, [this]() { update(); } );
}

void SkynetTerritoryTracker::update()
{
	for( Base base : getBaseTracker().getAllBases() )
	{
		if( !base->getPlayer() )
			continue;
	}
}
