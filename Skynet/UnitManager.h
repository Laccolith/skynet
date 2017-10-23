#pragma once

#include "CoreModule.h"

#include "Types.h"

class UnitManagerInterface : public CoreModule
{
public:
	UnitManagerInterface( Core & core ) : CoreModule( core, "UnitManager" ) {}

	virtual int getAvailableTime( Unit unit, int ideal_time, int required_duration ) const = 0;
	virtual int getAvailableTime( Unit unit, int ideal_time, int required_duration, int & travel_time, Position starting_position, Position ending_position = Positions::None ) const = 0;

	virtual void reserveTaskUnit( Unit unit, int start_time, int end_time, Position starting_position = Positions::None, Position ending_position = Positions::None ) = 0;
};