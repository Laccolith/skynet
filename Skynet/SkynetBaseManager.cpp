#include "SkynetBaseManager.h"

#include "UnitTracker.h"
#include "UnitManager.h"
#include "PlayerTracker.h"
#include "ResourceManager.h"
#include "MapUtil.h"

#include <array>

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
	if( message.unit->getType().isMineralField() )
	{
		for( auto & base_data : m_base_data )
		{
			if( base_data.second.sorted_minerals.contains( message.unit ) )
			{
				base_data.second.sorted_minerals.remove( message.unit );

				auto it = base_data.second.resource_to_workers.find( message.unit );
				if( it != base_data.second.resource_to_workers.end() )
				{
					for( auto unit : it->second )
					{
						base_data.second.worker_to_resource.erase( unit );
					}

					base_data.second.resource_to_workers.erase( it );
				}

				break;
			}
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

	std::map<Base, UnitGroup> current_base_workers;
	for( auto worker : getUnitTracker().getAllUnits( player->getRace().getWorker(), player ) )
	{
		if( getUnitManager().getFreeTime( worker ) < current_latency )
			continue;

		auto base = getBaseTracker().getBase( worker->getTilePosition() );
		if( !base || !base->getResourceDepot() )
			continue;

		m_base_data[base].available_workers.insert( worker );
		current_base_workers[base].insert( worker, true );
	}

	double mineral_rate = 0.0;
	double gas_rate = 0.0;

	for( auto & base_data : m_base_data )
	{
		const UnitGroup & current_workers = current_base_workers[base_data.first];

		base_data.second.available_workers.removeIf( [&current_workers, &base_data]( Unit worker )
		{
			if( !current_workers.contains( worker ) )
			{
				auto it = base_data.second.worker_to_resource.find( worker );
				if( it != base_data.second.worker_to_resource.end() )
				{
					if( it->second )
						base_data.second.resource_to_workers[it->second].remove( worker );
					base_data.second.worker_to_resource.erase( it );
				}

				return true;
			}

			return false;
		} );

		UnitGroup refineries;
		for( auto gas : base_data.first->getGeysers() )
		{
			if( !gas->getType().isRefinery() || !gas->isCompleted() || gas->getPlayer() != getPlayerTracker().getLocalPlayer() || gas->getResources() == 0 )
				continue;

			refineries.insert( gas );
		}

		int num_gas = std::min( refineries.size() * 3, base_data.second.available_workers.size() );
		int num_mining = std::min( base_data.second.sorted_minerals.size() * 3, base_data.second.available_workers.size() - num_gas );

		std::array<int, 3> mineral_assignments = { 0, 0, 0 };
		mineral_assignments[0] = std::min( (int) base_data.second.sorted_minerals.size(), num_mining );
		mineral_assignments[1] = std::min( (int) base_data.second.sorted_minerals.size(), num_mining - mineral_assignments[0] );
		mineral_assignments[2] = std::min( (int) base_data.second.sorted_minerals.size(), num_mining - mineral_assignments[0] - mineral_assignments[1] );

		for( auto & pair : base_data.second.resource_to_workers )
		{
			if( pair.first->getType().isRefinery() )
			{
				if( !refineries.contains( pair.first ) )
				{
					for( auto unit : pair.second )
					{
						base_data.second.worker_to_resource.erase( unit );
					}

					pair.second.clear();
				}
			}
			else if( pair.first->getType().isMineralField() )
			{
				int num_to_remove = 0;
				for( size_t i = 0; i < pair.second.size(); ++i )
				{
					if( mineral_assignments[i] == 0 )
						++num_to_remove;
					else
						--mineral_assignments[i];
				}

				for( int i = 0; i < num_to_remove; ++i )
				{
					base_data.second.worker_to_resource.erase( pair.second.back() );
					pair.second.pop_back();
				}
			}
		}

		for( auto worker : base_data.second.available_workers )
		{
			Unit & resource_assignment = base_data.second.worker_to_resource[worker];
			if( !resource_assignment )
			{
				// TODO: Room for improvement - Collect all workers without assignments first then choose the assignements for them all based on distance

				for( auto refinery : refineries )
				{
					auto & workers = base_data.second.resource_to_workers[refinery];
					if( workers.size() < 3 )
					{
						resource_assignment = refinery;
						workers.insert( worker, true );
						break;
					}
				}

				for( unsigned int i = 1; i <= 3 && !resource_assignment; ++i )
				{
					for( Unit mineral : base_data.second.sorted_minerals )
					{
						auto & workers = base_data.second.resource_to_workers[mineral];
						if( workers.size() < i )
						{
							resource_assignment = mineral;
							workers.insert( worker, true );
							break;
						}
					}
				}
			}

			if( resource_assignment )
			{
				if( worker->isCarryingGas() || worker->isCarryingMinerals() )
					worker->returnCargo();
				else
					worker->gather( resource_assignment );
			}
		}

		mineral_rate += double( num_mining ) * (8.0 / 180.0);
		gas_rate += double( num_gas ) * (8.0 / 180.0);
	}

	getResourceManager().setMineralRate( mineral_rate );
	getResourceManager().setGasRate( gas_rate );
}
