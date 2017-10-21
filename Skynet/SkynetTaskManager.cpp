#include "SkynetTaskManager.h"

#include "SkynetTask.h"

SkynetTaskManager::SkynetTaskManager( Core & core )
	: TaskManagerInterface( core )
{
	core.registerUpdateProcess( 2.0f, [this]() { update(); } );
}

void SkynetTaskManager::update()
{
	int current_time = BWAPI::Broodwar->getFrameCount();

	for( auto & task : m_tasks )
	{
		task->updateTime();
	}

	m_tasks.erase( std::remove_if( m_tasks.begin(), m_tasks.end(), [current_time]( auto task ) -> bool
	{
		return task->getPlannedTime() <= current_time;
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
