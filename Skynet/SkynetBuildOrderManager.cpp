#include "SkynetBuildOrderManager.h"

#include "LazyQuery.h"

SkynetBuildOrderManager::SkynetBuildOrderManager( Core & core )
	: BuildOrderManagerInterface( core )
{
	using namespace LazyQuery;

	auto & fourteen_nexus = createBuildOrder( "14 Nexus" );

	auto fourteen_nexus_test = enemyRace() == Races::Terran && numEnemies() == 1;
	bool can_fourteen_nexus = fourteen_nexus_test.evaluate( *this );

	auto & two_gate = createBuildOrder( "2 Gate" );

	auto terran_test = enemyRace() == Races::Terran;
	bool is_terran = terran_test.evaluate( *this );

	auto two_gate_test = enemyRace() == Races::Zerg || enemyRace() == Races::Protoss || enemyRace() == Races::Unknown;
	bool is_two_gate = two_gate_test.evaluate( *this );

	volatile int i = 0;
}

SkynetBuildOrder & SkynetBuildOrderManager::createBuildOrder( std::string_view name )
{
	m_build_orders.emplace_back( std::make_unique<SkynetBuildOrder>( name ) );
	return *m_build_orders.back().get();
}