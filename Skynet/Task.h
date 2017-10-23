#pragma once

#include "Types.h"

class TaskInterface
{
public:
	virtual bool requirementsFulfilled() const = 0;
	virtual int getPlannedTime() const  = 0;
	virtual Unit getAssignedUnit() const = 0;

	virtual void addRequirementMineral( int amount ) = 0;
	virtual void addRequirementGas( int amount ) = 0;
	virtual void addRequirementSupply( int amount ) = 0;

	virtual void addRequirementUnit( UnitType unit_type, int duration, Position starting_position = Positions::None, Position ending_position = Positions::None ) = 0;
	virtual void addRequirementUnit( UnitType unit_type, Position starting_position = Positions::None, Position ending_position = Positions::None ) = 0;

	virtual ~TaskInterface() {}
};
