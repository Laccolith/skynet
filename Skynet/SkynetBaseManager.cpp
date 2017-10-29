#include "SkynetBaseManager.h"

#include "UnitTracker.h"
#include "UnitManager.h"
#include "PlayerTracker.h"
#include "ResourceManager.h"
#include "MapUtil.h"

SkynetBaseManager::SkynetBaseManager( Core & core )
	: BaseManagerInterface( core )
	, MessageListener<BasesRecreated>( getBaseTracker() )
	, MessageListener<UnitDestroy>( getUnitTracker() )
{
	core.registerUpdateProcess( 5.0f, [this]() { update(); } );
}

void SkynetBaseManager::notify( const BasesRecreated & message )
{
	UnitType base_type = getPlayerTracker().getLocalPlayer()->getRace().getResourceDepot();

	for( Base base : getBaseTracker().getAllBases() )
	{
		auto & base_data = m_base_data[base];
		
		base_data.sorted_minerals = base->getMinerals();
		base_data.sorted_minerals.sort( [base, base_type]( Unit first, Unit second )
		{
			int d1 = MapUtil::getDistance( first->getPosition(), first->getType(), base->getCenterPosition(), base_type );
			int d2 = MapUtil::getDistance( second->getPosition(), second->getType(), base->getCenterPosition(), base_type );
			
			return d1 < d2;
		} );
	}
}

void SkynetBaseManager::notify( const UnitDestroy & message )
{
	if( !message.unit->getType().isMineralField() )
		return;

	for( auto & base_data : m_base_data )
	{
		if( base_data.second.sorted_minerals.contains( message.unit ) )
		{
			base_data.second.sorted_minerals.remove( message.unit );

			auto it = base_data.second.mineral_to_workers.find( message.unit );
			if( it != base_data.second.mineral_to_workers.end() )
			{
				for( auto unit : it->second )
				{
					base_data.second.worker_to_mineral.erase( unit );
				}

				base_data.second.mineral_to_workers.erase( it );
			}

			break;
		}
	}
}

void SkynetBaseManager::update()
{
	if( isDebugging( Debug::Default ) )
	{
		for( auto & base_data : m_base_data )
		{
			int i = 0;
			for( auto mineral : base_data.second.sorted_minerals )
			{
				BWAPI::Broodwar->drawTextMap( mineral->getPosition(), "%d", i );
				++i;
			}
		}
	}

	int current_latency = BWAPI::Broodwar->getRemainingLatencyFrames();

	auto player = getPlayerTracker().getLocalPlayer();

	UnitGroup current_workers;
	for( auto worker : getUnitTracker().getAllUnits( player->getRace().getWorker(), player ) )
	{
		if( getUnitManager().getFreeTime( worker ) < current_latency )
			continue;

		auto base = getBaseTracker().getBase( worker->getTilePosition() );
		if( !base || base->getResourceDepot() )
			continue;

		m_base_data[base].available_workers.insert( worker );
		current_workers.insert( worker, true );
	}

	double mineral_rate = 0.0;

	for( auto & base_data : m_base_data )
	{
		base_data.second.available_workers.removeIf( [&current_workers, &base_data]( Unit worker )
		{
			if( !current_workers.contains( worker ) )
			{
				auto it = base_data.second.worker_to_mineral.find( worker );
				if( it != base_data.second.worker_to_mineral.end() )
				{
					base_data.second.mineral_to_workers[it->second].remove( worker );
					base_data.second.worker_to_mineral.erase( it );
				}

				return true;
			}

			return false;
		} );

		mineral_rate += double( std::min( base_data.second.available_workers.size(), base_data.second.sorted_minerals.size() * 3 ) ) * (8.0 / 180.0);

		for( auto worker : base_data.second.available_workers )
		{
			Unit & mineral_assignment = base_data.second.worker_to_mineral[worker];
			if( !mineral_assignment )
			{
				for( unsigned int i = 1; i <= 3 && !mineral_assignment; ++i )
				{
					for( Unit mineral : base_data.second.sorted_minerals )
					{
						auto & workers = base_data.second.mineral_to_workers[mineral];
						if( workers.size() < i )
						{
							mineral_assignment = mineral;
							workers.insert( worker, true );
							break;
						}
					}
				}
			}

			if( mineral_assignment )
			{
				if( worker->isCarryingGas() || worker->isCarryingMinerals() )
					worker->returnCargo();
				else
					worker->gather( mineral_assignment );
			}
		}
	}

	getResourceManager().setMineralRate( mineral_rate );
}
