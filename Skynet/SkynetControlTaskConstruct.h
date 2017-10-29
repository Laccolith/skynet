#pragma once

#include "SkynetControlTask.h"

#include "Types.h"
#include "Messaging.h"
#include "UnitTracker.h"

#include <memory>

class TaskInterface;
class SkynetControlTaskConstruct : public SkynetControlTask
{
public:
	SkynetControlTaskConstruct( SkynetControlTaskFactory & skynet_control_task_factory, UnitType unit_type );

	bool isInProgress() const override;
	bool isFinished() const override;

	void preUpdate() override;
	void postUpdate() override;

private:
	UnitType m_unit_type;
	bool m_has_finished = false;
	Unit m_build_unit = nullptr;

	std::vector<int> m_reserved_resources;
	std::unique_ptr<TaskInterface> m_task;

	std::unique_ptr<MessageListenerFunction<UnitDiscover>> m_unit_discover_listener;

	void createTask();
};