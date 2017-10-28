#include "SkynetTaskRequirementUnit.h"

#include "PlayerTracker.h"
#include "UnitTracker.h"
#include "UnitManager.h"

SkynetTaskRequirementUnitType::SkynetTaskRequirementUnitType( UnitType unit_type, int duration, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position )
	: m_unit_type( unit_type )
	, m_duration( duration )
	, m_position( std::move( position ) )
{
}

SkynetTaskRequirementUnitType::SkynetTaskRequirementUnitType( UnitType unit_type, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position )
	: m_unit_type( unit_type )
	, m_duration( max_time )
	, m_position( std::move( position ) )
{
}

int SkynetTaskRequirementUnitType::getReserveEarliestTime( CoreAccess & access, int current_earliest_time )
{
	Position starting_position;
	Position ending_position;
	m_position->get( current_earliest_time, starting_position, ending_position );

	const UnitGroup & applicable_units = access.getUnitTracker().getAllUnits( m_unit_type, access.getPlayerTracker().getLocalPlayer() );

	if( starting_position == Positions::None )
		return chooseUnit( access, current_earliest_time, applicable_units, starting_position, ending_position );

	UnitGroup sorted_units = applicable_units;
	sorted_units.sortByDistance( starting_position );

	return chooseUnit( access, current_earliest_time, sorted_units, starting_position, ending_position );
}

void SkynetTaskRequirementUnitType::freeReserved( CoreAccess & access )
{
	if( m_chosen_unit )
		access.getUnitManager().freeTaskUnit( m_chosen_unit );
}

Unit SkynetTaskRequirementUnitType::getChosenUnit() const
{
	return m_chosen_unit;
}

TilePosition SkynetTaskRequirementUnitType::getBuildPosition() const
{
	return m_position->getBuildPosition();
}

int SkynetTaskRequirementUnitType::getRemainingUnitTime( CoreAccess & access ) const
{
	return m_chosen_unit ? access.getUnitManager().remainingReservedTaskTime( m_chosen_unit ) : 0;
}

void SkynetTaskRequirementUnitType::requestUnitTimeChange( CoreAccess & access, int time )
{
	if( m_chosen_unit )
	{
		access.getUnitManager().modifyReservedTaskTime( m_chosen_unit, time );
	}
}

int SkynetTaskRequirementUnitType::chooseUnit( CoreAccess & access, int current_earliest_time, const UnitGroup & applicable_units, Position starting_position, Position ending_position )
{
	m_chosen_unit = nullptr;

	int best_time = max_time;

	for( Unit unit : applicable_units )
	{
		// TODO: Allow choosing none existant units, but need to create a dependency to the task creating it
		// So that it doesn't cycle wether it can use or not
		// Still only choose it if it would be faster then using an existing unit
		if( !unit->exists() )
			continue;

		int unit_available_time = std::max( unit->getTimeTillCompleted(), current_earliest_time );

		if( starting_position != Positions::None )
		{
			int earliest_time = access.getUnitManager().getAvailableTime( unit, unit_available_time, m_duration, starting_position, ending_position );
			if( earliest_time < best_time )
			{
				m_chosen_unit = unit;
				best_time = earliest_time;
			}
		}
		else
		{
			int earliest_time = access.getUnitManager().getAvailableTime( unit, unit_available_time, m_duration );
			if( earliest_time < best_time )
			{
				m_chosen_unit = unit;
				best_time = earliest_time;
			}
		}

		if( best_time == current_earliest_time )
			break;
	}

	if( !m_chosen_unit )
		return max_time;

	int end_time = (best_time > (max_time - m_duration)) ? max_time : best_time + m_duration;

	m_position->reserve( best_time );
	access.getUnitManager().reserveTaskUnit( m_chosen_unit, best_time, end_time, starting_position, ending_position );

	if( best_time > 0 )
	{
		m_chosen_unit = nullptr;
	}

	return best_time;
}

SkynetTaskRequirementUnitSpecific::SkynetTaskRequirementUnitSpecific( Unit unit, int duration, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position )
	: m_unit( unit )
	, m_duration( duration )
	, m_position( std::move( position ) )
{
}

SkynetTaskRequirementUnitSpecific::SkynetTaskRequirementUnitSpecific( Unit unit, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position )
	: m_unit( unit )
	, m_duration( max_time )
	, m_position( std::move( position ) )
{
}

int SkynetTaskRequirementUnitSpecific::getReserveEarliestTime( CoreAccess & access, int current_earliest_time )
{
	Position starting_position;
	Position ending_position;
	m_position->get( current_earliest_time, starting_position, ending_position );

	int earliest_time = max_time;
	if( starting_position != Positions::None )
	{
		earliest_time = access.getUnitManager().getAvailableTime( m_unit, current_earliest_time, m_duration, starting_position, ending_position );
	}
	else
	{
		earliest_time = access.getUnitManager().getAvailableTime( m_unit, current_earliest_time, m_duration );
	}

	if( earliest_time == max_time )
		return max_time;

	int end_time = (earliest_time > (max_time - m_duration)) ? max_time : earliest_time + m_duration;

	m_position->reserve( earliest_time );
	access.getUnitManager().reserveTaskUnit( m_unit, earliest_time, end_time, starting_position, ending_position );

	if( earliest_time <= 0 )
	{
		m_is_reserved = true;
	}

	return earliest_time;
}

void SkynetTaskRequirementUnitSpecific::freeReserved( CoreAccess & access )
{
	if( m_is_reserved )
		access.getUnitManager().freeTaskUnit( m_unit );
}

Unit SkynetTaskRequirementUnitSpecific::getChosenUnit() const
{
	return m_is_reserved ? m_unit : nullptr;
}

TilePosition SkynetTaskRequirementUnitSpecific::getBuildPosition() const
{
	return m_position->getBuildPosition();
}

int SkynetTaskRequirementUnitSpecific::getRemainingUnitTime( CoreAccess & access ) const
{
	return m_is_reserved ? access.getUnitManager().remainingReservedTaskTime( m_unit ) : 0;
}

void SkynetTaskRequirementUnitSpecific::requestUnitTimeChange( CoreAccess & access, int time )
{
	if( m_is_reserved )
	{
		access.getUnitManager().modifyReservedTaskTime( m_unit, time );
	}
}

SkynetTaskRequirementUnitPosition::SkynetTaskRequirementUnitPosition( Position starting_position, Position ending_position )
	: m_starting_position( starting_position )
	, m_ending_position( ending_position )
{
}

void SkynetTaskRequirementUnitPosition::get( int & time, Position & starting_position, Position & ending_position )
{
	starting_position = m_starting_position;
	ending_position = m_ending_position;
}

void SkynetTaskRequirementUnitPosition::reserve( int time )
{
}

TilePosition SkynetTaskRequirementUnitPosition::getBuildPosition() const
{
	return TilePositions::Invalid;
}

SkynetTaskRequirementUnitPositionBuildLocation::SkynetTaskRequirementUnitPositionBuildLocation( std::unique_ptr<BuildLocation> build_location )
	: m_build_location( std::move( build_location ) )
{
}

void SkynetTaskRequirementUnitPositionBuildLocation::get( int & time, Position & starting_position, Position & ending_position )
{
	time = m_build_location->calculatePosition( time );

	starting_position = ending_position = m_build_location->getPosition();
}

void SkynetTaskRequirementUnitPositionBuildLocation::reserve( int time )
{
	m_build_location->reservePosition( time );
}

TilePosition SkynetTaskRequirementUnitPositionBuildLocation::getBuildPosition() const
{
	return m_build_location->getTilePosition();
}
