#include "SkynetTask.h"

#include "SkynetTaskManager.h"
#include "SkynetTaskRequirementResource.h"
#include "SkynetTaskRequirementUnit.h"
#include "ResourceManager.h"

#include "Types.h"

SkynetTask::SkynetTask( SkynetTaskManager & task_manager, std::string name )
	: m_task_manager( task_manager )
	, m_name( std::move( name ) )
{
}

SkynetTask::~SkynetTask()
{
	for( auto & requirement : m_requirements )
	{
		requirement.first->freeReserved( m_task_manager );
	}

	if( m_unit_requirement )
	{
		m_unit_requirement->freeReserved( m_task_manager );
	}

	if( m_earliest_time > 0 )
	{
		m_task_manager.onTaskDestroyed( *this );
	}
	else if( m_supply_output > 0 )
	{
		m_task_manager.getResourceManager().removeTaskSupplyOutput( m_earliest_time + m_supply_output_time, m_supply_output );
	}
}

void SkynetTask::updateTime()
{
	m_earliest_time = 0;

	for( auto & requirement : m_requirements )
	{
		m_earliest_time = std::max( m_earliest_time, requirement.first->getEarliestTime( m_task_manager ) );
	}

	// A unit requirement has to be calculated last so that it doesn't overrun it's allotted time
	if( m_unit_requirement && m_earliest_time < max_time )
	{
		m_earliest_time = m_unit_requirement->getReserveEarliestTime( m_task_manager, m_earliest_time );

		m_assigned_unit = m_unit_requirement->getChosenUnit();
	}

	if( m_earliest_time < max_time )
	{
		for( auto & requirement : m_requirements )
		{
			requirement.first->reserveTime( m_task_manager, m_earliest_time );
		}

		if( m_supply_output > 0 )
		{
			m_task_manager.getResourceManager().addTaskSupplyOutput( m_earliest_time + m_supply_output_time, m_supply_output, m_earliest_time > 0 );
		}
	}
}

void SkynetTask::drawInfo( int & y_pos )
{
	if( m_earliest_time > 2000 )
		return;

	if( m_earliest_time == max_time )
		BWAPI::Broodwar->drawTextScreen( 10, y_pos, "NA" );
	else
		BWAPI::Broodwar->drawTextScreen( 10, y_pos, "%d", m_earliest_time );

	BWAPI::Broodwar->drawTextScreen( 40, y_pos, "- %s", m_name.c_str() );

	y_pos += 8;
}

TilePosition SkynetTask::getBuildPosition() const
{
	return m_unit_requirement ? m_unit_requirement->getBuildPosition() : TilePositions::None;
}

int SkynetTask::getRemainingUnitTime() const
{
	return m_unit_requirement ? m_unit_requirement->getRemainingUnitTime( m_task_manager ) : 0;
}

void SkynetTask::requestUnitTimeChange( int time )
{
	if( m_unit_requirement )
	{
		m_unit_requirement->requestUnitTimeChange( m_task_manager, time );
	}
}

void SkynetTask::removeRequirement( int id )
{
	if( id == -1 )
	{
		m_unit_requirement->freeReserved( m_task_manager );
		m_unit_requirement.reset();
		m_assigned_unit = nullptr;
	}
	else
	{
		auto it = std::find_if( m_requirements.begin(), m_requirements.end(), [id]( const auto & requirement ) { return requirement.second == id; } );
		if( it != m_requirements.end() )
		{
			it->first->freeReserved( m_task_manager );
			std::swap( *it, m_requirements.back() );
			m_requirements.pop_back();
		}
	}
}

int SkynetTask::addRequirementMineral( int amount )
{
	m_requirements.emplace_back( std::make_unique<SkynetTaskRequirementMineral>( amount ), m_current_requirement_index++ );
	return m_requirements.back().second;
}

int SkynetTask::addRequirementGas( int amount )
{
	m_requirements.emplace_back( std::make_unique<SkynetTaskRequirementGas>( amount ), m_current_requirement_index++ );
	return m_requirements.back().second;
}

int SkynetTask::addRequirementSupply( int amount )
{
	m_requirements.emplace_back( std::make_unique<SkynetTaskRequirementSupply>( amount ), m_current_requirement_index++ );
	return m_requirements.back().second;
}

