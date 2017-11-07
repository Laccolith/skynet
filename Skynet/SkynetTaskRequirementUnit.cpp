#include "SkynetTaskRequirementUnit.h"

#include "PlayerTracker.h"
#include "UnitTracker.h"
#include "UnitManager.h"
#include "TerrainAnalyser.h"
#include "BaseTracker.h"

SkynetTaskRequirementUnitType::SkynetTaskRequirementUnitType( UnitType unit_type, int duration, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position )
	: m_unit_type( unit_type )
	, m_region( nullptr )
	, m_base( nullptr )
	, m_duration( duration )
	, m_position( std::move( position ) )
{
}

SkynetTaskRequirementUnitType::SkynetTaskRequirementUnitType( UnitType unit_type, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position )
	: m_unit_type( unit_type )
	, m_region( nullptr )
	, m_base( nullptr )
	, m_duration( max_time )
	, m_position( std::move( position ) )
{
}

SkynetTaskRequirementUnitType::SkynetTaskRequirementUnitType( UnitType unit_type, Region region, int duration, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position )
	: m_unit_type( unit_type )
	, m_region( region )
	, m_base( nullptr )
	, m_duration( duration )
	, m_position( std::move( position ) )
{
}

SkynetTaskRequirementUnitType::SkynetTaskRequirementUnitType( UnitType unit_type, Region region, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position )
	: m_unit_type( unit_type )
	, m_region( region )
	, m_base( nullptr )
	, m_duration( max_time )
	, m_position( std::move( position ) )
{
}

SkynetTaskRequirementUnitType::SkynetTaskRequirementUnitType( UnitType unit_type, Base base, int duration, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position )
	: m_unit_type( unit_type )
	, m_region( nullptr )
	, m_base( base )
	, m_duration( duration )
	, m_position( std::move( position ) )
{
}

SkynetTaskRequirementUnitType::SkynetTaskRequirementUnitType( UnitType unit_type, Base base, std::unique_ptr<SkynetTaskRequirementUnitPositionBase> position )
	: m_unit_type( unit_type )
	, m_region( nullptr )
	, m_base( base )
	, m_duration( max_time )
	, m_position( std::move( position ) )
{
}

Position getCenterPosition( UnitPosition pos )
{
	switch( pos.index() )
	{
	case 1:
		return std::get<1>( pos );

	case 2:
	{
		const BuildPosition & build_pos = std::get<2>( pos );
		return Position( build_pos.tile_position ) + Position( build_pos.unit_type.tileWidth() * 16, build_pos.unit_type.tileHeight() * 16 );
	}
	}

	return Positions::None;
}

int SkynetTaskRequirementUnitType::getReserveEarliestTime( CoreAccess & access, int current_earliest_time )
{
	UnitPosition starting_position;
	Position ending_position;
	m_position->get( current_earliest_time, starting_position, ending_position );

	const UnitGroup & applicable_units = access.getUnitTracker().getAllUnits( m_unit_type, access.getPlayerTracker().getLocalPlayer() );

	if( starting_position.index() == 0 )
		return chooseUnit( access, current_earliest_time, applicable_units, starting_position, ending_position );

	UnitGroup sorted_units = applicable_units;
	sorted_units.sortByDistance( getCenterPosition( starting_position ) );

	return chooseUnit( access, current_earliest_time, sorted_units, starting_position, ending_position );
}

void SkynetTaskRequirementUnitType::freeReserved( CoreAccess & access )
{
	if( m_is_reserved )
	{
		access.getUnitManager().freeTaskUnit( m_chosen_unit );
		m_position->freeReservation();
	}
}

Unit SkynetTaskRequirementUnitType::getChosenUnit() const
{
	return m_is_reserved ? m_chosen_unit : nullptr;
}

TilePosition SkynetTaskRequirementUnitType::getBuildPosition() const
{
	return m_position->getBuildPosition();
}

int SkynetTaskRequirementUnitType::getRemainingUnitTime( CoreAccess & access ) const
{
	return m_is_reserved ? access.getUnitManager().remainingReservedTaskTime( m_chosen_unit ) : 0;
}

void SkynetTaskRequirementUnitType::requestUnitTimeChange( CoreAccess & access, int time )
{
	if( m_is_reserved )
	{
		access.getUnitManager().modifyReservedTaskTime( m_chosen_unit, time );
	}
}

