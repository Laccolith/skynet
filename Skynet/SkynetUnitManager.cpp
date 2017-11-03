#include "SkynetUnitManager.h"

#include "Unit.h"

#include "TerrainAnalyser.h"

SkynetUnitManager::SkynetUnitManager( Core & core )
	: UnitManagerInterface( core )
{
	core.registerUpdateProcess( 2.0f, [this]() { preUpdate(); } );
	core.registerUpdateProcess( 5.0f, [this]() { postUpdate(); } );
}

void SkynetUnitManager::preUpdate()
{
	for( auto & timings : m_unit_timings )
	{
		timings.second.time_points.clear();

		if( timings.second.available_time > 1 )
			--timings.second.available_time;
	}
}

Position getTravelPosition( Position previous_pos, UnitPosition next_pos, bool safe_move_position )
{
	switch( next_pos.index() )
	{
	case 1:
		return std::get<1>( next_pos );

	case 2:
	{
		const BuildPosition & build_pos = std::get<2>( next_pos );

		if( safe_move_position )
			return Position( build_pos.tile_position ) + Position( build_pos.unit_type.tileSize() ) / 2;

		// TODO: based on the unit type it may need to get to a different position
		// Zerg units go to the middle for example, refineries always go to the edge

		int top = build_pos.tile_position.y * 32;
		int bottom = top + (build_pos.unit_type.tileHeight() * 32);
		int left = build_pos.tile_position.x * 32;
		int right = left + (build_pos.unit_type.tileWidth() * 32);

		if( previous_pos.x < left )
		{
			if( previous_pos.y < top )
				return Position( left, top );
			else if( previous_pos.y > bottom )
				return Position( left, bottom );
			else
				return Position( left, previous_pos.y );
		}
		else if( previous_pos.x > right )
		{
			if( previous_pos.y < top )
				return Position( right, top );
			else if( previous_pos.y > bottom )
				return Position( right, bottom );
			else
				return Position( right, previous_pos.y );
		}
		else
		{
			if( previous_pos.y < top )
				return Position( previous_pos.x, top );
			else if( previous_pos.y > bottom )
				return Position( previous_pos.x, bottom );
		}

		return previous_pos;
	}
	}

	return Positions::None;
}

void SkynetUnitManager::postUpdate()
{
	int current_latency = BWAPI::Broodwar->getRemainingLatencyFrames();

	for( auto & timings : m_unit_timings )
	{
		if( timings.second.available_time > 0 || timings.second.time_points.empty() )
			continue;

		Unit unit = timings.first;

		int free_time = getFreeTime( unit );

		if( free_time > current_latency )
			continue;

		UnitPosition required_position;

		for( auto & time_point : timings.second.time_points )
		{
			if( time_point.starting_position.index() != 0 )
			{
				required_position = time_point.starting_position;
				break;
			}
		}

		if( required_position.index() != 0 )
		{
			Position target_position = getTravelPosition( unit->getPosition(), required_position, true );

			if( isDebugging( Debug::Default ) )
				BWAPI::Broodwar->drawLineMap( unit->getPosition(), target_position, Colors::Orange );

			unit->move( target_position );
		}
	}
}

int SkynetUnitManager::getTravelTime( Unit unit, Position starting_position, UnitPosition ending_position, Position * out_actual_ending_position ) const
{
	Position actual_ending_position = getTravelPosition( starting_position, ending_position, false );
	if( out_actual_ending_position )
		*out_actual_ending_position = actual_ending_position;

	int distance = getTerrainAnalyser().getGroundDistance( WalkPosition( starting_position ), WalkPosition( actual_ending_position ) );

	// If we are close enough, assume we are there, to counteract errors from movement
	if( distance < 24 )
		return 0;

	// TODO: Create a better estimate using terrain analysis
	return int( (distance * 1.2) / unit->getType().topSpeed() );
}

bool SkynetUnitManager::canTravel( Unit unit, Position starting_position, UnitPosition ending_position ) const
{
	Position actual_ending_position = getTravelPosition( starting_position, ending_position, true );

	if( !unit->getType().isFlyer() && !BWAPI::Broodwar->hasPath( starting_position, actual_ending_position ) )
		return false;

	// TODO: Have a manager that knows of map control etc so workers don't travel really far to their deaths

	return true;
}

bool SkynetUnitManager::canFitInTravelTime( const std::vector<UnitTimePoint> & time_points, Unit unit, int from_time, int available_idle_time, Position previous_pos ) const
{
	for( auto & time_point : time_points )
	{
		if( from_time > time_point.start_time )
			continue;

		available_idle_time += time_point.start_time - from_time;

		if( time_point.starting_position.index() != 0 )
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
		Position actual_ending_position = Positions::None;
		if( ideal_time + required_duration < time_point.start_time )
		{
			int travel_time = time_point.starting_position.index() != 0 ? getTravelTime( unit, previous_pos, time_point.starting_position, &actual_ending_position ) : 0;
			if( previous_time + required_duration + travel_time < time_point.start_time )
			{
				if( time_point.starting_position.index() != 0 || canFitInTravelTime( unit_timing.time_points, unit, ideal_time + required_duration, idle_time, previous_pos ) )
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
		previous_pos = actual_ending_position != Positions::None ? actual_ending_position : previous_pos;
	}

	return std::max( previous_time, ideal_time );
}

int SkynetUnitManager::getAvailableTime( Unit unit, int ideal_time, int required_duration, UnitPosition starting_position, Position ending_position ) const
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
		Position actual_ending_position = Positions::None;
		if( ideal_time + required_duration < time_point.start_time && canTravel( unit, previous_pos, starting_position ) )
		{
			int current_travel_time = getTravelTime( unit, previous_pos, starting_position, &actual_ending_position ) - idle_time;
			int earliest_start_time = std::max( ideal_time, previous_time + current_travel_time );

			if( earliest_start_time + required_duration < time_point.start_time )
			{
				int next_travel_time = time_point.starting_position.index() != 0 ? getTravelTime( unit, ending_position, time_point.starting_position ) : 0;
				if( earliest_start_time + required_duration + next_travel_time < time_point.start_time )
				{
					if( time_point.starting_position.index() != 0 || canFitInTravelTime( unit_timing.time_points, unit, earliest_start_time + required_duration, 0, ending_position ) )
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
		previous_pos = actual_ending_position != Positions::None ? actual_ending_position : previous_pos;
	}

	int travel_time = getTravelTime( unit, previous_pos, starting_position ) - idle_time;

	return std::max( previous_time + std::max( travel_time, 0 ), ideal_time );
}

void SkynetUnitManager::reserveTaskUnit( Unit unit, int start_time, int end_time, UnitPosition starting_position, Position ending_position )
{
	auto & unit_timing = m_unit_timings[unit];

	if( start_time <= 0 )
	{
		unit_timing.available_time = end_time;
		unit_timing.available_position = ending_position != Positions::None ? ending_position : unit->getPosition();
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

	int time_till_first = it->second.time_points.front().start_time;
	int previous_time = 0;
	int idle_time = 0;

	for( auto & time_point : it->second.time_points )
	{
		idle_time += time_point.start_time - previous_time;

		if( time_point.starting_position.index() != 0 )
		{
			int travel_time = getTravelTime( unit, unit->getPosition(), time_point.starting_position );
			idle_time -= travel_time;

			return std::min( std::max( idle_time, 0 ), time_till_first );
		}

		previous_time = time_point.end_time;
	}

	return time_till_first;
}
