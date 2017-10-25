#pragma once

#include "BaseManager.h"
#include "BaseTracker.h"

#include <vector>

class SkynetBaseManager : public BaseManagerInterface, public MessageListener<BasesRecreated>
{
public:
	SkynetBaseManager( Core & core );

	void notify( const BasesRecreated & message ) override;

	void update();

private:
	struct BaseData
	{
	};
	std::map<Base, BaseData> m_base_data;
};
