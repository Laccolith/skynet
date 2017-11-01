#include "SkynetControlTaskFactory.h"

#include "SkynetControlTask.h"
#include "SkynetControlTaskTrain.h"
#include "SkynetControlTaskConstruct.h"

SkynetControlTaskFactory::SkynetControlTaskFactory( Core & core )
	: ControlTaskFactoryInterface( core )
{
	core.registerUpdateProcess( 3.0f, [this]() { preUpdate(); } );
	core.registerUpdateProcess( 5.0f, [this]() { postUpdate(); } );
}

void SkynetControlTaskFactory::preUpdate()
{
	for( auto task : m_control_tasks )
	{
		task->preUpdate();
	}
}

void SkynetControlTaskFactory::postUpdate()
{
	for( auto task : m_control_tasks )
	{
		task->postUpdate();
	}
}

std::unique_ptr<ControlTask> SkynetControlTaskFactory::createBuildControlTask( UnitType unit_type, BuildLocationType build_location_type )
{
	std::unique_ptr<SkynetControlTask> control_task;

	if( unit_type.getRace() == BWAPI::Races::Zerg && unit_type.isBuilding() == unit_type.whatBuilds().first.isBuilding() )
	{
		// TODO: Morph
	}
	else if( unit_type.isAddon() )
	{
		// TODO: Addon
	}
	else if( unit_type.isBuilding() )
	{
		control_task = std::make_unique<SkynetControlTaskConstruct>( *this, unit_type, build_location_type );
	}
	else
	{
		control_task = std::make_unique<SkynetControlTaskTrain>( *this, unit_type );
	}

	if( control_task )
		m_control_tasks.push_back( control_task.get() );

	return control_task;
}

void SkynetControlTaskFactory::onTaskDestroyed( SkynetControlTask * task )
{
	auto it = std::find( m_control_tasks.begin(), m_control_tasks.end(), task );
	std::swap( *it, m_control_tasks.back() );
	m_control_tasks.pop_back();
}
