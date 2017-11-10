#pragma once

#include "BuildOrderManager.h"

#include "SkynetBuildOrder.h"

class SkynetBuildOrderManager : public BuildOrderManagerInterface
{
public:
	SkynetBuildOrderManager( Core & core );

	SkynetBuildOrder & createBuildOrder( std::string_view name );

	void update();

	TaskPriority * getTaskPriority() const { return m_build_order_priority; }
	int timeTillItemStarts( int id );
	bool isItemInProgress( int id );
	bool isItemComplete( int id );

	void changeBuild( SkynetBuildOrder & next_build );

private:
	TaskPriority * m_build_order_priority = nullptr;
	std::vector<std::unique_ptr<SkynetBuildOrder>> m_build_orders;

	SkynetBuildOrder * m_current_build_order = nullptr;
	std::vector<std::pair<std::function<std::unique_ptr<ControlTask>()>, Condition>> m_current_build_items;
	std::vector<std::unique_ptr<ControlTask>> m_current_build_item_tasks;
	std::vector<std::pair<std::function<void()>, Condition>> m_current_generic_items;

	DEFINE_DEBUGGING_INTERFACE( Default );
};