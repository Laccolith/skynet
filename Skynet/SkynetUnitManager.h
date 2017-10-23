#pragma once

#include "UnitManager.h"

#include <map>
#include <vector>

class SkynetUnitManager : public UnitManagerInterface
{
public:
	SkynetUnitManager( Core & core );

	void update();

	int getAvailableTime( Unit unit, int ideal_time, int required_duration ) const override;
	int getAvailableTime( Unit unit, int ideal_time, int required_duration, int & travel_time, Position starting_position, Position ending_position = Positions::None ) const override;

	void reserveTaskUnit( Unit unit, int start_time, int end_time, Position starting_position = Positions::None, Position ending_position = Positions::None ) override;

private:
	struct UnitTiming
	{
		int start_time;
		int end_time;
		Position starting_position;
		Position ending_position;
	};
	std::map<Unit, std::vector<UnitTiming>> m_unit_timings;

	DEFINE_DEBUGGING_INTERFACE( Default );
};