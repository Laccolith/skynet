#include "SkynetCore.h"

#include "CoreModule.h"
#include "StringUtils.h"

#include "DrawBuffer.h"
#include "SkynetPlayerTracker.h"
#include "SkynetUnitTracker.h"
#include "SkynetUnitManager.h"
#include "SkynetTerrainAnalyser.h"
#include "SkynetBaseTracker.h"
#include "SkynetResourceManager.h"
#include "SkynetTaskManager.h"

SkynetCore::SkynetCore()
{
	BWAPI::Broodwar->enableFlag( BWAPI::Flag::UserInput );

	m_draw_buffer = std::make_unique<DrawBuffer>( *this );
	m_player_tracker = std::make_unique<SkynetPlayerTracker>( *this );
	m_unit_tracker = std::make_unique<SkynetUnitTracker>( *this );
	m_unit_manager = std::make_unique<SkynetUnitManager>( *this );
	m_terrain_analyser = std::make_unique<SkynetTerrainAnalyser>( *this );
	m_base_tracker = std::make_unique<SkynetBaseTracker>( *this );
	m_resource_tracker = std::make_unique<SkynetResourceManager>( *this );
	m_task_manager = std::make_unique<SkynetTaskManager>( *this );

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
	static std::unique_ptr<TaskInterface> train_task;
	if( !train_task )
	{
		train_task = getTaskManager().createTask();
		train_task->addRequirementMineral( 50 );
		train_task->addRequirementUnit( UnitTypes::Protoss_Nexus, UnitTypes::Protoss_Probe.buildTime() );
	}

	static std::unique_ptr<TaskInterface> second_task;
	if( !second_task )
	{
		second_task = getTaskManager().createTask();
		second_task->addRequirementUnit( UnitTypes::Protoss_Nexus, UnitTypes::Protoss_Probe.buildTime() );
	}

	static std::unique_ptr<TaskInterface> third_task;
	if( !third_task )
	{
		third_task = getTaskManager().createTask();
		third_task->addRequirementUnit( UnitTypes::Protoss_Nexus, UnitTypes::Protoss_Probe.buildTime() );
	}

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

	if( train_task->requirementsFulfilled() )
	{
		if( !train_task->getAssignedUnit()->isTraining() )
		{
			train_task->getAssignedUnit()->train( UnitTypes::Protoss_Probe );
		}
	}
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