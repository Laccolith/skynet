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
	int getAvailableTime( Unit unit, int ideal_time, int required_duration, Position starting_position, Position ending_position = Positions::None ) const override;

	void reserveTaskUnit( Unit unit, int start_time, int end_time, Position starting_position = Positions::None, Position ending_position = Positions::None ) override;
	void freeTaskUnit( Unit unit ) override;

private:
	struct UnitTimePoint
	{
		int start_time;
		int end_time;
		Position starting_position;
		Position ending_position;
	};
	struct UnitTiming
	{
		int available_time = 0;
		Position available_position = Positions::None;

		std::vector<UnitTimePoint> time_points;
	};
	std::map<Unit, UnitTiming> m_unit_timings;

	bool canTravel( Unit unit, Position starting_position, Position ending_position ) const;
	bool canFitInTravelTime( const std::vector<UnitTimePoint> & time_points, Unit unit, int from_time, int available_idle_time, Position previous_pos ) const;

	DEFINE_DEBUGGING_INTERFACE( Default );
};