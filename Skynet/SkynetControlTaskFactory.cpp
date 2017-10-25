#include "SkynetControlTaskFactory.h"

#include "SkynetControlTask.h"
#include "SkynetControlTaskTrain.h"

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

std::unique_ptr<ControlTask> SkynetControlTaskFactory::createTrainControlTask( UnitType unit_type )
{
	auto control_task = std::make_unique<SkynetControlTaskTrain>( *this, unit_type );
	m_control_tasks.push_back( control_task.get() );
	return control_task;
}

void SkynetControlTaskFactory::onTaskDestroyed( SkynetControlTask * task )
{
	auto it = std::find( m_control_tasks.begin(), m_control_tasks.end(), task );
	std::swap( *it, m_control_tasks.back() );
	m_control_tasks.pop_back();
}
