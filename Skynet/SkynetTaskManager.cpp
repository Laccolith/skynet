#include "SkynetTaskManager.h"

#include "SkynetTask.h"

SkynetTaskManager::SkynetTaskManager( Core & core )
	: TaskManagerInterface( core )
{
	core.registerUpdateProcess( 4.0f, [this]() { update(); } );
}

void SkynetTaskManager::update()
{
	for( auto & task : m_tasks )
	{
		task->updateTime();
	}

	m_tasks.erase( std::remove_if( m_tasks.begin(), m_tasks.end(), []( auto task ) -> bool
	{
		return task->requirementsFulfilled();
	} ), m_tasks.end() );
}

std::unique_ptr<TaskInterface> SkynetTaskManager::createTask()
{
	auto task = std::make_unique<SkynetTask>( *this );
	m_tasks.emplace_back( task.get() );
	return task;
}

void SkynetTaskManager::onTaskDestroyed( SkynetTask & task )
{
	m_tasks.erase( std::find( m_tasks.begin(), m_tasks.end(), &task ) );
}
