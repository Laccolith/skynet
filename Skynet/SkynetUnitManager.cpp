#include "SkynetUnitManager.h"

#include "Unit.h"

SkynetUnitManager::SkynetUnitManager( Core & core )
	: UnitManagerInterface( core )
{
	core.registerUpdateProcess( 2.0f, [this]() { update(); } );
}

void SkynetUnitManager::update()
{
	m_unit_timings.clear();
}

int SkynetUnitManager::getAvailableTime( Unit unit, int ideal_time, int required_duration ) const
{
	auto unit_it = m_unit_timings.find( unit );
	if( unit_it == m_unit_timings.end() )
		return ideal_time;

	auto & unit_timing = unit_it->second;

	int previous_time = 0;
	Position previous_pos = unit->getPosition();

	for( auto & time_point : unit_timing )
	{
		if( ideal_time < time_point.start_time )
		{
			// TODO: Create a better estimate using terrain analysis
			int travel_time = time_point.starting_position != Positions::None ? int( (previous_pos.getApproxDistance( time_point.starting_position ) * 1.6) / unit->getType().topSpeed() ) + 15 : 0;
			if( previous_time + travel_time + required_duration <= time_point.start_time )
				return ideal_time;
		}

		if( time_point.end_time == max_time )
			return max_time;

		previous_time = time_point.end_time;
		previous_pos = time_point.ending_position != Positions::None ? time_point.ending_position : time_point.starting_position != Positions::None ? time_point.starting_position : previous_pos;
	}

	return previous_time;
}

int SkynetUnitManager::getAvailableTime( Unit unit, int ideal_time, int required_duration, int & travel_time, Position starting_position, Position ending_position ) const
{
	//if( !unit->hasPath( m_position ) )
	//	continue;

	// TODO: Create a better estimate using terrain analysis
	//travel_time = int( (unit->getPosition().getApproxDistance( m_position ) * 1.6) / unit->getType().topSpeed() ) + 15;

	return max_time;
}

void SkynetUnitManager::reserveTaskUnit( Unit unit, int start_time, int end_time, Position starting_position, Position ending_position )
{
	auto & unit_timing = m_unit_timings[unit];
	auto it = std::upper_bound( unit_timing.begin(), unit_timing.end(), start_time, []( int start_time, auto & item ) { return item.start_time >= start_time; } );
	unit_timing.insert( it, UnitTiming{ start_time, end_time, starting_position, ending_position } );
}
