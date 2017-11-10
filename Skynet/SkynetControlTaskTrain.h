#pragma once

#include "SkynetControlTask.h"

#include "Types.h"

#include <memory>

class TaskInterface;
class SkynetControlTaskTrain : public SkynetControlTask
{
public:
	SkynetControlTaskTrain( SkynetControlTaskFactory & skynet_control_task_factory, UnitType unit_type );

	int timeTillStart() const override;
	bool isInProgress() const override;
	bool isComplete() const override;

	void preUpdate() override;
	void postUpdate() override;

private:
	UnitType m_unit_type;
	Unit m_build_unit = nullptr;

	std::vector<int> m_reserved_resources;
	std::unique_ptr<TaskInterface> m_task;

	void createTask();
};
