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

	int getPlannedTime() override { return m_earliest_time; }
	Unit getAssignedUnit() override { return m_assigned_unit; }

	void addRequirementMineral( int amount ) override;
	void addRequirementGas( int amount ) override;
	void addRequirementSupply( int amount ) override;

private:
	SkynetTaskManager & m_task_manager;

	bool m_active = false;
	int m_earliest_time = requirement_max_time;
	Unit m_assigned_unit = nullptr;

	std::vector<std::unique_ptr<SkynetTaskRequirement>> m_requirements;
	std::unique_ptr<SkynetTaskRequirementUnit> m_unit_requirement;
};