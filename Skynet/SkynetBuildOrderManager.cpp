#include "SkynetBuildOrderManager.h"

#include "TaskManager.h"
#include "LazyQuery.h"

#include <random>

SkynetBuildOrderManager::SkynetBuildOrderManager( Core & core )
	: BuildOrderManagerInterface( core )
{
	core.registerUpdateProcess( 3.0f, [this]() { update(); } );

	m_build_order_priority = getTaskManager().createPriorityGroup( "Build Order", 80.0 );

	setDebugging( Debug::Default, true );

	using namespace LazyQuery;
	using namespace UnitTypes;
	using namespace TechTypes;
	using namespace UpgradeTypes;

	auto & pvt_end_game = createBuildOrder( "PvT End Game" );
	{
		pvt_end_game.addArmyUnit( Protoss_Dragoon, 14.0 );
		pvt_end_game.addArmyUnit( Protoss_Zealot, 14.0 );
		pvt_end_game.addArmyUnit( Protoss_High_Templar, 3.0 );
		pvt_end_game.addArmyUnit( Protoss_Arbiter, 1.0 );
	}

	auto & pvt_mid_game = createBuildOrder( "PvT Mid Game" );
	{
		pvt_mid_game.addArmyUnit( Protoss_Dragoon, 10.0 );
		pvt_mid_game.addArmyUnit( Protoss_Zealot, 10.0 );

		pvt_mid_game.addItem( Protoss_Templar_Archives );
		auto last_item = pvt_mid_game.addItem( Psionic_Storm );

		pvt_mid_game.addArmyUnit( Protoss_High_Templar, 1.0, last_item.isInProgress() );

		pvt_mid_game.setAutoBuildTech( true, last_item.isInProgress() );

		pvt_mid_game.addBuild( pvt_end_game, last_item.isComplete() && counter() > (24 * 60) );
	}

	auto & citadel_first = createBuildOrder( "Citadel First" );
	{
		citadel_first.addArmyUnit( Protoss_Zealot, 1.0 );

		// Auto Build expansion

		citadel_first.addItem( Protoss_Citadel_of_Adun );
		auto last_item = citadel_first.addItem( Leg_Enhancements );

		citadel_first.addArmyUnit( Protoss_Dragoon, 1.0, last_item.isInProgress() );

		citadel_first.addBuild( pvt_mid_game, last_item.isComplete() && counter() > (24 * 60) );
	}

	auto & expand = createBuildOrder( "Expand" );
	{
		expand.addArmyUnit( Protoss_Dragoon, 1.0 );

		auto last_item = expand.addItem( Protoss_Nexus, BuildLocationType::Expansion );

		expand.addBuild( citadel_first, last_item.isInProgress() && counter() > (24 * 60 * 2) );
	}

	auto & additional_gateways = createBuildOrder( "Additional Gateways" );
	{
		additional_gateways.addArmyUnit( Protoss_Dragoon, 1.0 );

		auto last_item = additional_gateways.addItem( Protoss_Dragoon );
		additional_gateways.setAutoBuildArmy( true, last_item.isInProgress() );

		additional_gateways.addItem( Protoss_Gateway );
		last_item = additional_gateways.addItem( Singularity_Charge );

		additional_gateways.setAutoBuildProduction( true, last_item.isInProgress() );

		additional_gateways.addBuild( expand, last_item.isInProgress() && counter() > (24 * 60 * 4) ); // TODO: or enemy has researched siege tech
	}

	auto & one_gate_core = createBuildOrder( "1 Gate Core" );
	{
		one_gate_core.setStartingCondition( enemyRace() == Races::Terran );

		one_gate_core.addItem( Protoss_Probe, 4 );
		one_gate_core.addItem( Protoss_Pylon );
		// Scout

		auto build_workers_item = one_gate_core.addItem( Protoss_Probe );
		one_gate_core.setAutoBuildWorkers( true, build_workers_item.isInProgress() );

		one_gate_core.addItem( Protoss_Gateway );
		one_gate_core.addItem( Protoss_Assimilator );
		one_gate_core.addItem( Protoss_Cybernetics_Core );
		auto last_item = one_gate_core.addItem( Protoss_Pylon );
		one_gate_core.setAutoBuildSupply( true, last_item.isInProgress() );

		one_gate_core.addBuild( additional_gateways, last_item.isInProgress() );
	}

	auto & fourteen_nexus = createBuildOrder( "14 Nexus" );
	{
		fourteen_nexus.setStartingCondition( enemyRace() == Races::Terran && numEnemies() == 1 && numStartPositions() >= 4 );

		fourteen_nexus.addItem( Protoss_Probe, 4 );
		fourteen_nexus.addItem( Protoss_Pylon );
		// Scout

		fourteen_nexus.addItem( Protoss_Probe, 5 );
		fourteen_nexus.addItem( Protoss_Nexus, BuildLocationType::Expansion );
		fourteen_nexus.addItem( Protoss_Probe );
		fourteen_nexus.addItem( Protoss_Gateway );
		fourteen_nexus.addItem( Protoss_Probe );
		fourteen_nexus.addItem( Protoss_Assimilator );
		fourteen_nexus.addItem( Protoss_Probe );
		fourteen_nexus.addItem( Protoss_Cybernetics_Core );
		fourteen_nexus.addItem( Protoss_Gateway );
		fourteen_nexus.addItem( Protoss_Zealot );
		fourteen_nexus.addItem( Protoss_Probe, 2 );
		fourteen_nexus.addItem( Protoss_Pylon );
		fourteen_nexus.addItem( Protoss_Dragoon, 2 );
		fourteen_nexus.addItem( Singularity_Charge );
		fourteen_nexus.addItem( Protoss_Probe, 2 );
		fourteen_nexus.addItem( Protoss_Pylon );
		fourteen_nexus.addItem( Protoss_Dragoon, 2 );
		fourteen_nexus.addItem( Protoss_Probe, 2 );
		fourteen_nexus.addItem( Protoss_Pylon );
		auto last_item = fourteen_nexus.addItem( Protoss_Dragoon, 2 );

		fourteen_nexus.addArmyUnit( Protoss_Dragoon, 1.0 );

		fourteen_nexus.setAutoBuildWorkers( true, last_item.isInProgress() );
		fourteen_nexus.setAutoBuildSupply( true, last_item.isInProgress() );
		fourteen_nexus.setAutoBuildArmy( true, last_item.isInProgress() );
		fourteen_nexus.setAutoBuildProduction( true, last_item.isInProgress() );

		fourteen_nexus.addBuild( citadel_first, last_item.isComplete() && counter() > (24 * 60 * 2) );
	}
}

