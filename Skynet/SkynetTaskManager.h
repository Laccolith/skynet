#pragma once

#include "TaskManager.h"
#include "SkynetTask.h"

#include <vector>
#include <memory>

class SkynetTaskPriority : public TaskPriority
{
public:
	SkynetTaskPriority( std::string name, double starting_priority )
		: m_name( std::move( name ) )
		, m_priority( starting_priority )
	{
	}

	~SkynetTaskPriority()
	{
	}

	void changePriority( double priority ) override { m_priority = priority; }
	double getPriority() const { return m_priority; }

private:
	std::string m_name;
	double m_priority;
};

class SkynetTaskManager : public TaskManagerInterface
{
public:
	SkynetTaskManager( Core & core );

	void update();

	std::unique_ptr<TaskInterface> createTask( std::string name, TaskPriority * priority ) override;
	void onTaskDestroyed( SkynetTask & task );

	TaskPriority * createPriorityGroup( std::string name, double starting_priority ) override;

private:
	std::vector<std::unique_ptr<SkynetTaskPriority>> m_priorities;
	std::map<SkynetTaskPriority *, std::vector<SkynetTask *>> m_tasks;

	DEFINE_DEBUGGING_INTERFACE( Default );
};