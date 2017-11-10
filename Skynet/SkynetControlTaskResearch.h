#pragma once

#include "SkynetControlTask.h"

#include "Types.h"

#include <memory>

class TaskInterface;
class SkynetControlTaskResearch : public SkynetControlTask
{
public:
	SkynetControlTaskResearch( SkynetControlTaskFactory & skynet_control_task_factory, TechType tech_type );

	bool isInProgress() const override;
	bool isFinished() const override;

	void preUpdate() override;
	void postUpdate() override;

private:
	TechType m_tech_type;

	std::vector<int> m_reserved_resources;
	std::unique_ptr<TaskInterface> m_task;

	void createTask();
};