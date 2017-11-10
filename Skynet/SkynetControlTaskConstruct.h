#pragma once

#include "SkynetControlTask.h"

#include "Types.h"
#include "Messaging.h"
#include "UnitTracker.h"
#include "BuildLocation.h"

#include <memory>

class TaskInterface;
class SkynetControlTaskConstruct : public SkynetControlTask
{
public:
	SkynetControlTaskConstruct( SkynetControlTaskFactory & skynet_control_task_factory, TaskPriority * priority, UnitType unit_type, BuildLocationType build_location_type );

	int timeTillStart() const override;
	bool isInProgress() const override;
	bool isComplete() const override;

	void preUpdate() override;
	void postUpdate() override;

private:
	TaskPriority * m_priority;
	UnitType m_unit_type;
	BuildLocationType m_build_location_type;
	bool m_has_finished = false;
	Unit m_build_unit = nullptr;

	std::vector<int> m_reserved_resources;
	std::unique_ptr<TaskInterface> m_task;

	std::unique_ptr<MessageListenerFunction<UnitDiscover>> m_unit_discover_listener;
	std::unique_ptr<MessageListenerFunction<UnitMorphRenegade>> m_unit_morph_listener;

	void createTask();
};
