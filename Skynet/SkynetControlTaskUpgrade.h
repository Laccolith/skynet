#pragma once

#include "SkynetControlTask.h"

#include "Types.h"

#include <memory>

class TaskInterface;
class SkynetControlTaskUpgrade : public SkynetControlTask
{
public:
	SkynetControlTaskUpgrade( SkynetControlTaskFactory & skynet_control_task_factory, UpgradeType upgrade_type, int upgrade_level );

	bool isInProgress() const override;
	bool isFinished() const override;

	void preUpdate() override;
	void postUpdate() override;

private:
	UpgradeType m_upgrade_type;
	int m_upgrade_level;

	std::vector<int> m_reserved_resources;
	std::unique_ptr<TaskInterface> m_task;

	void createTask();
};