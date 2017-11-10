#include "SkynetControlTaskTrain.h"

#include "TaskManager.h"
#include "Unit.h"

SkynetControlTaskTrain::SkynetControlTaskTrain( SkynetControlTaskFactory & skynet_control_task_factory, UnitType unit_type )
	: SkynetControlTask( skynet_control_task_factory )
	, m_unit_type( unit_type )
{
	createTask();
}

int SkynetControlTaskTrain::timeTillStart() const
{
	return m_task ? m_task->getPlannedTime() : 0;
}

bool SkynetControlTaskTrain::isInProgress() const
{
	return !m_task || m_build_unit;
}

bool SkynetControlTaskTrain::isComplete() const
{
	return !m_task;
}

void SkynetControlTaskTrain::preUpdate()
{
	if( m_build_unit && m_build_unit->isCompleted() )
	{
		m_task.reset();
		m_build_unit = nullptr;
	}

	if( !m_task )
	{
		return;
	}

	auto producer = m_task->getAssignedUnit();
	if( producer )
	{
		if( !producer->getTrainingQueue().empty() && producer->getTrainingQueue().front() == m_unit_type )
		{
			if( !m_build_unit )
			{
				for( auto id : m_reserved_resources )
				{
					m_task->removeRequirement( id );
				}

				m_reserved_resources.clear();
			}

			m_build_unit = producer->getBuildUnit();

			int remaining_train_time = producer->getRemainingTrainTime();
			int remaining_unit_time = m_task->getRemainingUnitTime();
			if( remaining_train_time != remaining_unit_time )
			{
				int time_change_required = remaining_train_time - remaining_unit_time;
				m_task->requestUnitTimeChange( time_change_required );
			}
		}
		else if( m_build_unit )
		{
			createTask();
			m_build_unit = nullptr;
		}
	}
}

void SkynetControlTaskTrain::postUpdate()
{
	if( !m_task )
	{
		return;
	}

	auto producer = m_task->getAssignedUnit();
	if( producer )
	{
		if( !producer->isTraining() )
		{
			producer->train( m_unit_type );
		}
		else if( !producer->getTrainingQueue().empty() )
		{
			if( producer->getTrainingQueue().front() != m_unit_type )
			{
				producer->cancel( 0 );
			}
			else if( producer->getTrainingQueue().size() > 1 )
			{
				producer->cancel();
			}
		}
	}
}

void SkynetControlTaskTrain::createTask()
{
	m_task = getAccess().getTaskManager().createTask( "Training - " + m_unit_type.getName() );

	if( m_unit_type.mineralPrice() > 0 )
		m_reserved_resources.push_back( m_task->addRequirementMineral( m_unit_type.mineralPrice() ) );
	if( m_unit_type.gasPrice() > 0 )
		m_reserved_resources.push_back( m_task->addRequirementGas( m_unit_type.gasPrice() ) );
	if( m_unit_type.supplyRequired() > 0 )
		m_reserved_resources.push_back( m_task->addRequirementSupply( m_unit_type.supplyRequired() ) );

	m_task->addRequirementUnit( m_unit_type.whatBuilds().first, m_unit_type.buildTime() + BWAPI::Broodwar->getLatencyFrames() );

	if( m_unit_type.supplyProvided() > 0 )
		m_task->addOutputSupply( m_unit_type.buildTime() + BWAPI::Broodwar->getLatencyFrames(), m_unit_type.supplyProvided() );

	m_task->addOutputUnit( m_unit_type.buildTime() + BWAPI::Broodwar->getLatencyFrames(), m_unit_type.whatBuilds().first );

	// TODO: Add required tech
}
