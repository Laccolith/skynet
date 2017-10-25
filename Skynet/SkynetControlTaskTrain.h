#pragma once

#include "SkynetControlTask.h"

#include "Types.h"

#include <memory>

class TaskInterface;
class SkynetControlTaskTrain : public SkynetControlTask
{
public:
	SkynetControlTaskTrain( SkynetControlTaskFactory & skynet_control_task_factory, UnitType unit_type );

	bool isInProgress() const override;
	bool isFinished() const override;

	void preUpdate() override;
	void postUpdate() override;

private:
	UnitType m_unit_type;
	bool m_has_started = false;
	bool m_will_complete_next_frame = false;

	std::vector<int> m_reserved_resources;
	std::unique_ptr<TaskInterface> m_task;

	void createTask();
};
