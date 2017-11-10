#pragma once

#include "SkynetControlTask.h"

#include "Types.h"

#include <memory>

class TaskInterface;
class SkynetControlTaskResearch : public SkynetControlTask
{
public:
	SkynetControlTaskResearch( SkynetControlTaskFactory & skynet_control_task_factory, TaskPriority * priority, TechType tech_type );

	int timeTillStart() const override;
	bool isInProgress() const override;
	bool isComplete() const override;

	void preUpdate() override;
	void postUpdate() override;

private:
	TaskPriority * m_priority;
	TechType m_tech_type;

	std::vector<int> m_reserved_resources;
	std::unique_ptr<TaskInterface> m_task;

	void createTask();
};