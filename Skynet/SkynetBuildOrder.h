#pragma once

#include "Types.h"
#include "LazyQuery.h"
#include "ControlTask.h"
#include "BuildLocation.h"

#include <string>

class SkynetBuildOrder;
class SkynetBuildOrderManager;
class SkynetBuildOrderItem
{
public:
	SkynetBuildOrderItem( SkynetBuildOrderManager & build_order_manager, SkynetBuildOrder & build_order, int first_item_id, int last_item_id );

	Value<int> timeTillStart();
	Condition isInProgress();
	Condition isComplete();

private:
	SkynetBuildOrderManager * m_build_order_manager;
	SkynetBuildOrder * m_build_order;
	int m_first_item_id;
	int m_last_item_id;
};

class SkynetBuildOrder
{
public:
	SkynetBuildOrder( SkynetBuildOrderManager & build_order_manager, std::string_view name );

	SkynetBuildOrderItem addItem( UnitType unit_type, int count, BuildLocationType build_location_type, Condition condition = LazyQuery::conditionTrue() );
	SkynetBuildOrderItem addItem( UnitType unit_type, BuildLocationType build_location_type, Condition condition = LazyQuery::conditionTrue() );
	SkynetBuildOrderItem addItem( UnitType unit_type, int count, Condition condition = LazyQuery::conditionTrue() );
	SkynetBuildOrderItem addItem( UnitType unit_type, Condition condition = LazyQuery::conditionTrue() );
	SkynetBuildOrderItem addItem( TechType tech_type, Condition condition = LazyQuery::conditionTrue() );
	SkynetBuildOrderItem addItem( UpgradeType upgrade_type, int level = 1, Condition condition = LazyQuery::conditionTrue() );

	void setAutoBuildWorkers( bool value, Condition condition = LazyQuery::conditionTrue() );
	void setAutoBuildSupply( bool value, Condition condition = LazyQuery::conditionTrue() );
	void setAutoBuildArmy( bool value, Condition condition = LazyQuery::conditionTrue() );
	void setAutoBuildProduction( bool value, Condition condition = LazyQuery::conditionTrue() );
	void setAutoBuildTech( bool value, Condition condition = LazyQuery::conditionTrue() );

	void addArmyUnit( UnitType unit_type, double weight, Condition unit_condition = LazyQuery::conditionTrue(), Condition production_condition = LazyQuery::conditionTrue() );

	void addBuild( SkynetBuildOrder & next_build, Condition condition );

	void setStartingCondition( Condition starting_condition );
	bool canBeStartingBuild();

	const std::string & getName() const { return m_name; }

	auto getBuildItems() const { return m_build_items; }
	auto getGenericItems() const { return m_generic_items; }

private:
	SkynetBuildOrderManager & m_build_order_manager;
	std::string m_name;

	Condition m_starting_condition = LazyQuery::conditionFalse();

	std::vector<std::pair<std::function<std::unique_ptr<ControlTask>()>, Condition>> m_build_items;
	std::vector<std::pair<std::function<void()>, Condition>> m_generic_items;
};