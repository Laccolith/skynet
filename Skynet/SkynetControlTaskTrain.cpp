#include "SkynetControlTaskTrain.h"

#include "TaskManager.h"
#include "Unit.h"

SkynetControlTaskTrain::SkynetControlTaskTrain( SkynetControlTaskFactory & skynet_control_task_factory, UnitType unit_type )
	: SkynetControlTask( skynet_control_task_factory )
	, m_unit_type( unit_type )
{
	createTask();
}

bool SkynetControlTaskTrain::isInProgress() const
{
	return !m_task || m_task->requirementsFulfilled();
}

bool SkynetControlTaskTrain::isFinished() const
{
	return !m_task;
}

void SkynetControlTaskTrain::preUpdate()
{
	if( m_will_complete_next_frame )
	{
		m_task.reset();
		return;
	}

	auto producer = m_task->getAssignedUnit();
	if( producer )
	{
		if( producer->isTraining() && producer->getTrainingQueue().front() == m_unit_type )
		{
			if( !m_has_started )
			{
				for( auto id : m_reserved_resources )
				{
					m_task->removeRequirement( id );
				}

				m_reserved_resources.clear();
				m_has_started = true;
			}

			int remaining_train_time = producer->getRemainingTrainTime();
			int remaining_unit_time = m_task->getRemainingUnitTime();
			if( remaining_train_time != remaining_unit_time )
			{
				int time_change_required = remaining_train_time - remaining_unit_time;
				m_task->requestUnitTimeChange( time_change_required );
			}

			if( remaining_train_time == 1 )
			{
				m_will_complete_next_frame = true;
			}
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
			if( m_has_started )
			{
				createTask();
				m_has_started = false;
			}
			else
			{
				producer->train( m_unit_type );
			}
		}
		else if( producer->getTrainingQueue().front() != m_unit_type )
		{
			producer->cancel( 0 );
		}
		else if( producer->getTrainingQueue().size() > 1 )
		{
			producer->cancel();
		}
	}
}

void SkynetControlTaskTrain::createTask()
{
	m_task = getAccess().getTaskManager().createTask();

	if( m_unit_type.mineralPrice() > 0 )
		m_reserved_resources.push_back( m_task->addRequirementMineral( m_unit_type.mineralPrice() ) );
	if( m_unit_type.gasPrice() > 0 )
		m_reserved_resources.push_back( m_task->addRequirementGas( m_unit_type.gasPrice() ) );
	if( m_unit_type.supplyRequired() > 0 )
		m_reserved_resources.push_back( m_task->addRequirementSupply( m_unit_type.supplyRequired() ) );

	m_task->addRequirementUnit( m_unit_type.whatBuilds().first, m_unit_type.buildTime() + BWAPI::Broodwar->getLatencyFrames() );

	// TODO: Add supply produced
	// TODO: Add required tech
}