SkynetBuildOrder & SkynetBuildOrderManager::createBuildOrder( std::string_view name )
{
	m_build_orders.emplace_back( std::make_unique<SkynetBuildOrder>( *this, name ) );
	return *m_build_orders.back().get();
}

void SkynetBuildOrderManager::update()
{
	if( !m_current_build_order )
	{
		std::vector<SkynetBuildOrder *> starting_options;
		for( auto & build_order : m_build_orders )
		{
			if( build_order->canBeStartingBuild() )
			{
				starting_options.push_back( build_order.get() );
			}
		}

		if( starting_options.size() == 1 )
		{
			changeBuild( *starting_options[0] );
		}
		else if( !starting_options.empty() )
		{
			std::random_device rd;
			std::mt19937 gen( rd() );
			std::uniform_int_distribution<> dis( 0, starting_options.size() - 1 );

			changeBuild( *starting_options[dis( gen )] );
		}
	}

	SkynetBuildOrder * starting_build;
	do
	{
		starting_build = m_current_build_order;

		int build_id = 0;
		for( auto & build_item : m_current_build_items )
		{
			if( build_item.first && build_item.second.evaluate( *this ) )
			{
				m_current_build_item_tasks[build_id] = build_item.first();
				build_item.first = nullptr;
			}

			++build_id;
		}

		for( auto & generic_item : m_current_generic_items )
		{
			if( generic_item.first && generic_item.second.evaluate( *this ) )
			{
				generic_item.first();
				if( m_current_build_order != starting_build )
					break;

				generic_item.first = nullptr;
			}
		}
	}
	while( m_current_build_order != starting_build );
}

int SkynetBuildOrderManager::timeTillItemStarts( int id )
{
	return m_current_build_item_tasks[id] ? m_current_build_item_tasks[id]->timeTillStart() : max_time;
}

bool SkynetBuildOrderManager::isItemInProgress( int id )
{
	return m_current_build_item_tasks[id] && m_current_build_item_tasks[id]->isInProgress();
}

bool SkynetBuildOrderManager::isItemComplete( int id )
{
	return m_current_build_item_tasks[id] && m_current_build_item_tasks[id]->isComplete();
}

void SkynetBuildOrderManager::changeBuild( SkynetBuildOrder & next_build )
{
	if( isDebugging( Debug::Default ) )
		BWAPI::Broodwar->printf( m_current_build_order ? "Build transitioning to %s." : "Build starting with %s.", next_build.getName().c_str() );

	m_current_build_order = &next_build;

	m_current_build_items.clear();
	m_current_build_item_tasks.clear(); // TODO: Need to keep these if they are still active so we don't stop something in progress
	m_current_generic_items.clear();

	m_current_build_items = next_build.getBuildItems();
	m_current_generic_items = next_build.getGenericItems();

	m_current_build_item_tasks.resize( m_current_build_items.size() );

	postMessage<BuildOrderChanged>();
}
