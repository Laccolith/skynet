#include "SkynetBaseManager.h"

#include "UnitTracker.h"
#include "UnitManager.h"
#include "PlayerTracker.h"

SkynetBaseManager::SkynetBaseManager( Core & core )
	: BaseManagerInterface( core )
	, MessageListener<BasesRecreated>( getBaseTracker() )
{
	core.registerUpdateProcess( 5.0f, [this]() { update(); } );
}

void SkynetBaseManager::notify( const BasesRecreated & message )
{
}

void SkynetBaseManager::update()
{
	int current_latency = BWAPI::Broodwar->getRemainingLatencyFrames();

	auto player = getPlayerTracker().getLocalPlayer();

	int probe_index = 0;
	for( auto probe : getUnitTracker().getAllUnits( UnitTypes::Protoss_Probe, player ) )
	{
		if( getUnitManager().getFreeTime( probe ) < current_latency )
			continue;

		auto base = getBaseTracker().getBase( probe->getTilePosition() );
		if( !base )
			continue;

		auto minerals = base->getMinerals();
		size_t garther_index = probe_index % minerals.size();

		auto mineral = minerals[garther_index];

		if( probe->isCarryingGas() || probe->isCarryingMinerals() )
		{
			probe->returnCargo();
		}
		else
			probe->gather( mineral );

		++probe_index;
	}
}
