#include "SkynetProductionManger.h"

#include "UnitTracker.h"
#include "PlayerTracker.h"
#include "ControlTaskFactory.h"

SkynetProductionManager::SkynetProductionManager( Core & core )
	: ProductionManagerInterface( core )
	, MessageListener<BuildOrderChanged>( getBuildOrderManager() )
{
	core.registerUpdateProcess( 5.0f, [this]() { update(); } );

	m_build_army_priority = getTaskManager().createPriorityGroup( "Auto Build Army", 75.0 );
	m_build_production_priority = getTaskManager().createPriorityGroup( "Auto Build Production", 70.0 );
}

void SkynetProductionManager::notify( const BuildOrderChanged & message )
{
	m_army_units.clear();
}

void SkynetProductionManager::update()
{
	update_production_units();
	update_army_units();
}

void SkynetProductionManager::update_army_units()
{
	if( !m_can_build_army )
		return;

	for ( auto & production_queue : m_unit_type_to_planned_army )
	{
		production_queue.second.erase( std::remove_if( production_queue.second.begin(), production_queue.second.end(), []( auto & production_item )
		{
			return production_item->isComplete();
		} ), production_queue.second.end() );
	}

	for( auto & production_queue : m_production_type_to_planned_army )
	{
		production_queue.second.erase( std::remove_if( production_queue.second.begin(), production_queue.second.end(), []( auto & production_item )
		{
			return production_item->isComplete();
		} ), production_queue.second.end() );
	}

	double total_weight = 0.0;
	double total_unit_count = 0.0;
	struct UnitToBuild
	{
		UnitType unit_type;
		double weight;
		double num_units;
		std::vector<ControlTask*> * planned_units;
		bool skip = false;
	};
	std::vector<UnitToBuild> units_to_build;
	for( auto & army_unit : m_army_units )
	{
		if( !army_unit.unit_condition.evaluate( *this ) )
			continue;

		auto & planned_units = m_unit_type_to_planned_army[army_unit.unit_type];

		units_to_build.emplace_back( UnitToBuild{ army_unit.unit_type, army_unit.weight, (double)getUnitTracker().getAllUnits( army_unit.unit_type, getPlayerTracker().getLocalPlayer() ).size() + planned_units.size(), &planned_units } );
		total_weight += army_unit.weight;
		total_unit_count += units_to_build.back().num_units;
	}

	if( units_to_build.empty() || total_weight <= 0.0 )
		return;

	for(;;)
	{
		UnitToBuild * current_lowest_weight = nullptr;
		double current_weight_difference = 0.0;
		for( auto & unit_to_build : units_to_build )
		{
			if( unit_to_build.skip )
				continue;

			double required_weight = unit_to_build.weight / total_weight;
			double current_weight = total_unit_count <= 0.0 ? 0.0 : unit_to_build.num_units / total_unit_count;

			double weight_difference = required_weight - current_weight;

			if( !current_lowest_weight || weight_difference > current_weight_difference )
			{
				current_lowest_weight = &unit_to_build;
				current_weight_difference = weight_difference;
			}
		}

		if( !current_lowest_weight )
			break;

		auto & production_queue = m_production_type_to_planned_army[current_lowest_weight->unit_type.whatBuilds().first];

		auto available_production_buildings = getUnitTracker().getAllUnits( current_lowest_weight->unit_type.whatBuilds().first, getPlayerTracker().getLocalPlayer() ).size() + m_production_type_to_planned[current_lowest_weight->unit_type.whatBuilds().first].size();

		if( production_queue.size() >= (available_production_buildings * 2) )
		{
			current_lowest_weight->skip = true;
			continue;
		}

		++current_lowest_weight->num_units;
		++total_unit_count;

		production_queue.push_back( getControlTaskFactory().createBuildControlTask( m_build_army_priority, current_lowest_weight->unit_type ) );

		current_lowest_weight->planned_units->push_back( production_queue.back().get() );
	}
}

void SkynetProductionManager::update_production_units()
{
	if( !m_can_build_production )
		return;

	if( m_last_production_task && m_last_production_task->isComplete() )
		m_last_production_task = nullptr;

	for( auto & production_queue : m_production_type_to_planned )
	{
		production_queue.second.erase( std::remove_if( production_queue.second.begin(), production_queue.second.end(), []( auto & production_item )
		{
			return production_item->isComplete();
		} ), production_queue.second.end() );
	}

	if( m_last_production_task && m_last_production_task->timeTillStart() > 100 )
		return;

	struct ProductionToBuild
	{
		double production_weight = 0.0;
		bool has_counted = false;
		double num_units = 0.0;
	};
	std::map<UnitType, ProductionToBuild> production_to_build;

	double total_weight = 0.0;
	double total_unit_count = 0.0;
	for( auto & army_unit : m_army_units )
	{
		if( !army_unit.production_condition.evaluate( *this ) )
			continue;

		auto & production = production_to_build[army_unit.unit_type.whatBuilds().first];
		if( !production.has_counted )
		{
			production.num_units = (double)getUnitTracker().getAllUnits( army_unit.unit_type.whatBuilds().first, getPlayerTracker().getLocalPlayer() ).size() + m_production_type_to_planned[army_unit.unit_type.whatBuilds().first].size();
			total_unit_count += production.num_units;
			production.has_counted = true;
		}

		double production_weight = army_unit.weight * army_unit.unit_type.buildTime();

		production.production_weight += production_weight;

		total_weight += production_weight;
	}

	decltype(production_to_build)::value_type * current_lowest_weight = nullptr;
	double current_weight_difference = 0.0;
	for( auto & production : production_to_build )
	{
		double required_weight = production.second.production_weight / total_weight;
		double current_weight = total_unit_count <= 0.0 ? 0.0 : production.second.num_units / total_unit_count;

		double weight_difference = required_weight - current_weight;

		if( !current_lowest_weight || weight_difference > current_weight_difference )
		{
			current_lowest_weight = &production;
			current_weight_difference = weight_difference;
		}
	}

	if( current_lowest_weight )
	{
		int idle_count = 0;
		for( auto unit : getUnitTracker().getAllUnits( current_lowest_weight->first, getPlayerTracker().getLocalPlayer() ) )
		{
			if( unit->isCompleted() && !unit->isTraining() )
				++idle_count;
		}

		if( idle_count > 0 )
			return;

		auto & planned_tasks = m_production_type_to_planned[current_lowest_weight->first];

		planned_tasks.push_back( getControlTaskFactory().createBuildControlTask( m_build_production_priority, current_lowest_weight->first ) );

		m_last_production_task = planned_tasks.back().get();
	}
}

void SkynetProductionManager::addArmyUnit( UnitType unit_type, double weight, Condition unit_condition, Condition production_condition )
{
	m_army_units.emplace_back( ArmyUnit{ unit_type, weight, unit_condition, production_condition } );
}