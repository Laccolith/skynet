#pragma once

#include "SupplyManager.h"

#include <memory>

class TaskPriority;
class ControlTask;
class SkynetSupplyManager : public SupplyManagerInterface
{
public:
	SkynetSupplyManager( Core & core );

	void update();

	void setBuilding( bool enabled ) override { m_can_build = true; }

private:
	bool m_can_build = false;
	TaskPriority * m_build_priority = nullptr;
	std::unique_ptr<ControlTask> m_build_task;
};