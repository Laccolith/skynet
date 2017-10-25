#include "SkynetCore.h"

#include "CoreModule.h"
#include "StringUtils.h"

#include "DrawBuffer.h"
#include "SkynetPlayerTracker.h"
#include "SkynetUnitTracker.h"
#include "SkynetUnitManager.h"
#include "SkynetTerrainAnalyser.h"
#include "SkynetBaseTracker.h"
#include "SkynetBaseManager.h"
#include "SkynetResourceManager.h"
#include "SkynetTaskManager.h"
#include "SkynetControlTaskFactory.h"

SkynetCore::SkynetCore()
{
	BWAPI::Broodwar->enableFlag( BWAPI::Flag::UserInput );

	m_draw_buffer = std::make_unique<DrawBuffer>( *this );
	m_player_tracker = std::make_unique<SkynetPlayerTracker>( *this );
	m_unit_tracker = std::make_unique<SkynetUnitTracker>( *this );
	m_unit_manager = std::make_unique<SkynetUnitManager>( *this );
	m_terrain_analyser = std::make_unique<SkynetTerrainAnalyser>( *this );
	m_base_tracker = std::make_unique<SkynetBaseTracker>( *this );
	m_base_manager = std::make_unique<SkynetBaseManager>( *this );
	m_resource_tracker = std::make_unique<SkynetResourceManager>( *this );
	m_task_manager = std::make_unique<SkynetTaskManager>( *this );
	m_control_task_factory = std::make_unique<SkynetControlTaskFactory>( *this );

	std::sort( m_update_processes.begin(), m_update_processes.end(), []( const std::pair<float, std::function<void()>> & lhs, const std::pair<float, std::function<void()>> & rhs )
	{
		return lhs.first < rhs.first;
	} );

	m_in_startup = false;
	BWAPI::Broodwar->printf( "Skynet is online." );
}

SkynetCore::~SkynetCore() = default;

void SkynetCore::update()
{
	static auto train_task1 = getControlTaskFactory().createTrainControlTask( UnitTypes::Protoss_Probe );
	static auto train_task2 = getControlTaskFactory().createTrainControlTask( UnitTypes::Protoss_Probe );
	static auto train_task3 = getControlTaskFactory().createTrainControlTask( UnitTypes::Protoss_Probe );
	static auto train_task4 = getControlTaskFactory().createTrainControlTask( UnitTypes::Protoss_Probe );
	static auto train_task5 = getControlTaskFactory().createTrainControlTask( UnitTypes::Protoss_Probe );
	static auto train_task6 = getControlTaskFactory().createTrainControlTask( UnitTypes::Protoss_Probe );
	static auto train_task7 = getControlTaskFactory().createTrainControlTask( UnitTypes::Protoss_Probe );
	static auto train_task8 = getControlTaskFactory().createTrainControlTask( UnitTypes::Protoss_Probe );
	static auto train_task9 = getControlTaskFactory().createTrainControlTask( UnitTypes::Protoss_Probe );

	for( auto & e : BWAPI::Broodwar->getEvents() )
	{
		if( e.getType() == BWAPI::EventType::SendText )
		{
			auto tokens = StringUtils::split( e.getText(), "\t " );

			if( tokens.empty() )
				continue;

			if( tokens[0] == "debug" )
			{
				if( tokens.size() < 3 )
				{
					BWAPI::Broodwar->printf( "Not enough commands for debug, expects interface and command." );
					continue;
				}

				auto it = m_interfaces.find( tokens[1] );
				if( it != m_interfaces.end() )
				{
					if( !it->second->debugCommand( tokens[2] ) )
					{
						BWAPI::Broodwar->printf( "%s is not a valid debug command.", tokens[2].c_str() );
					}
				}
				else
					BWAPI::Broodwar->printf( "%s is not a known interface.", tokens[1].c_str() );
			}
			else
				BWAPI::Broodwar->printf( "%s is not a command.", tokens[0].c_str() );
		}
	}

	for( auto & update : m_update_processes )
		update.second();
}

void SkynetCore::registerModule( CoreModule & inter )
{
	if( m_in_startup )
		m_interfaces[inter.getName()] = &inter;
	else
		BWAPI::Broodwar->printf( "Module %s has attempted to register itself after startup.", inter.getName().c_str() );
}

void SkynetCore::registerUpdateProcess( float priority, std::function<void()> update_function )
{
	if( m_in_startup )
		m_update_processes.emplace_back( priority, std::move( update_function ) );
	else
		BWAPI::Broodwar->printf( "An update process has been attempted to register after startup." );
}