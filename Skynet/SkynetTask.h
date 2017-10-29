#pragma once

#include "Task.h"

#include "SkynetTaskRequirement.h"

#include <vector>
#include <memory>

class SkynetTaskManager;
class SkynetTask : public TaskInterface
{
public:
	SkynetTask( SkynetTaskManager & task_manager, std::string name );
	~SkynetTask();

	void updateTime();

	void drawInfo( int & y_pos );

	bool requirementsFulfilled() const override { return m_earliest_time <= 0; }
	int getPlannedTime() const override { return m_earliest_time; }
	Unit getAssignedUnit() const override { return m_assigned_unit; }
	TilePosition getBuildPosition() const override;
	int getRemainingUnitTime() const override;
	void requestUnitTimeChange( int time ) override;

	void removeRequirement( int id ) override;

	int addRequirementMineral( int amount ) override;
	int addRequirementGas( int amount ) override;
	int addRequirementSupply( int amount ) override;

	int addRequirementUnit( UnitType unit_type ) override;
	int addRequirementUnit( UnitType unit_type, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( UnitType unit_type, std::unique_ptr<BuildLocation> build_location ) override;
	int addRequirementUnit( UnitType unit_type, int duration ) override;
	int addRequirementUnit( UnitType unit_type, int duration, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( UnitType unit_type, int duration, std::unique_ptr<BuildLocation> build_location ) override;

	int addRequirementUnit( Unit unit ) override;
	int addRequirementUnit( Unit unit, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( Unit unit, std::unique_ptr<BuildLocation> build_location ) override;
	int addRequirementUnit( Unit unit, int duration ) override;
	int addRequirementUnit( Unit unit, int duration, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( Unit unit, int duration, std::unique_ptr<BuildLocation> build_location ) override;

private:
	SkynetTaskManager & m_task_manager;
	std::string m_name;

	int m_earliest_time = max_time;
	Unit m_assigned_unit = nullptr;

	int m_current_requirement_index = 0;
	std::vector<std::pair<std::unique_ptr<SkynetTaskRequirement>, int>> m_requirements;
	std::unique_ptr<SkynetTaskRequirementUnit> m_unit_requirement;
};