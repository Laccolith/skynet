#include "SkynetUnitManager.h"

#include "Unit.h"

#include "TerrainAnalyser.h"

SkynetUnitManager::SkynetUnitManager( Core & core )
	: UnitManagerInterface( core )
{
	core.registerUpdateProcess( 2.0f, [this]() { update(); } );
}

void SkynetUnitManager::update()
{
	for( auto & timings : m_unit_timings )
	{
		timings.second.time_points.clear();

		if( timings.second.available_time > 1 )
			--timings.second.available_time;
	}
}

int getTravelTime( Unit unit, Position starting_position, Position ending_position )
{
	// If we are close enough, assume we are there, to counteract errors from movement
	int distance = starting_position.getApproxDistance( ending_position );
	if( distance < 16 )
		return 0;

	// TODO: Create a better estimate using terrain analysis
	return int( (distance * 1.2) / unit->getType().topSpeed() );
}

bool SkynetUnitManager::canTravel( Unit unit, Position starting_position, Position ending_position ) const
{
	if( !unit->getType().isFlyer() && !BWAPI::Broodwar->hasPath( starting_position, ending_position ) )
		return false;

	// Don't allow units to move across long distances as they could encounter harm
	// If a unit needs to be at a further location, another manager will take care of this
	if( getTerrainAnalyser().getRegion( WalkPosition( starting_position ) ) != getTerrainAnalyser().getRegion( WalkPosition( ending_position ) ) )
		return false;

	return true;
}

bool SkynetUnitManager::canFitInTravelTime( const std::vector<UnitTimePoint> & time_points, Unit unit, int from_time, int available_idle_time, Position previous_pos ) const
{
	for( auto & time_point : time_points )
	{
		if( from_time > time_point.start_time )
			continue;

		available_idle_time += time_point.start_time - from_time;

		if( time_point.starting_position != Positions::None )
		{
			if( !canTravel( unit, previous_pos, time_point.starting_position ) )
				return false;

			int travel_time = getTravelTime( unit, previous_pos, time_point.starting_position );
			return travel_time <= available_idle_time;
		}

		from_time = time_point.end_time;
	}

	return true;
}

int SkynetUnitManager::getAvailableTime( Unit unit, int ideal_time, int required_duration ) const
{
	auto unit_it = m_unit_timings.find( unit );
	if( unit_it == m_unit_timings.end() )
		return ideal_time;

	auto & unit_timing = unit_it->second;

	int previous_time = unit_timing.available_time;
	Position previous_pos = unit_timing.available_position != Positions::None ? unit_timing.available_position : unit->getPosition();
	int idle_time = 0;

	for( auto & time_point : unit_timing.time_points )
	{
		if( ideal_time + required_duration < time_point.start_time )
		{
			int travel_time = time_point.starting_position != Positions::None ? getTravelTime( unit, previous_pos, time_point.starting_position ) : 0;
			if( previous_time + required_duration + travel_time < time_point.start_time )
			{
				if( time_point.starting_position != Positions::None || canFitInTravelTime( unit_timing.time_points, unit, ideal_time + required_duration, idle_time, previous_pos ) )
				{
					return ideal_time;
				}
			}
		}

		if( time_point.end_time == max_time )
			return max_time;

		if( time_point.ending_position != Positions::None )
			idle_time = 0;
		else
			idle_time += time_point.start_time - previous_time;

		previous_time = time_point.end_time;
		previous_pos = time_point.ending_position != Positions::None ? time_point.ending_position : previous_pos;
	}

	return previous_time;
}

int SkynetUnitManager::getAvailableTime( Unit unit, int ideal_time, int required_duration, Position starting_position, Position ending_position ) const
{
	auto unit_it = m_unit_timings.find( unit );
	if( unit_it == m_unit_timings.end() )
		return ideal_time;

	auto & unit_timing = unit_it->second;

	int previous_time = unit_timing.available_time;
	Position previous_pos = unit_timing.available_position != Positions::None ? unit_timing.available_position : unit->getPosition();
	int idle_time = 0;

	for( auto & time_point : unit_timing.time_points )
	{
		if( ideal_time + required_duration < time_point.start_time && canTravel( unit, previous_pos, starting_position ) )
		{
			int current_travel_time = getTravelTime( unit, previous_pos, starting_position ) - idle_time;
			int earliest_start_time = std::max( ideal_time, previous_time + current_travel_time );

			if( earliest_start_time + required_duration < time_point.start_time )
			{
				int next_travel_time = time_point.starting_position != Positions::None ? getTravelTime( unit, ending_position, time_point.starting_position ) : 0;
				if( earliest_start_time + required_duration + next_travel_time < time_point.start_time )
				{
					if( time_point.starting_position != Positions::None || canFitInTravelTime( unit_timing.time_points, unit, earliest_start_time + required_duration, 0, ending_position ) )
					{
						return earliest_start_time;
					}
				}
			}
		}

		if( time_point.end_time == max_time )
			return max_time;

		if( time_point.ending_position != Positions::None )
			idle_time = 0;
		else
			idle_time += time_point.start_time - previous_time;

		previous_time = time_point.end_time;
		previous_pos = time_point.ending_position != Positions::None ? time_point.ending_position : previous_pos;
	}

	int travel_time = getTravelTime( unit, previous_pos, starting_position ) - idle_time;

	return previous_time + std::max( travel_time, 0 );
}

void SkynetUnitManager::reserveTaskUnit( Unit unit, int start_time, int end_time, Position starting_position, Position ending_position )
{
	auto & unit_timing = m_unit_timings[unit];

	if( start_time <= 0 )
	{
		unit_timing.available_time = end_time;
		unit_timing.available_position = ending_position;
		return;
	}

	auto & time_points = unit_timing.time_points;
	auto it = std::upper_bound( time_points.begin(), time_points.end(), start_time, []( int start_time, auto & item ) { return item.start_time >= start_time; } );
	time_points.insert( it, UnitTimePoint{ start_time, end_time, starting_position, ending_position } );
}

void SkynetUnitManager::freeTaskUnit( Unit unit )
{
	auto & unit_timing = m_unit_timings[unit];
	unit_timing.available_time = 0;
	unit_timing.available_position = Positions::None;
}

int SkynetUnitManager::remainingReservedTaskTime( Unit unit ) const
{
	auto it = m_unit_timings.find( unit );
	if( it == m_unit_timings.end() )
		return 0;

	return it->second.available_time;
}

void SkynetUnitManager::modifyReservedTaskTime( Unit unit, int time )
{
	auto it = m_unit_timings.find( unit );
	if( it != m_unit_timings.end() )
	{
		it->second.available_time += time;
	}
}

int SkynetUnitManager::getFreeTime( Unit unit ) const
{
	auto it = m_unit_timings.find( unit );
	if( it == m_unit_timings.end() )
		return max_time;

	if( it->second.available_time > 0 )
		return 0;

	if( it->second.time_points.empty() )
		return max_time;

	auto& time_point = it->second.time_points.front();

	int first_free_time = time_point.start_time;
	int previous_time = 0;
	int idle_time = 0;

	for( auto & time_point : it->second.time_points )
	{
		idle_time += time_point.start_time - previous_time;

		if( time_point.starting_position != Positions::None )
		{
			int travel_time = getTravelTime( unit, unit->getPosition(), time_point.starting_position );
			idle_time -= travel_time;

			return std::max( idle_time, first_free_time );
		}

		previous_time = time_point.end_time;
	}

	return first_free_time;
}
