#pragma once

#include "BaseManager.h"
#include "BaseTracker.h"
#include "UnitTracker.h"

#include <vector>

class SkynetBaseManager : public BaseManagerInterface, public MessageListener<BasesRecreated>, public MessageListener<UnitDestroy>
{
public:
	SkynetBaseManager( Core & core );

	void notify( const BasesRecreated & message ) override;
	void notify( const UnitDestroy & message ) override;

	void update();

private:
	struct BaseData
	{
		UnitGroup sorted_minerals;
		UnitGroup available_workers;
		std::map<Unit, Unit> worker_to_mineral;
		std::map<Unit, UnitGroup> mineral_to_workers;
	};
	std::map<Base, BaseData> m_base_data;

	DEFINE_DEBUGGING_INTERFACE( Default );
};
