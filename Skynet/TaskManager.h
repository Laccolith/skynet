#pragma once

#include "CoreModule.h"

#include "Task.h"

#include <memory>

class TaskManagerInterface : public CoreModule
{
public:
	TaskManagerInterface( Core & core ) : CoreModule( core, "TaskManager" ) {}

	virtual std::unique_ptr<TaskInterface> createTask() = 0;
};