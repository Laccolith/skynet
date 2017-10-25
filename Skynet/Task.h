#pragma once

#include "Types.h"

class TaskInterface
{
public:
	virtual bool requirementsFulfilled() const = 0;
	virtual int getPlannedTime() const  = 0;
	virtual Unit getAssignedUnit() const = 0;
	virtual int getRemainingUnitTime() const = 0;
	virtual void requestUnitTimeChange( int time ) = 0;

	virtual void removeRequirement( int id ) = 0;

	virtual int addRequirementMineral( int amount ) = 0;
	virtual int addRequirementGas( int amount ) = 0;
	virtual int addRequirementSupply( int amount ) = 0;

	virtual int addRequirementUnit( UnitType unit_type ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, Position starting_position, Position ending_position ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, int duration ) = 0;
	virtual int addRequirementUnit( UnitType unit_type, int duration, Position starting_position, Position ending_position ) = 0;

	virtual int addRequirementUnit( Unit unit ) = 0;
	virtual int addRequirementUnit( Unit unit, Position starting_position, Position ending_position ) = 0;
	virtual int addRequirementUnit( Unit unit, int duration ) = 0;
	virtual int addRequirementUnit( Unit unit, int duration, Position starting_position, Position ending_position ) = 0;

	virtual ~TaskInterface() {}
};
