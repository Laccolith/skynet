#pragma once

#include "ControlTaskFactory.h"

#include <vector>

class SkynetControlTask;
class SkynetControlTaskFactory : public ControlTaskFactoryInterface
{
public:
	SkynetControlTaskFactory( Core & core );

	void preUpdate();
	void postUpdate();

	std::unique_ptr<ControlTask> createTrainControlTask( UnitType unit_type ) override;

	void onTaskDestroyed( SkynetControlTask * task );

private:
	std::vector<SkynetControlTask*> m_control_tasks;

	DEFINE_DEBUGGING_INTERFACE( Default );
};
