#include "SkynetControlTask.h"

#include "SkynetControlTaskFactory.h"

SkynetControlTask::SkynetControlTask( SkynetControlTaskFactory & skynet_control_task_factory )
	: m_skynet_control_task_factory( skynet_control_task_factory )
{
}

SkynetControlTask::~SkynetControlTask()
{
	m_skynet_control_task_factory.onTaskDestroyed( this );
}

CoreAccess & SkynetControlTask::getAccess()
{
	return m_skynet_control_task_factory;
}

const CoreAccess & SkynetControlTask::getAccess() const
{
	return m_skynet_control_task_factory;
}
