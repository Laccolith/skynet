#pragma once

#include "UnitManager.h"

#include <map>
#include <vector>

class SkynetUnitManager : public UnitManagerInterface
{
public:
	SkynetUnitManager( Core & core );

	void preUpdate();
	void postUpdate();

	int getAvailableTime( Unit unit, int ideal_time, int required_duration ) const override;
	int getAvailableTime( Unit unit, int ideal_time, int required_duration, UnitPosition starting_position, Position ending_position = Positions::None ) const override;

	void reserveTaskUnit( Unit unit, int start_time, int end_time, UnitPosition starting_position = UnitPosition(), Position ending_position = Positions::None ) override;
	void freeTaskUnit( Unit unit ) override;

	int remainingReservedTaskTime( Unit unit ) const override;
	void modifyReservedTaskTime( Unit unit, int time ) override;

	int getFreeTime( Unit unit ) const override;

private:
	struct UnitTimePoint
	{
		int start_time;
		int end_time;
		UnitPosition starting_position;
		Position ending_position;
	};
	struct UnitTiming
	{
		int available_time = 0;
		Position available_position = Positions::None;

		std::vector<UnitTimePoint> time_points;
	};
	std::map<Unit, UnitTiming> m_unit_timings;

	int getTravelTime( Unit unit, Position starting_position, UnitPosition ending_position, Position * out_actual_ending_position = nullptr ) const;
	bool canTravel( Unit unit, Position starting_position, UnitPosition ending_position ) const;
	bool canFitInTravelTime( const std::vector<UnitTimePoint> & time_points, Unit unit, int from_time, int available_idle_time, Position previous_pos ) const;

	DEFINE_DEBUGGING_INTERFACE( Default );
};