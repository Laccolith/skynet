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
#include "SkynetBuildLocationManager.h"

SkynetCore::SkynetCore()
{
	BWAPI::Broodwar->enableFlag( BWAPI::Flag::UserInput );
	BWAPI::Broodwar->setLocalSpeed( 0 );

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
	m_build_location_manager = std::make_unique<SkynetBuildLocationManager>( *this );

	std::sort( m_update_processes.begin(), m_update_processes.end(), []( const std::pair<float, std::function<void()>> & lhs, const std::pair<float, std::function<void()>> & rhs )
	{
		return lhs.first < rhs.first;
	} );

	m_in_startup = false;
	BWAPI::Broodwar->printf( "Skynet is online." );

	auto worker_type = getPlayerTracker().getLocalPlayer()->getRace().getWorker();
	auto supply_type = getPlayerTracker().getLocalPlayer()->getRace().getSupplyProvider();

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( supply_type ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Nexus, BuildLocationType::Expansion ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Gateway ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Gateway ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	//m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Assimilator ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( supply_type ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( supply_type ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Gateway ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( supply_type ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( supply_type ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );

	for( int i = 0; i < 160; ++i )
		m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Gateway ) );

	for( int i = 0; i < 10; ++i )
		m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( supply_type ) );

	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Gateway ) );

	/*for( int i = 0; i < 16; ++i )
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( worker_type ) );

	for( int i = 0; i < 160; ++i )
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Zealot ) );

	for( int i = 0; i < 4; ++i )
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( UnitTypes::Protoss_Gateway ) );

	for( int i = 0; i < 25; ++i )
	m_tasks.emplace_back( getControlTaskFactory().createBuildControlTask( supply_type ) );*/
}

SkynetCore::~SkynetCore() = default;

