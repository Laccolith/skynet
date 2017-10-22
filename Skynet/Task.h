#pragma once

#include "Types.h"

class TaskInterface
{
public:
	virtual int getPlannedTime() = 0;
	virtual Unit getAssignedUnit() = 0;

	virtual void addRequirementMineral( int amount ) = 0;
	virtual void addRequirementGas( int amount ) = 0;
	virtual void addRequirementSupply( int amount ) = 0;

	virtual ~TaskInterface() {}
};
