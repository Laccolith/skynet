#pragma once

#include "Task.h"

#include "SkynetTaskRequirement.h"

#include <vector>
#include <memory>

class SkynetTaskManager;
class SkynetTask : public TaskInterface
{
public:
	SkynetTask( SkynetTaskManager & task_manager );
	~SkynetTask();

	void updateTime();

	bool requirementsFulfilled() const override { return m_earliest_time <= 0; }
	int getPlannedTime() const override { return m_earliest_time; }
	Unit getAssignedUnit() const override { return m_assigned_unit; }
	int getRemainingUnitTime() const override;
	void requestUnitTimeChange( int time ) override;

	void removeRequirement( int id ) override;

	int addRequirementMineral( int amount ) override;
	int addRequirementGas( int amount ) override;
	int addRequirementSupply( int amount ) override;

	int addRequirementUnit( UnitType unit_type ) override;
	int addRequirementUnit( UnitType unit_type, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( UnitType unit_type, int duration ) override;
	int addRequirementUnit( UnitType unit_type, int duration, Position starting_position, Position ending_position ) override;

	int addRequirementUnit( Unit unit ) override;
	int addRequirementUnit( Unit unit, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( Unit unit, int duration ) override;
	int addRequirementUnit( Unit unit, int duration, Position starting_position, Position ending_position ) override;

private:
	SkynetTaskManager & m_task_manager;

	int m_earliest_time = max_time;
	Unit m_assigned_unit = nullptr;

	int m_current_requirement_index = 0;
	std::vector<std::pair<std::unique_ptr<SkynetTaskRequirement>, int>> m_requirements;
	std::unique_ptr<SkynetTaskRequirementUnit> m_unit_requirement;
};