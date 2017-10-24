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

	void addRequirementMineral( int amount ) override;
	void addRequirementGas( int amount ) override;
	void addRequirementSupply( int amount ) override;

	void addRequirementUnit( UnitType unit_type ) override;
	void addRequirementUnit( UnitType unit_type, Position starting_position, Position ending_position ) override;
	void addRequirementUnit( UnitType unit_type, int duration ) override;
	void addRequirementUnit( UnitType unit_type, int duration, Position starting_position, Position ending_position ) override;

	void addRequirementUnit( Unit unit ) override;
	void addRequirementUnit( Unit unit, Position starting_position, Position ending_position ) override;
	void addRequirementUnit( Unit unit, int duration ) override;
	void addRequirementUnit( Unit unit, int duration, Position starting_position, Position ending_position ) override;

private:
	SkynetTaskManager & m_task_manager;

	int m_earliest_time = max_time;
	Unit m_assigned_unit = nullptr;

	std::vector<std::unique_ptr<SkynetTaskRequirement>> m_requirements;
	std::unique_ptr<SkynetTaskRequirementUnit> m_unit_requirement;
};