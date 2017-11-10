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

	std::unique_ptr<ControlTask> createBuildControlTask( UnitType unit_type, BuildLocationType build_location_type = BuildLocationType::Base ) override;
	std::unique_ptr<ControlTask> createResearchControlTask( TechType tech_type ) override;
	std::unique_ptr<ControlTask> createUpgradeControlTask( UpgradeType upgrade_type, int upgrade_level ) override;

	void onTaskDestroyed( SkynetControlTask * task );

private:
	std::vector<SkynetControlTask*> m_control_tasks;

	DEFINE_DEBUGGING_INTERFACE( Default );
};
