#include "SkynetProductionManger.h"

#include "UnitTracker.h"
#include "PlayerTracker.h"
#include "ControlTaskFactory.h"

SkynetProductionManager::SkynetProductionManager( Core & core )
	: ProductionManagerInterface( core )
	, MessageListener<BuildOrderChanged>( getBuildOrderManager() )
{
	core.registerUpdateProcess( 5.0f, [this]() { update(); } );

	m_build_army_priority = getTaskManager().createPriorityGroup( "Auto Build Army", 85.0 );
}

void SkynetProductionManager::notify( const BuildOrderChanged & message )
{
	m_army_units.clear();
}

void SkynetProductionManager::update()
{
	if( !m_can_build_army )
		return;

	for( auto & production_queue : m_planned_army_units )
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
		bool skip = false;
	};
	std::vector<UnitToBuild> units_to_build;
	for( auto & army_unit : m_army_units )
	{
		if( !army_unit.unit_condition.evaluate( *this ) )
			continue;

		units_to_build.emplace_back( UnitToBuild{ army_unit.unit_type, army_unit.weight, (double)getUnitTracker().getAllUnits( army_unit.unit_type, getPlayerTracker().getLocalPlayer() ).size() } ); // TODO: Add queued up items too
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

		auto & production_queue = m_planned_army_units[current_lowest_weight->unit_type.whatBuilds().first];

		auto available_production_buildings = getUnitTracker().getAllUnits( current_lowest_weight->unit_type.whatBuilds().first, getPlayerTracker().getLocalPlayer() ).size();

		if( production_queue.size() >= (available_production_buildings * 2) )
		{
			current_lowest_weight->skip = true;
			continue;
		}

		++current_lowest_weight->num_units;
		++total_unit_count;

		production_queue.push_back( getControlTaskFactory().createBuildControlTask( m_build_army_priority, current_lowest_weight->unit_type ) );
	}
}

void SkynetProductionManager::addArmyUnit( UnitType unit_type, double weight, Condition unit_condition, Condition production_condition )
{
	m_army_units.emplace_back( ArmyUnit{ unit_type, weight, unit_condition, production_condition } );
}