#pragma once

#include "CoreModule.h"

#include "Types.h"

#include <variant>

struct BuildPosition
{
	TilePosition tile_position;
	UnitType unit_type;
};

typedef std::variant<std::monostate, Position, BuildPosition> UnitPosition;

class UnitManagerInterface : public CoreModule
{
public:
	UnitManagerInterface( Core & core ) : CoreModule( core, "UnitManager" ) {}

	virtual int getAvailableTime( Unit unit, int ideal_time, int required_duration ) const = 0;
	virtual int getAvailableTime( Unit unit, int ideal_time, int required_duration, UnitPosition starting_position, Position ending_position = Positions::None ) const = 0;

	virtual void reserveTaskUnit( Unit unit, int start_time, int end_time, UnitPosition starting_position = UnitPosition(), Position ending_position = Positions::None ) = 0;
	virtual void freeTaskUnit( Unit unit ) = 0;

	virtual int remainingReservedTaskTime( Unit unit ) const = 0;
	virtual void modifyReservedTaskTime( Unit unit, int time ) = 0;

	virtual int getFreeTime( Unit unit ) const = 0;
};