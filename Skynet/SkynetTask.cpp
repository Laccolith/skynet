#include "SkynetTask.h"

#include "SkynetTaskManager.h"
#include "SkynetTaskRequirementMineral.h"

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
	int current_time = BWAPI::Broodwar->getFrameCount();

	m_earliest_time = current_time;

	for( auto & requirement : m_requirements )
	{
		m_earliest_time = std::max( m_earliest_time, requirement->getEarliestTime() );
	}

	// A unit requirement has to be calculated last so that it doesn't overrun it's allotted time
	if( m_unit_requirement )
	{
		int travel_time = 0;
		m_earliest_time = std::max( m_earliest_time, m_unit_requirement->getEarliestTime( m_earliest_time, travel_time ) );

		if( m_earliest_time - travel_time <= current_time )
		{
			m_assigned_unit = m_unit_requirement->getChosenUnit();
		}
		else
		{
			m_assigned_unit = nullptr;
		}
	}
}

void SkynetTask::addRequirementMineral( int ammount )
{
	m_requirements.emplace_back( std::make_unique<SkynetTaskRequirementMineral>( ammount ) );
}
