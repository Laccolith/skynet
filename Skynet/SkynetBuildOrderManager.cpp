#include "SkynetBuildOrderManager.h"

#include "LazyQuery.h"

SkynetBuildOrderManager::SkynetBuildOrderManager( Core & core )
	: BuildOrderManagerInterface( core )
{
	core.registerUpdateProcess( 3.0f, [this]() { update(); } );

	using namespace LazyQuery;
	using namespace UnitTypes;
	using namespace TechTypes;
	using namespace UpgradeTypes;

	auto & citadel_first = createBuildOrder( "Citadel First" );

	auto & fourteen_nexus = createBuildOrder( "14 Nexus" );
	
	fourteen_nexus.setStartingCondition( enemyRace() == Races::Terran && numEnemies() == 1 && numStartPositions() >= 4 );
	
	fourteen_nexus.addItem( Protoss_Probe, 4 );
	fourteen_nexus.addItem( Protoss_Pylon );
	// Scout when started
	fourteen_nexus.addItem( Protoss_Probe, 5 );
	fourteen_nexus.addItem( Protoss_Nexus );
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
	SkynetBuildOrderItem last_item = fourteen_nexus.addItem( Protoss_Dragoon, 2 );

	fourteen_nexus.addBuild( citadel_first, last_item.isComplete() && counter() > (24 * 60 * 2) );

	volatile int i = 0;
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

		if( !starting_options.empty() )
		{
			int randon_choice = 0;
			changeBuild( *starting_options[randon_choice] );
		}
	}

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
			generic_item.first = nullptr;
		}
	}
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
	m_current_build_order = &next_build;

	m_current_build_items.clear();
	m_current_build_item_tasks.clear();
	m_current_generic_items.clear();

	m_current_build_items = next_build.getBuildItems();
	m_current_generic_items = next_build.getGenericItems();

	m_current_build_item_tasks.resize( m_current_build_items.size() );
}
