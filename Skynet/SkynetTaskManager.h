#pragma once

#include "TaskManager.h"
#include "SkynetTask.h"

#include <vector>
#include <memory>

class SkynetTaskManager : public TaskManagerInterface
{
public:
	SkynetTaskManager( Core & core );

	void update();

	std::unique_ptr<TaskInterface> createTask() override;
	void onTaskDestroyed( SkynetTask & task );

private:
	std::vector<SkynetTask *> m_tasks;

	DEFINE_DEBUGGING_INTERFACE( Default );
};