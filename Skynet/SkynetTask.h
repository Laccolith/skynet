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

	int addRequirementUnitTypeAvailable( UnitType unit_type ) override;

	int addRequirementUnit( UnitType unit_type ) override;
	int addRequirementUnit( UnitType unit_type, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( UnitType unit_type, std::unique_ptr<BuildLocation> build_location ) override;
	int addRequirementUnit( UnitType unit_type, int duration ) override;
	int addRequirementUnit( UnitType unit_type, int duration, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( UnitType unit_type, int duration, std::unique_ptr<BuildLocation> build_location ) override;

	int addRequirementUnit( UnitType unit_type, Region region ) override;
	int addRequirementUnit( UnitType unit_type, Region region, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( UnitType unit_type, Region region, std::unique_ptr<BuildLocation> build_location ) override;
	int addRequirementUnit( UnitType unit_type, Region region, int duration ) override;
	int addRequirementUnit( UnitType unit_type, Region region, int duration, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( UnitType unit_type, Region region, int duration, std::unique_ptr<BuildLocation> build_location ) override;

	int addRequirementUnit( UnitType unit_type, Base base ) override;
	int addRequirementUnit( UnitType unit_type, Base base, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( UnitType unit_type, Base base, std::unique_ptr<BuildLocation> build_location ) override;
	int addRequirementUnit( UnitType unit_type, Base base, int duration ) override;
	int addRequirementUnit( UnitType unit_type, Base base, int duration, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( UnitType unit_type, Base base, int duration, std::unique_ptr<BuildLocation> build_location ) override;

	int addRequirementUnit( Unit unit ) override;
	int addRequirementUnit( Unit unit, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( Unit unit, std::unique_ptr<BuildLocation> build_location ) override;
	int addRequirementUnit( Unit unit, int duration ) override;
	int addRequirementUnit( Unit unit, int duration, Position starting_position, Position ending_position ) override;
	int addRequirementUnit( Unit unit, int duration, std::unique_ptr<BuildLocation> build_location ) override;

	void addOutputSupply( int time, int amount ) override;

	void addOutputUnit( int time, UnitType unit_type ) override;

private:
	SkynetTaskManager & m_task_manager;
	std::string m_name;

	int m_earliest_time = max_time;
	Unit m_assigned_unit = nullptr;

	int m_current_requirement_index = 0;
	std::vector<std::pair<std::unique_ptr<SkynetTaskRequirement>, int>> m_requirements;
	std::unique_ptr<SkynetTaskRequirementUnit> m_unit_requirement;

	int m_supply_output = 0;
	int m_supply_output_time = 0;

	UnitType m_unit_type_output = UnitTypes::None;
	int m_unit_type_output_time = 0;
};