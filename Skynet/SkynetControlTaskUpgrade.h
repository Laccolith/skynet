#pragma once

#include "SkynetControlTask.h"

#include "Types.h"

#include <memory>

class TaskInterface;
class SkynetControlTaskUpgrade : public SkynetControlTask
{
public:
	SkynetControlTaskUpgrade( SkynetControlTaskFactory & skynet_control_task_factory, TaskPriority * priority, UpgradeType upgrade_type, int upgrade_level );

	int timeTillStart() const override;
	bool isInProgress() const override;
	bool isComplete() const override;

	void preUpdate() override;
	void postUpdate() override;

private:
	TaskPriority * m_priority;
	UpgradeType m_upgrade_type;
	int m_upgrade_level;

	std::vector<int> m_reserved_resources;
	std::unique_ptr<TaskInterface> m_task;

	void createTask();
};