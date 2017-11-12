#pragma once

#include "Types.h"

#include <memory>

class TaskPriority
{
public:
	virtual ~TaskPriority() {}

	virtual void changePriority( double priority ) = 0;
};

class BuildLocation;
class TaskInterface
{
public:
	virtual bool requirementsFulfilled() const = 0;
	virtual int getPlannedTime() const  = 0;
	virtual Unit getAssignedUnit() const = 0;
	virtual TilePosition getBuildPosition() const = 0;
	virtual int getRemainingUnitTime() const = 0;
	virtual void requestUnitTimeChange( int time ) = 0;

	virtual void removeRequirement( int id ) = 0;

	virtual int addRequirementMineral( int amount ) = 0;
	virtual int addRequirementGas( int amount ) = 0;
	virtual int addRequirementSupply( int amount ) = 0;

	virtual int addRequirementUnitTypeAvailable( UnitType unit_type ) = 0;

	virtual int addRequirementUnit( UnitType unit_type ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Position starting_position, Position ending_position ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, std::unique_ptr<BuildLocation> build_location ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, int duration ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, int duration, Position starting_position, Position ending_position ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, int duration, std::unique_ptr<BuildLocation> build_location ) = 0;

	virtual int addRequirementUnit( UnitType unit_type, Region region ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Region region, Position starting_position, Position ending_position ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Region region, std::unique_ptr<BuildLocation> build_location ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Region region, int duration ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Region region, int duration, Position starting_position, Position ending_position ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Region region, int duration, std::unique_ptr<BuildLocation> build_location ) = 0;

	virtual int addRequirementUnit( UnitType unit_type, Base base ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Base base, Position starting_position, Position ending_position ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Base base, std::unique_ptr<BuildLocation> build_location ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Base base, int duration ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Base base, int duration, Position starting_position, Position ending_position ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Base base, int duration, std::unique_ptr<BuildLocation> build_location ) = 0;

	virtual int addRequirementUnit( Unit unit ) = 0;
	virtual int addRequirementUnit( Unit unit, Position starting_position, Position ending_position ) = 0;
	virtual int addRequirementUnit( Unit unit, std::unique_ptr<BuildLocation> build_location ) = 0;
	virtual int addRequirementUnit( Unit unit, int duration ) = 0;
	virtual int addRequirementUnit( Unit unit, int duration, Position starting_position, Position ending_position ) = 0;
	virtual int addRequirementUnit( Unit unit, int duration, std::unique_ptr<BuildLocation> build_location ) = 0;

	virtual void addOutputSupply( int time, int amount ) = 0;

	virtual void addOutputUnit( int time, UnitType unit_type ) = 0;

	virtual ~TaskInterface() {}
};
