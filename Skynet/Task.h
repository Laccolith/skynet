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

	virtual ~TaskInterface() {}
};