int SkynetTask::addRequirementUnit( UnitType unit_type )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, std::make_unique<SkynetTaskRequirementUnitPosition>() );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Position starting_position, Position ending_position )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, std::make_unique<SkynetTaskRequirementUnitPosition>( starting_position, ending_position ) );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, std::unique_ptr<BuildLocation> build_location )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, std::make_unique<SkynetTaskRequirementUnitPositionBuildLocation>( std::move( build_location ) ) );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, int duration )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, duration, std::make_unique<SkynetTaskRequirementUnitPosition>() );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, int duration, Position starting_position, Position ending_position )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, duration, std::make_unique<SkynetTaskRequirementUnitPosition>( starting_position, ending_position ) );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, int duration, std::unique_ptr<BuildLocation> build_location )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, duration, std::make_unique<SkynetTaskRequirementUnitPositionBuildLocation>( std::move( build_location ) ) );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Region region )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, region, std::make_unique<SkynetTaskRequirementUnitPosition>() );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Region region, Position starting_position, Position ending_position )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, region, std::make_unique<SkynetTaskRequirementUnitPosition>( starting_position, ending_position ) );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Region region, std::unique_ptr<BuildLocation> build_location )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, region, std::make_unique<SkynetTaskRequirementUnitPositionBuildLocation>( std::move( build_location ) ) );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Region region, int duration )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, region, duration, std::make_unique<SkynetTaskRequirementUnitPosition>() );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Region region, int duration, Position starting_position, Position ending_position )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, region, duration, std::make_unique<SkynetTaskRequirementUnitPosition>( starting_position, ending_position ) );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Region region, int duration, std::unique_ptr<BuildLocation> build_location )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, region, duration, std::make_unique<SkynetTaskRequirementUnitPositionBuildLocation>( std::move( build_location ) ) );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Base base )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, base, std::make_unique<SkynetTaskRequirementUnitPosition>() );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Base base, Position starting_position, Position ending_position )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, base, std::make_unique<SkynetTaskRequirementUnitPosition>( starting_position, ending_position ) );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Base base, std::unique_ptr<BuildLocation> build_location )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, base, std::make_unique<SkynetTaskRequirementUnitPositionBuildLocation>( std::move( build_location ) ) );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Base base, int duration )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, base, duration, std::make_unique<SkynetTaskRequirementUnitPosition>() );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Base base, int duration, Position starting_position, Position ending_position )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, base, duration, std::make_unique<SkynetTaskRequirementUnitPosition>( starting_position, ending_position ) );
	return -1;
}

int SkynetTask::addRequirementUnit( UnitType unit_type, Base base, int duration, std::unique_ptr<BuildLocation> build_location )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, base, duration, std::make_unique<SkynetTaskRequirementUnitPositionBuildLocation>( std::move( build_location ) ) );
	return -1;
}

int SkynetTask::addRequirementUnit( Unit unit )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitSpecific>( unit, std::make_unique<SkynetTaskRequirementUnitPosition>() );
	return -1;
}

int SkynetTask::addRequirementUnit( Unit unit, Position starting_position, Position ending_position )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitSpecific>( unit, std::make_unique<SkynetTaskRequirementUnitPosition>( starting_position, ending_position ) );
	return -1;
}

int SkynetTask::addRequirementUnit( Unit unit, std::unique_ptr<BuildLocation> build_location )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitSpecific>( unit, std::make_unique<SkynetTaskRequirementUnitPositionBuildLocation>( std::move( build_location ) ) );
	return -1;
}

int SkynetTask::addRequirementUnit( Unit unit, int duration )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitSpecific>( unit, duration, std::make_unique<SkynetTaskRequirementUnitPosition>() );
	return -1;
}

int SkynetTask::addRequirementUnit( Unit unit, int duration, Position starting_position, Position ending_position )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitSpecific>( unit, duration, std::make_unique<SkynetTaskRequirementUnitPosition>( starting_position, ending_position ) );
	return -1;
}

int SkynetTask::addRequirementUnit( Unit unit, int duration, std::unique_ptr<BuildLocation> build_location )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitSpecific>( unit, duration, std::make_unique<SkynetTaskRequirementUnitPositionBuildLocation>( std::move( build_location ) ) );
	return -1;
}

void SkynetTask::addOutputSupply( int time, int amount )
{
	m_supply_output = amount;
	m_supply_output_time = time;
}

void SkynetTask::addOutputUnit( int time, UnitType unit_type )
{
}
