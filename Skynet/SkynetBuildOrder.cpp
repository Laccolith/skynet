#include "SkynetBuildOrder.h"

#include "SkynetBuildOrderManager.h"
#include "ControlTaskFactory.h"
#include "BaseManager.h"
#include "SupplyManager.h"

SkynetBuildOrderItem::SkynetBuildOrderItem( SkynetBuildOrderManager & build_order_manager, SkynetBuildOrder & build_order, int first_item_id, int last_item_id )
	: m_build_order_manager( &build_order_manager )
	, m_build_order( &build_order )
	, m_first_item_id( first_item_id )
	, m_last_item_id( last_item_id )
{
}

Value<int> SkynetBuildOrderItem::timeTillStart()
{
	return Value<int>( [build_order_manager = m_build_order_manager, id = m_first_item_id]( CoreAccess & ) -> int
	{
		return build_order_manager->timeTillItemStarts( id );
	} );
}

Condition SkynetBuildOrderItem::isInProgress()
{
	return Condition( [build_order_manager = m_build_order_manager, id = m_first_item_id]( CoreAccess & ) -> bool
	{
		return build_order_manager->isItemInProgress( id );
	} );
}

Condition SkynetBuildOrderItem::isComplete()
{
	return Condition( [build_order_manager = m_build_order_manager, id = m_first_item_id]( CoreAccess & ) -> bool
	{
		return build_order_manager->isItemComplete( id );
	} );
}

SkynetBuildOrder::SkynetBuildOrder( SkynetBuildOrderManager & build_order_manager, std::string_view name )
	: m_build_order_manager( build_order_manager )
	, m_name( name )
{
}

SkynetBuildOrderItem SkynetBuildOrder::addItem( UnitType unit_type, int count, BuildLocationType build_location_type, Condition condition )
{
	int first_id = m_build_items.size();
	int last_id = first_id + count - 1;

	for( int i = 0; i < count; ++i )
	{
		m_build_items.emplace_back( [&build_order_manager = m_build_order_manager, unit_type, build_location_type]() -> std::unique_ptr<ControlTask>
		{
			return build_order_manager.getControlTaskFactory().createBuildControlTask( build_order_manager.getTaskPriority(), unit_type, build_location_type );
		}, condition );
	}

	return SkynetBuildOrderItem( m_build_order_manager, *this, first_id, last_id );
}

SkynetBuildOrderItem SkynetBuildOrder::addItem( UnitType unit_type, BuildLocationType build_location_type, Condition condition )
{
	return addItem( unit_type, 1, build_location_type, condition );
}

SkynetBuildOrderItem SkynetBuildOrder::addItem( UnitType unit_type, int count, Condition condition )
{
	return addItem( unit_type, count, BuildLocationType::Base, condition );
}

SkynetBuildOrderItem SkynetBuildOrder::addItem( UnitType unit_type, Condition condition )
{
	return addItem( unit_type, 1, BuildLocationType::Base, condition );
}

SkynetBuildOrderItem SkynetBuildOrder::addItem( TechType tech_type, Condition condition )
{
	int id = m_build_items.size();

	m_build_items.emplace_back( [&build_order_manager = m_build_order_manager, tech_type]() -> std::unique_ptr<ControlTask>
	{
		return build_order_manager.getControlTaskFactory().createResearchControlTask( build_order_manager.getTaskPriority(), tech_type );
	}, condition );

	return SkynetBuildOrderItem( m_build_order_manager, *this, id, id );
}

SkynetBuildOrderItem SkynetBuildOrder::addItem( UpgradeType upgrade_type, int level, Condition condition )
{
	int id = m_build_items.size();

	m_build_items.emplace_back( [&build_order_manager = m_build_order_manager, upgrade_type, level]() -> std::unique_ptr<ControlTask>
	{
		return build_order_manager.getControlTaskFactory().createUpgradeControlTask( build_order_manager.getTaskPriority(), upgrade_type, level );
	}, condition );

	return SkynetBuildOrderItem( m_build_order_manager, *this, id, id );
}

void SkynetBuildOrder::setAutoBuildWorkers( bool value, Condition condition )
{
	m_generic_items.emplace_back( [&build_order_manager = m_build_order_manager, value]()
	{
		build_order_manager.getBaseManager().setWorkerTraining( value );
	}, condition );
}

void SkynetBuildOrder::setAutoBuildSupply( bool value, Condition condition )
{
	m_generic_items.emplace_back( [&build_order_manager = m_build_order_manager, value]()
	{
		build_order_manager.getSupplyManager().setBuilding( value );
	}, condition );
}

void SkynetBuildOrder::addBuild( SkynetBuildOrder & next_build, Condition condition )
{
	m_generic_items.emplace_back( [&build_order_manager = m_build_order_manager, &next_build]()
	{
		build_order_manager.changeBuild( next_build );
	}, condition );
}

void SkynetBuildOrder::setStartingCondition( Condition starting_condition )
{
	m_starting_condition = starting_condition;
}

bool SkynetBuildOrder::canBeStartingBuild()
{
	return m_starting_condition.evaluate( m_build_order_manager );
}