void SkynetCore::update()
{
	/*static BWAPI::Unit resource = nullptr;
	static std::vector<std::pair<BWAPI::Unit, int>> bwapi_units;

	const int num_workers = 2;

	bool testing_gas = false;

	static double expected_gather_rate = 0.0;

	if( bwapi_units.empty() )
	{
		for( auto unit : BWAPI::Broodwar->getAllUnits() )
		{
			if( unit->getType().isWorker() )
			{
				bwapi_units.push_back( std::make_pair( unit, -1 ) );

				if( bwapi_units.size() == num_workers )
					break;
			}
		}

		BWAPI::Broodwar->setCommandOptimizationLevel( 0 );
		BWAPI::Broodwar->setLatCom( false );

		BWAPI::Unit bwapi_unit = BWAPI::Broodwar->getClosestUnit( Position( 0, 0 ), BWAPI::Filter::IsWorker );
		resource = BWAPI::Broodwar->getClosestUnit( bwapi_unit->getPosition(), testing_gas ? BWAPI::Filter::IsRefinery : BWAPI::Filter::IsMineralField );
		BWAPI::Unit depot = BWAPI::Broodwar->getClosestUnit( bwapi_unit->getPosition(), BWAPI::Filter::IsResourceDepot );

		double ground_distance = depot->getDistance( resource );

		double acceleration = bwapi_unit->getType().acceleration();
		double deceleration = bwapi_unit->getType().haltDistance();
		double top_speed = bwapi_unit->getType().topSpeed();

		double acceleration_time = top_speed / acceleration;
		double acceleration_distance = 0.5 * acceleration * (acceleration_time * acceleration_time);

		double deceleration_time = top_speed / deceleration;
		double deceleration_distance = 0.5 * deceleration * (deceleration_time * deceleration_time);

		double top_speed_distance = ground_distance - acceleration_distance - deceleration_distance;
		double top_speed_time = top_speed_distance / top_speed;

		double total_traveling_time = acceleration_time + deceleration_time + top_speed_time;

		double average_gathering_time = 81.0;

		double return_time = 20;

		double gather_rate = 8 / (return_time + average_gathering_time + (total_traveling_time * 2.0));

		expected_gather_rate = gather_rate;

		volatile int i = 0;

		//for( auto worker : bwapi_units )
		{
			//if( worker->isCarryingGas() || worker->isCarryingMinerals() )
			//	worker->returnCargo();
			//else
			//	worker->gather( mineral );
		}
	}

	static int gathering_time = 0;
	static int total_gathering_time = 0;
	static int num_gathers = 0;

	static int carry_time = 0;
	static int total_carry_time = 0;
	static int num_carries = 0;

	static int last_resources = 0;

	if( last_resources != (testing_gas ? BWAPI::Broodwar->self()->gas() : BWAPI::Broodwar->self()->minerals()) )
	{
		last_resources = (testing_gas ? BWAPI::Broodwar->self()->gas() : BWAPI::Broodwar->self()->minerals());
	}

	for( auto & pair : bwapi_units )
	{
		auto worker = pair.first;
		auto& last_order_execute_time = pair.second;

		if( worker->getOrder() == Orders::MiningMinerals || worker->getOrder() == Orders::HarvestGas )
			++gathering_time;
		else if( gathering_time != 0 )
		{
			total_gathering_time += gathering_time;
			++num_gathers;

			BWAPI::Broodwar->printf( "gather time = %d", gathering_time );
			gathering_time = 0;
		}

		if( worker->isCarryingGas() || worker->isCarryingMinerals() )
		{
			++carry_time;

			if( worker->getOrder() == Orders::ReturnGas || worker->getOrder() == Orders::ReturnMinerals )
				continue;

			if( worker->getLastCommand().getType() == UnitCommandTypes::Return_Cargo )
			{
				if( last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
					continue;
			}

			if( worker->returnCargo() )
				last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
		}
		else
		{
			if( carry_time != 0 )
			{
				total_carry_time += carry_time;
				++num_carries;

				BWAPI::Broodwar->printf( "carry time = %d", carry_time );
				carry_time = 0;
			}

			const auto order = worker->getOrder();
			if( order == Orders::MoveToMinerals || order == Orders::WaitForMinerals || order == Orders::MiningMinerals ||
				order == Orders::MoveToGas || order == Orders::WaitForGas || order == Orders::HarvestGas || order == Orders::Harvest1 )
			{
				if( worker->getOrderTarget() == resource )
					continue;
			}

			if( worker->getLastCommand().getType() == UnitCommandTypes::Gather && worker->getLastCommand().getTarget() == resource )
			{
				if( last_order_execute_time >= BWAPI::Broodwar->getFrameCount() )
					continue;
			}

			if( worker->gather( resource ) )
				last_order_execute_time = BWAPI::Broodwar->getFrameCount() + BWAPI::Broodwar->getRemainingLatencyFrames();
		}
	}

	double resource_rate = double( (testing_gas ? BWAPI::Broodwar->self()->gas() : (BWAPI::Broodwar->self()->minerals() - 50)) ) / double( BWAPI::Broodwar->getFrameCount() );

	BWAPI::Broodwar->drawTextScreen( 5, 30, "Resource Rate = %.4f", resource_rate );

	BWAPI::Broodwar->drawTextScreen( 5, 40, "Expected = %.4f", expected_gather_rate );

	double gathering_average = num_gathers > 1 ? (double) total_gathering_time / (double) num_gathers : 0.0;

	BWAPI::Broodwar->drawTextScreen( 5, 55, "Gathering Rate = %.4f", gathering_average );

	double carry_average = num_carries > 1 ? (double) total_carry_time / (double) num_carries : 0.0;

	BWAPI::Broodwar->drawTextScreen( 5, 65, "Carry Rate = %.4f", carry_average );*/

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

	if( BWAPI::Broodwar->getMouseState( BWAPI::MouseButton::M_MIDDLE ) )
	{
		Position mouse_position = BWAPI::Broodwar->getMousePosition() + BWAPI::Broodwar->getScreenPosition();
		for( Unit unit : getUnitTracker().getAllUnits( UnitTypes::Protoss_Zealot, getPlayerTracker().getLocalPlayer() ) )
		{
			unit->attack( mouse_position );
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