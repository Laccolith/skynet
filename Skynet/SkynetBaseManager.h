#pragma once

#include "BaseManager.h"
#include "BaseTracker.h"
#include "UnitTracker.h"
#include "TaskManager.h"

#include <vector>

class SkynetBaseManager : public BaseManagerInterface, public MessageListener<BasesRecreated>, public MessageListener<UnitDestroy>
{
public:
	SkynetBaseManager( Core & core );

	void notify( const BasesRecreated & message ) override;
	void notify( const UnitDestroy & message ) override;

	void preUpdate();
	void postUpdate();

private:
	struct BaseData
	{
		UnitGroup sorted_minerals;
		UnitGroup available_workers;
		std::map<Unit, Unit> worker_to_resource;
		std::map<Unit, UnitGroup> resource_to_workers;
	};
	std::map<Base, BaseData> m_base_data;

	std::map<std::pair<Base, Base>, std::vector<std::unique_ptr<TaskInterface>>> m_worker_transfers;

	DEFINE_DEBUGGING_INTERFACE( Default );
};
