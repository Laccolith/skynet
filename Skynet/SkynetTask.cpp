#include "SkynetTask.h"

#include "SkynetTaskManager.h"
#include "SkynetTaskRequirementResource.h"
#include "SkynetTaskRequirementUnit.h"

#include "Types.h"

SkynetTask::SkynetTask( SkynetTaskManager & task_manager )
	: m_task_manager( task_manager )
{
}

SkynetTask::~SkynetTask()
{
	m_task_manager.onTaskDestroyed( *this );
}

void SkynetTask::updateTime()
{
	m_earliest_time = 0;

	for( auto & requirement : m_requirements )
	{
		m_earliest_time = std::max( m_earliest_time, requirement->getEarliestTime( m_task_manager ) );
	}

	// A unit requirement has to be calculated last so that it doesn't overrun it's allotted time
	if( m_unit_requirement && m_earliest_time < max_time )
	{
		int travel_time = 0;
		m_earliest_time = std::max( m_earliest_time, m_unit_requirement->getReserveEarliestTime( m_task_manager, m_earliest_time, travel_time ) );

		m_assigned_unit = travel_time >= m_earliest_time ? m_unit_requirement->getChosenUnit() : nullptr;
	}

	if( m_earliest_time < max_time )
	{
		for( auto & requirement : m_requirements )
		{
			requirement->reserveTime( m_task_manager, m_earliest_time );
		}
	}
}

void SkynetTask::addRequirementMineral( int amount )
{
	m_requirements.emplace_back( std::make_unique<SkynetTaskRequirementMineral>( amount ) );
}

void SkynetTask::addRequirementGas( int amount )
{
	m_requirements.emplace_back( std::make_unique<SkynetTaskRequirementGas>( amount ) );
}

void SkynetTask::addRequirementSupply( int amount )
{
	m_requirements.emplace_back( std::make_unique<SkynetTaskRequirementSupply>( amount ) );
}

void SkynetTask::addRequirementUnit( UnitType unit_type, int duration, Position starting_position, Position ending_position )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, duration, starting_position, ending_position );
}

void SkynetTask::addRequirementUnit( UnitType unit_type, Position starting_position, Position ending_position )
{
	// TODO: Log if there is already a unit requirement
	m_unit_requirement = std::make_unique<SkynetTaskRequirementUnitType>( unit_type, starting_position, ending_position );
}
