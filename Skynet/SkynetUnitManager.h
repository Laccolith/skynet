#pragma once

#include "UnitManager.h"
#include "UnitTracker.h"

#include <map>
#include <vector>

class SkynetUnitManager : public UnitManagerInterface, public MessageListener<UnitDestroy>
{
public:
	SkynetUnitManager( Core & core );

	void notify( const UnitDestroy & message ) override;

	void preUpdate();
	void postUpdate();

	int getAvailableTime( Unit unit, int ideal_time, int required_duration ) const override;
	int getAvailableTime( Unit unit, int ideal_time, int required_duration, UnitPosition starting_position, Position ending_position, int & out_travel_time ) const override;

	void reserveTaskUnit( Unit unit, int start_time, int end_time, UnitPosition starting_position, Position ending_position ) override;
	void freeTaskUnit( Unit unit ) override;

	int remainingReservedTaskTime( Unit unit ) const override;
	void modifyReservedTaskTime( Unit unit, int time ) override;

	int getFreeTime( Unit unit ) const override;

	void addOutputUnit( int time, UnitType unit_type, bool temporary ) override;
	void removeOutputUnit( int time, UnitType unit_type ) override;

	int earliestUnitOutputTime( UnitType unit_type ) const override;

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

	int getTravelTime( Unit unit, Position starting_position, UnitPosition ending_position ) const;
	bool canTravel( Unit unit, Position starting_position, UnitPosition ending_position ) const;
	bool canFitInTravelTime( const std::vector<UnitTimePoint> & time_points, Unit unit, int from_time, int available_idle_time, Position previous_pos ) const;

	std::map<UnitType, std::vector<std::pair<int, bool>>> m_task_output_units;

	DEFINE_DEBUGGING_INTERFACE( Default );
};