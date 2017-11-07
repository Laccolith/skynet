#include "SkynetBaseManager.h"

#include "UnitTracker.h"
#include "UnitManager.h"
#include "PlayerTracker.h"
#include "ResourceManager.h"
#include "TerrainAnalyser.h"
#include "Chokepoint.h"
#include "MapUtil.h"

#include <array>

constexpr int c_max_mineral_workers = 2;
constexpr int c_max_gas_workers = 3;

SkynetBaseManager::SkynetBaseManager( Core & core )
	: BaseManagerInterface( core )
	, MessageListener<BasesRecreated>( getBaseTracker() )
	, MessageListener<UnitDestroy>( getUnitTracker() )
{
	core.registerUpdateProcess( 3.0f, [this]() { preUpdate(); } );
	core.registerUpdateProcess( 5.0f, [this]() { postUpdate(); } );
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

void SkynetBaseManager::preUpdate()
{
	for( auto & worker_transfer : m_worker_transfers )
	{
		if( !worker_transfer.first.second->getResourceDepot() || worker_transfer.first.second->getResourceDepot()->getPlayer() != getPlayerTracker().getLocalPlayer() )
		{
			worker_transfer.second.clear();
			continue;
		}

		worker_transfer.second.erase( std::remove_if( worker_transfer.second.begin(), worker_transfer.second.end(), [this, &worker_transfer]( auto & transfer_task )
		{
			Unit worker = transfer_task->getAssignedUnit();
			if( !worker )
				return false;

			if( !worker->exists() )
				return true;

			Base current_base = getBaseTracker().getBase( worker->getTilePosition() );

			return current_base == worker_transfer.first.second;
		} ), worker_transfer.second.end() );
	}

	for( auto it = m_worker_transfers.begin(); it != m_worker_transfers.end(); )
	{
		if( it->second.empty() )
			it = m_worker_transfers.erase( it );
		else
			++it;
	}
}

void SkynetBaseManager::postUpdate()
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

	struct WorkerCount
	{
		int assigned_workers = 0;
		int required_workers = 0;
	};

	struct MultiBaseData
	{
		int time_till_operational = 0;

		std::array<WorkerCount, c_max_mineral_workers + 2> worker_counts;

		int required_change = 0;
	};

	std::map<Base, MultiBaseData> multi_base_data_map;

	for( auto & base_data : m_base_data )
	{
		bool is_active = base_data.first->getResourceDepot() && base_data.first->getResourceDepot()->getPlayer() == player;

		auto base_worker_it = current_base_workers.find( base_data.first );
		if( base_worker_it == current_base_workers.end() )
		{
			if( !is_active )
			{
				base_data.second.available_workers.clear();
				base_data.second.worker_to_resource.clear();
				base_data.second.resource_to_workers.clear();
				continue;
			}
			else
			{
				base_worker_it = current_base_workers.emplace( base_data.first, UnitGroup() ).first;
			}
		}

		const UnitGroup & current_workers = base_worker_it->second;

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
			if( !gas->getType().isRefinery() || !gas->isCompleted() || gas->getPlayer() != player || gas->getResources() == 0 )
				continue;

			refineries.insert( gas );
		}

		int max_gas = is_active ? std::min( refineries.size() * c_max_gas_workers, base_data.second.available_workers.size() ) : 0;
		int max_mining = is_active ? std::min( base_data.second.sorted_minerals.size() * c_max_mineral_workers, base_data.second.available_workers.size() - max_gas ) : 0;

		std::array<int, c_max_mineral_workers> mineral_assignments;
		for( int i = 0; i < c_max_mineral_workers; ++i )
		{
			mineral_assignments[i] = std::min( (int) base_data.second.sorted_minerals.size(), max_mining );
			max_mining -= mineral_assignments[i];
		}

		auto & muti_base_data = multi_base_data_map[base_data.first];
		muti_base_data.time_till_operational = base_data.first->getResourceDepot() ? base_data.first->getResourceDepot()->getTimeTillCompleted() : max_time;

		muti_base_data.worker_counts[0].assigned_workers = max_gas;
		muti_base_data.worker_counts[0].required_workers = (refineries.size() * c_max_gas_workers) - max_gas;

		muti_base_data.worker_counts[c_max_mineral_workers + 1].assigned_workers = base_data.second.available_workers.size() - max_gas;

		for( int i = 0; i < c_max_mineral_workers; ++i )
		{
			muti_base_data.worker_counts[i + 1].assigned_workers = mineral_assignments[i];
			muti_base_data.worker_counts[i + 1].required_workers = base_data.second.sorted_minerals.size() - mineral_assignments[i];

			muti_base_data.worker_counts[c_max_mineral_workers + 1].assigned_workers -= mineral_assignments[i];
		}

		for( auto & pair : base_data.second.resource_to_workers )
		{
			if( pair.first->getType().isMineralField() )
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
			else
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
		}

		bool given_move_order = false;

		int depot_time_till_complete = base_data.first->getResourceDepot()->getTimeTillCompleted();
		if( depot_time_till_complete > 40.0 )
		{
			Base nearest_base = nullptr;
			int nearest_distance = max_distance;

			for( Base base : getBaseTracker().getAllBases( player ) )
			{
				if( base == base_data.first )
					continue;

				int distance = getTerrainAnalyser().getGroundDistance( WalkPosition( base_data.first->getCenterPosition() ), WalkPosition( base->getCenterPosition() ) ) * 8;
				if( distance < nearest_distance )
				{
					nearest_distance = distance;
					nearest_base = base;
				}
			}

			if( nearest_base )
			{
				int travel_time = int( double( nearest_distance ) / player->getRace().getWorker().topSpeed() );
				if( ((travel_time*2.0) + 40.0) < depot_time_till_complete )
				{
					for( auto worker : base_data.second.available_workers )
					{
						worker->move( nearest_base->getCenterPosition() );
					}

					given_move_order = true;
				}
			}
		}
		
		if( !given_move_order )
		{
			for( auto worker : base_data.second.available_workers )
			{
				Unit & resource_assignment = base_data.second.worker_to_resource[worker];
				if( !resource_assignment )
				{
					// TODO: Room for improvement - Collect all workers without assignments first then choose the assignements for them all based on distance

					for( auto refinery : refineries )
					{
						auto & workers = base_data.second.resource_to_workers[refinery];
						if( workers.size() < c_max_gas_workers )
						{
							resource_assignment = refinery;
							workers.insert( worker, true );
							break;
						}
					}

					for( unsigned int i = 1; i <= c_max_mineral_workers && !resource_assignment; ++i )
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
						worker->returnCargo( base_data.first->getResourceDepot() );
					else
						worker->gather( resource_assignment );
				}
			}
		}

		// TODO: Figure out travel time and generate these values
		if( base_data.first->getResourceDepot()->isCompleted() )
		{
			for( auto & pair : base_data.second.resource_to_workers )
			{
				switch( pair.second.size() )
				{
				case 0:
					continue;

				case 1:
					if( pair.first->getType().isMineralField() )
						mineral_rate += 0.0501;
					else
						gas_rate += 0.0701;

					continue;

				case 2:
					if( pair.first->getType().isMineralField() )
						mineral_rate += 0.08745;
					else
						gas_rate += 0.13985;

					continue;

				default:
					if( pair.first->getType().isMineralField() )
						mineral_rate += 0.08745;
					else
						gas_rate += 0.2083;

					continue;
				}
			}
		}
	}

	getResourceManager().setMineralRate( mineral_rate );
	getResourceManager().setGasRate( gas_rate );

	for( auto & worker_transfer : m_worker_transfers )
	{
		Base source_base = worker_transfer.first.first;
		Base target_base = worker_transfer.first.second;

		int workers_transfering = 0;
		int awaiting_transfer = 0;
		for( auto & transfer_task : worker_transfer.second )
		{
			if( transfer_task->getAssignedUnit() )
			{
				transfer_task->getAssignedUnit()->move( target_base->getCenterPosition() );
				++workers_transfering;
			}
			else
				++awaiting_transfer;
		}

		if( awaiting_transfer > 0 || workers_transfering > 0 )
		{
			int transfering_target = awaiting_transfer + workers_transfering;
			auto & muti_base_data_target = multi_base_data_map[target_base];
			
			for( int i = 0; i < (int) muti_base_data_target.worker_counts.size(); ++i )
			{
				int workers_assigned = std::min( transfering_target, muti_base_data_target.worker_counts[i].required_workers );
				muti_base_data_target.worker_counts[i].required_workers -= workers_assigned;
				muti_base_data_target.worker_counts[i].assigned_workers += workers_assigned;
				transfering_target -= workers_assigned;
			}

			if( transfering_target > 0 )
			{
				muti_base_data_target.worker_counts[c_max_mineral_workers + 1].assigned_workers += transfering_target;
			}

			if( awaiting_transfer )
			{
				auto & muti_base_data_source = multi_base_data_map[source_base];

				for( int i = muti_base_data_source.worker_counts.size() - 1; i >= 0; --i )
				{
					int workers_assigned = std::min( awaiting_transfer, muti_base_data_source.worker_counts[i].assigned_workers );
					muti_base_data_source.worker_counts[i].required_workers += workers_assigned;
					muti_base_data_source.worker_counts[i].assigned_workers -= workers_assigned;
					awaiting_transfer -= workers_assigned;
				}
			}
		}
	}

	for(;;)
	{
		int most_saturated_i = -1;
		std::pair<const Base, MultiBaseData> * most_saturated = nullptr;

		int least_saturated_i = c_max_mineral_workers + 2;
		std::pair<const Base, MultiBaseData> * least_saturated = nullptr;

		for( auto & multi_base_data : multi_base_data_map )
		{
			for( int i = 0; i < (int)multi_base_data.second.worker_counts.size(); ++i)
			{
				if( (i > most_saturated_i && multi_base_data.second.worker_counts[i].assigned_workers > 0) ||
					(i == most_saturated_i && most_saturated->second.worker_counts[i].assigned_workers < multi_base_data.second.worker_counts[i].assigned_workers) )
				{
					most_saturated_i = i;
					most_saturated = &multi_base_data;
				}

				if( (i < least_saturated_i && multi_base_data.second.worker_counts[i].required_workers > 0) ||
					(i == least_saturated_i && least_saturated->second.worker_counts[i].required_workers < multi_base_data.second.worker_counts[i].required_workers) )
				{
					least_saturated_i = i;
					least_saturated = &multi_base_data;
				}
			}
		}

		if( !most_saturated || !least_saturated || most_saturated == least_saturated || most_saturated_i <= least_saturated_i )
			break;

		int workers_to_move = std::min( most_saturated->second.worker_counts[most_saturated_i].assigned_workers, least_saturated->second.worker_counts[least_saturated_i].required_workers );

		most_saturated->second.required_change -= workers_to_move;
		least_saturated->second.required_change += workers_to_move;

		most_saturated->second.worker_counts[most_saturated_i].assigned_workers -= workers_to_move;
		most_saturated->second.worker_counts[most_saturated_i].required_workers += workers_to_move;

		least_saturated->second.worker_counts[least_saturated_i].required_workers -= workers_to_move;
		least_saturated->second.worker_counts[least_saturated_i].assigned_workers += workers_to_move;
	}

	// TODO: Pair these up based on distance
	for( auto & multi_base_data : multi_base_data_map )
	{
		while( multi_base_data.second.required_change < 0 )
		{
			for( auto & multi_base_data_target : multi_base_data_map )
			{
				if( multi_base_data_target.second.required_change > 0 )
				{
					int total_to_move = std::min( -multi_base_data.second.required_change, multi_base_data_target.second.required_change );
					int remaining_to_move = total_to_move;

					auto reverse_transfers = m_worker_transfers.find(std::make_pair( multi_base_data_target.first, multi_base_data.first ));

					if( reverse_transfers != m_worker_transfers.end() && !reverse_transfers->second.empty() )
					{
						while( remaining_to_move > 0 && !reverse_transfers->second.empty() )
						{
							reverse_transfers->second.pop_back();
							--remaining_to_move;
						}
					}

					if( remaining_to_move > 0 )
					{
						auto start_chokepoint = getTerrainAnalyser().getTravelChokepoints( multi_base_data.first->getRegion(), multi_base_data_target.first->getRegion() ).first;

						if( start_chokepoint )
						{
							int ground_distance = getTerrainAnalyser().getGroundDistance( WalkPosition( multi_base_data.first->getCenterPosition() ), WalkPosition( multi_base_data_target.first->getCenterPosition() ) ) * 8;
							int travel_time = int( double( ground_distance ) / player->getRace().getWorker().topSpeed() ) + 40;

							if( travel_time > multi_base_data_target.first->getResourceDepot()->getTimeTillCompleted() )
							{
								auto & transfers = m_worker_transfers[std::make_pair( multi_base_data.first, multi_base_data_target.first )];

								for( int i = 0; i < remaining_to_move; ++i )
								{
									auto task = getTaskManager().createTask( "Worker Base Transfer" );

									// TODO: Required time needs to be how long it will take to travel
									task->addRequirementUnit( player->getRace().getWorker(), multi_base_data.first, 16, Position( start_chokepoint->getCenter() ), multi_base_data_target.first->getCenterPosition() );

									transfers.push_back( std::move( task ) );
								}
							}
						}
					}

					multi_base_data.second.required_change += total_to_move;
					multi_base_data_target.second.required_change -= total_to_move;
				}
			}
		}
	}

	volatile int i = 0;
}
