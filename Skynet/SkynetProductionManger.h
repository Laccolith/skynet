#pragma once

#include "ProductionManager.h"

#include "BuildOrderManager.h"
#include "TaskManager.h"

class ControlTask;
class SkynetProductionManager : public ProductionManagerInterface, public MessageListener<BuildOrderChanged>
{
public:
	SkynetProductionManager( Core & core );

	void notify( const BuildOrderChanged & message ) override;

	void update();

	void setArmyBuilding( bool enabled ) override { m_can_build_army = enabled; }
	void setProductionBuilding( bool enabled ) override { m_can_build_production = enabled; }
	void setTechBuilding( bool enabled ) override { m_can_build_tech = enabled; }

	void addArmyUnit( UnitType unit_type, double weight, Condition unit_condition, Condition production_condition ) override;

private:
	bool m_can_build_army = false;
	bool m_can_build_production = false;
	bool m_can_build_tech = false;

	TaskPriority * m_build_army_priority = nullptr;

	struct ArmyUnit
	{
		UnitType unit_type;
		double weight;
		Condition unit_condition;
		Condition production_condition;
	};
	std::vector<ArmyUnit> m_army_units;

	std::map<UnitType, std::vector<std::unique_ptr<ControlTask>>> m_planned_army_units;
};