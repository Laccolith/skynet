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
	std::vector<decltype(m_tasks)::value_type *> task_priorities;
	for( auto & task_priority : m_tasks )
	{
		task_priorities.push_back( &task_priority );
	}

	std::sort( task_priorities.begin(), task_priorities.end(), []( auto & first, auto & second )
	{
		return first->first->getPriority() > second->first->getPriority();
	} );

	for( auto & task_priority : task_priorities )
	{
		for( auto & task : task_priority->second )
		{
			task->updateTime();
		}
	}

	if( isDebugging( Debug::Default ) )
	{
		int y_pos = 25;
		for( auto & task_priority : task_priorities )
		{
			for( auto & task : task_priority->second )
			{
				task->drawInfo( y_pos );
			}
		}
	}

	for( auto & task_priority : m_tasks )
	{
		task_priority.second.erase( std::remove_if( task_priority.second.begin(), task_priority.second.end(), []( auto task )
		{
			return task->requirementsFulfilled();
		} ), task_priority.second.end() );
	}
}

std::unique_ptr<TaskInterface> SkynetTaskManager::createTask( std::string name, TaskPriority * priority )
{
	auto task = std::make_unique<SkynetTask>( *this, std::move( name ) );
	m_tasks[static_cast<SkynetTaskPriority *>(priority)].emplace_back( task.get() );
	return task;
}

void SkynetTaskManager::onTaskDestroyed( SkynetTask & task )
{
	for( auto & task_priority : m_tasks )
	{
		auto it = std::find( task_priority.second.begin(), task_priority.second.end(), &task );
		if( it != task_priority.second.end() )
		{
			task_priority.second.erase( it );
			break;
		}
	}
}

TaskPriority * SkynetTaskManager::createPriorityGroup( std::string name, double starting_priority )
{
	m_priorities.push_back( std::make_unique<SkynetTaskPriority>( std::move( name ), starting_priority ) );
	return m_priorities.back().get();
}
