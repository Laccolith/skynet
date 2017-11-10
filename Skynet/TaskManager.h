#pragma once

#include "CoreModule.h"

#include "Task.h"

#include <memory>

class TaskManagerInterface : public CoreModule
{
public:
	TaskManagerInterface( Core & core ) : CoreModule( core, "TaskManager" ) {}

	virtual std::unique_ptr<TaskInterface> createTask( std::string name, TaskPriority * priority_group ) = 0;

	virtual TaskPriority * createPriorityGroup( std::string name, double starting_priority ) = 0;
};