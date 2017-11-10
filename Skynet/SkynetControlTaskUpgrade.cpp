#include "SkynetControlTaskUpgrade.h"

#include "TaskManager.h"
#include "Unit.h"

SkynetControlTaskUpgrade::SkynetControlTaskUpgrade( SkynetControlTaskFactory & skynet_control_task_factory, UpgradeType upgrade_type, int upgrade_level )
	: SkynetControlTask( skynet_control_task_factory )
	, m_upgrade_type( upgrade_type )
	, m_upgrade_level( upgrade_level )
{
	createTask();
}

int SkynetControlTaskUpgrade::timeTillStart() const
{
	return m_task ? m_task->getPlannedTime() : 0;
}

bool SkynetControlTaskUpgrade::isInProgress() const
{
	return !m_task || m_task->getAssignedUnit();
}

bool SkynetControlTaskUpgrade::isComplete() const
{
	return !m_task;
}

void SkynetControlTaskUpgrade::preUpdate()
{
	if( BWAPI::Broodwar->self()->getUpgradeLevel( m_upgrade_type ) >= m_upgrade_level )
	{
		m_task.reset();
	}

	if( !m_task )
	{
		return;
	}

	auto upgrader = m_task->getAssignedUnit();
	if( upgrader )
	{
		if( upgrader->getUpgrade() == m_upgrade_type )
		{
			if( !m_reserved_resources.empty() )
			{
				for( auto id : m_reserved_resources )
				{
					m_task->removeRequirement( id );
				}

				m_reserved_resources.clear();
			}

			int remaining_upgrade_time = upgrader->getRemainingUpgradeTime();
			int remaining_unit_time = m_task->getRemainingUnitTime();
			if( remaining_upgrade_time != remaining_unit_time )
			{
				int time_change_required = remaining_upgrade_time - remaining_unit_time;
				m_task->requestUnitTimeChange( time_change_required );
			}
		}
	}
}

void SkynetControlTaskUpgrade::postUpdate()
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
			producer->upgrade( m_upgrade_type );
		}
		else if( producer->getUpgrade() != m_upgrade_type )
		{
			producer->cancel();
		}
	}
}

void SkynetControlTaskUpgrade::createTask()
{
	m_task = getAccess().getTaskManager().createTask( "Upgrading - " + m_upgrade_type.getName() );

	if( m_upgrade_type.mineralPrice() > 0 )
		m_reserved_resources.push_back( m_task->addRequirementMineral( m_upgrade_type.mineralPrice() ) );
	if( m_upgrade_type.gasPrice() > 0 )
		m_reserved_resources.push_back( m_task->addRequirementGas( m_upgrade_type.gasPrice() ) );

	m_task->addRequirementUnit( m_upgrade_type.whatUpgrades(), m_upgrade_type.upgradeTime( m_upgrade_level ) + BWAPI::Broodwar->getLatencyFrames() );

	// TODO: Add required tech
}
