#include "SkynetTaskManager.h"

#include "SkynetTask.h"

SkynetTaskManager::SkynetTaskManager( Core & core )
	: TaskManagerInterface( core )
{
	core.registerUpdateProcess( 4.0f, [this]() { update(); } );

	setDebugging( Debug::Default, true );
}

void SkynetTaskManager::update()
{
	for( auto & task : m_tasks )
	{
		task->updateTime();
	}

	if( isDebugging( Debug::Default ) )
	{
		int y_pos = 25;
		for( auto & task : m_tasks )
		{
			task->drawInfo( y_pos );
		}
	}

	m_tasks.erase( std::remove_if( m_tasks.begin(), m_tasks.end(), []( auto task ) -> bool
	{
		return task->requirementsFulfilled();
	} ), m_tasks.end() );
}

std::unique_ptr<TaskInterface> SkynetTaskManager::createTask( std::string name )
{
	auto task = std::make_unique<SkynetTask>( *this, std::move( name ) );
	m_tasks.emplace_back( task.get() );
	return task;
}

void SkynetTaskManager::onTaskDestroyed( SkynetTask & task )
{
	m_tasks.erase( std::find( m_tasks.begin(), m_tasks.end(), &task ) );
}
