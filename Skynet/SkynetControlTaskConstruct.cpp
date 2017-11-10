#include "SkynetControlTaskConstruct.h"

#include "TaskManager.h"
#include "Unit.h"
#include "BuildLocationManager.h"

SkynetControlTaskConstruct::SkynetControlTaskConstruct( SkynetControlTaskFactory & skynet_control_task_factory, TaskPriority * priority, UnitType unit_type, BuildLocationType build_location_type )
	: SkynetControlTask( skynet_control_task_factory )
	, m_priority( priority )
	, m_unit_type( unit_type )
	, m_build_location_type( build_location_type )
{
	createTask();
}

int SkynetControlTaskConstruct::timeTillStart() const
{
	return m_task ? m_task->getPlannedTime() : 0;
}

bool SkynetControlTaskConstruct::isInProgress() const
{
	return m_build_unit || m_has_finished;
}

bool SkynetControlTaskConstruct::isComplete() const
{
	return m_has_finished;
}

void SkynetControlTaskConstruct::preUpdate()
{
	if( m_build_unit )
	{
		if( m_unit_discover_listener || m_unit_morph_listener )
		{
			m_unit_discover_listener.reset();
			m_unit_morph_listener.reset();

			if( m_unit_type.getRace() == Races::Terran )
			{
				for( auto id : m_reserved_resources )
				{
					m_task->removeRequirement( id );
				}

				m_reserved_resources.clear();
			}
			else
			{
				m_task.reset();
			}
		}

		if( m_build_unit->isCompleted() )
		{
			m_has_finished = true;
			m_build_unit = nullptr;
		}
		else if( !m_build_unit->exists() )
		{
			m_build_unit = nullptr;
		}
	}

	if( m_has_finished )
	{
		m_task.reset();
		return;
	}

	if( m_task )
	{
		auto producer = m_task->getAssignedUnit();
		if( producer && !producer->exists() )
		{
			createTask();
		}

		if( m_unit_type.getRace() == Races::Terran && m_build_unit )
		{
			int remaining_construct_time = m_build_unit->getTimeTillCompleted();
			int remaining_unit_time = m_task->getRemainingUnitTime();
			if( remaining_construct_time != remaining_unit_time )
			{
				int time_change_required = remaining_construct_time - remaining_unit_time;
				m_task->requestUnitTimeChange( time_change_required );
			}
		}
	}
}

void SkynetControlTaskConstruct::postUpdate()
{
	if( m_has_finished || !m_task )
	{
		return;
	}

	auto producer = m_task->getAssignedUnit();
	if( producer )
	{
		if( !producer->isConstructing() )
		{
			if( m_build_unit )
			{
				producer->build( m_build_unit );
			}
			else
			{
				producer->build( m_unit_type, m_task->getBuildPosition() );

				if( m_unit_type.isRefinery() )
				{
					if( !m_unit_morph_listener )
					{
						m_unit_morph_listener = std::make_unique<MessageListenerFunction<UnitMorphRenegade>>( getAccess().getUnitTracker(), [this]( const UnitMorphRenegade & message )
						{
							if( !m_build_unit && message.unit->getType() == m_unit_type && message.unit->getTilePosition() == m_task->getBuildPosition() )
							{
								m_build_unit = message.unit;
							}
						} );
					}
				}
				else
				{
					if( !m_unit_discover_listener )
					{
						m_unit_discover_listener = std::make_unique<MessageListenerFunction<UnitDiscover>>( getAccess().getUnitTracker(), [this]( const UnitDiscover & message )
						{
							if( !m_build_unit && message.unit->getType() == m_unit_type && message.unit->getTilePosition() == m_task->getBuildPosition() )
							{
								m_build_unit = message.unit;
							}
						} );
					}
				}
			}
		}
	}
}

void SkynetControlTaskConstruct::createTask()
{
	m_task = getAccess().getTaskManager().createTask( "Building - " + m_unit_type.getName(), m_priority );

	if( !m_build_unit )
	{
		if( m_unit_type.mineralPrice() > 0 )
			m_reserved_resources.push_back( m_task->addRequirementMineral( m_unit_type.mineralPrice() ) );
		if( m_unit_type.gasPrice() > 0 )
			m_reserved_resources.push_back( m_task->addRequirementGas( m_unit_type.gasPrice() ) );
		if( m_unit_type.supplyRequired() > 0 )
			m_reserved_resources.push_back( m_task->addRequirementSupply( m_unit_type.supplyRequired() ) );
	}

	int required_duration = m_unit_type.getRace() == Races::Zerg ? max_time : m_unit_type.getRace() == Races::Protoss ? BWAPI::Broodwar->getLatencyFrames() : m_unit_type.buildTime();

	m_task->addRequirementUnit( m_unit_type.whatBuilds().first, required_duration, getAccess().getBuildLocationManager().createBuildLocation( m_unit_type, m_build_location_type ) );

	if( m_unit_type.supplyProvided() > 0 )
		m_task->addOutputSupply( m_unit_type.buildTime() + BWAPI::Broodwar->getLatencyFrames(), m_unit_type.supplyProvided() );

	m_task->addOutputUnit( m_unit_type.buildTime() + BWAPI::Broodwar->getLatencyFrames(), m_unit_type.whatBuilds().first );

	// TODO: Add required tech
}
