#pragma once

#include "BaseManager.h"
#include "BaseTracker.h"
#include "UnitTracker.h"
#include "TaskManager.h"

#include <vector>

class ControlTask;
class SkynetBaseManager : public BaseManagerInterface, public MessageListener<BasesRecreated>, public MessageListener<UnitDestroy>
{
public:
	SkynetBaseManager( Core & core );

	void notify( const BasesRecreated & message ) override;
	void notify( const UnitDestroy & message ) override;

	void preUpdate();
	void postUpdate();

	void setWorkerTraining( bool enabled ) override { m_can_train_workers = true; }

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

	TaskPriority * m_transfer_worker_priority = nullptr;
	TaskPriority * m_train_worker_priority = nullptr;

	bool m_can_train_workers = false;
	std::vector<std::unique_ptr<ControlTask>> m_worker_train_items;

	DEFINE_DEBUGGING_INTERFACE( Default );
};