int SkynetTaskRequirementUnitType::chooseUnit( CoreAccess & access, int current_earliest_time, const UnitGroup & applicable_units, UnitPosition starting_position, Position ending_position )
{
	Unit previous_unit = m_chosen_unit;

	m_chosen_unit = nullptr;
	int best_time = max_time;
	int best_travel_time = 0;

	enum class ChoiceType
	{
		TestingPrevious,
		CompareToPrevious,
		TestingNew
	};

	auto update_for_unit = [&]( Unit unit, ChoiceType & choice_type ) -> bool
	{
		if( choice_type != ChoiceType::TestingPrevious && m_region && access.getTerrainAnalyser().getRegion( unit->getWalkPosition() ) != m_region )
			return false;

		if( choice_type != ChoiceType::TestingPrevious && m_base && access.getBaseTracker().getBaseTracker().getBase( unit->getTilePosition() ) != m_base )
			return false;

		// TODO: Allow choosing none existant units, but need to create a dependency to the task creating it
		// So that it doesn't cycle wether it can use or not
		// Still only choose it if it would be faster then using an existing unit
		if( !unit->exists() )
			return false;

		int unit_available_time = std::max( unit->getTimeTillCompleted(), current_earliest_time );

		if( starting_position.index() != 0 )
		{
			int travel_time = 0;
			int earliest_time = access.getUnitManager().getAvailableTime( unit, unit_available_time, m_duration, starting_position, ending_position, travel_time );

			if( choice_type == ChoiceType::CompareToPrevious )
			{
				if( earliest_time == current_earliest_time && best_travel_time <= travel_time + 36 )
					return true;
				else
					choice_type = ChoiceType::TestingNew;
			}

			if( earliest_time < best_time )
			{
				m_chosen_unit = unit;
				best_time = earliest_time;
				best_travel_time = travel_time;
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

		return best_time - best_travel_time <= current_earliest_time;
	};

	ChoiceType choice_type = ChoiceType::TestingNew;
	if( previous_unit && applicable_units.contains( previous_unit ) )
	{
		choice_type = ChoiceType::TestingPrevious;
		if( update_for_unit( previous_unit, choice_type ) )
			choice_type = ChoiceType::CompareToPrevious;
		else
			choice_type = ChoiceType::TestingNew;
	}

	for( Unit unit : applicable_units )
	{
		if( unit == previous_unit )
			continue;

		if( update_for_unit( unit, choice_type ) )
			break;
	}

	if( !m_chosen_unit )
		return max_time;

	int end_time = (best_time > (max_time - m_duration)) ? max_time : best_time + m_duration;

	m_position->reserve( best_time );
	access.getUnitManager().reserveTaskUnit( m_chosen_unit, best_time, end_time, starting_position, ending_position );

	if( best_time <= 0 )
	{
		m_is_reserved = true;
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
	UnitPosition starting_position;
	Position ending_position;
	m_position->get( current_earliest_time, starting_position, ending_position );

	int earliest_time = max_time;
	if( starting_position.index() != 0 )
	{
		int travel_time = 0;
		earliest_time = access.getUnitManager().getAvailableTime( m_unit, current_earliest_time, m_duration, starting_position, ending_position, travel_time );
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
	{
		access.getUnitManager().freeTaskUnit( m_unit );
		m_position->freeReservation();
	}
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

SkynetTaskRequirementUnitPosition::SkynetTaskRequirementUnitPosition( UnitPosition starting_position, Position ending_position )
	: m_starting_position( starting_position )
	, m_ending_position( ending_position )
{
}

void SkynetTaskRequirementUnitPosition::get( int & time, UnitPosition & starting_position, Position & ending_position )
{
	starting_position = m_starting_position;
	ending_position = m_ending_position;
}

void SkynetTaskRequirementUnitPosition::reserve( int time )
{
}

void SkynetTaskRequirementUnitPosition::freeReservation()
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

void SkynetTaskRequirementUnitPositionBuildLocation::get( int & time, UnitPosition & starting_position, Position & ending_position )
{
	time = m_build_location->calculatePosition( time );

	starting_position = BuildPosition{ m_build_location->getTilePosition(), m_build_location->getUnitType() };
	ending_position = Positions::None;
}

void SkynetTaskRequirementUnitPositionBuildLocation::reserve( int time )
{
	m_build_location->reservePosition( time );
}

void SkynetTaskRequirementUnitPositionBuildLocation::freeReservation()
{
	m_build_location->freeReservation();
}

TilePosition SkynetTaskRequirementUnitPositionBuildLocation::getBuildPosition() const
{
	return m_build_location->getTilePosition();
}
