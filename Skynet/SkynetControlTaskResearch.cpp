#include "SkynetControlTaskResearch.h"

#include "TaskManager.h"
#include "Unit.h"

SkynetControlTaskResearch::SkynetControlTaskResearch( SkynetControlTaskFactory & skynet_control_task_factory, TaskPriority * priority, TechType tech_type )
	: SkynetControlTask( skynet_control_task_factory )
	, m_priority( priority )
	, m_tech_type( tech_type )
{
	createTask();
}

int SkynetControlTaskResearch::timeTillStart() const
{
	return m_task ? m_task->getPlannedTime() : 0;
}

bool SkynetControlTaskResearch::isInProgress() const
{
	return !m_task || m_task->getAssignedUnit();
}

bool SkynetControlTaskResearch::isComplete() const
{
	return !m_task;
}

void SkynetControlTaskResearch::preUpdate()
{
	if( BWAPI::Broodwar->self()->hasResearched( m_tech_type ) )
	{
		m_task.reset();
	}

	if( !m_task )
	{
		return;
	}

	auto researcher = m_task->getAssignedUnit();
	if( researcher )
	{
		if( researcher->getTech() == m_tech_type )
		{
			if( !m_reserved_resources.empty() )
			{
				for( auto id : m_reserved_resources )
				{
					m_task->removeRequirement( id );
				}

				m_reserved_resources.clear();
			}

			int remaining_research_time = researcher->getRemainingResearchTime();
			int remaining_unit_time = m_task->getRemainingUnitTime();
			if( remaining_research_time != remaining_unit_time )
			{
				int time_change_required = remaining_research_time - remaining_unit_time;
				m_task->requestUnitTimeChange( time_change_required );
			}
		}
	}
}

void SkynetControlTaskResearch::postUpdate()
{
	if( !m_task )
	{
		return;
	}

	auto producer = m_task->getAssignedUnit();
	if( producer )
	{
		if( !producer->isResearching() )
		{
			producer->research( m_tech_type );
		}
		else if( producer->getTech() != m_tech_type )
		{
			producer->cancel();
		}
	}
}

void SkynetControlTaskResearch::createTask()
{
	m_task = getAccess().getTaskManager().createTask( "Researching - " + m_tech_type.getName(), m_priority );

	if( m_tech_type.mineralPrice() > 0 )
		m_reserved_resources.push_back( m_task->addRequirementMineral( m_tech_type.mineralPrice() ) );
	if( m_tech_type.gasPrice() > 0 )
		m_reserved_resources.push_back( m_task->addRequirementGas( m_tech_type.gasPrice() ) );

	m_task->addRequirementUnit( m_tech_type.whatResearches(), m_tech_type.researchTime() + BWAPI::Broodwar->getLatencyFrames() );

	if( m_tech_type.requiredUnit() != UnitTypes::None )
		m_task->addRequirementUnitTypeAvailable( m_tech_type.requiredUnit() );
}
