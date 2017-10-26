#pragma once

#include "ResourceManager.h"

#include <vector>

class SkynetResourceManager : public ResourceManagerInterface
{
public:
	SkynetResourceManager( Core & core );

	void update();

	void reserveTaskMinerals( int time, int amount ) override;
	void reserveTaskGas( int time, int amount ) override;
	void reserveTaskSupply( int time, int amount ) override;

	void freeTaskMinerals( int amount ) override;
	void freeTaskGas( int amount ) override;
	void freeTaskSupply( int amount ) override;

	int earliestMineralAvailability( int amount ) const override;
	int earliestGasAvailability( int amount ) const override;
	int earliestSupplyAvailability( int amount ) const override;

private:
	struct ResourceTiming
	{
		int time;
		int amount;
	};
	std::vector<ResourceTiming> m_task_reserved_minerals;
	std::vector<ResourceTiming> m_task_reserved_gas;
	std::vector<ResourceTiming> m_task_reserved_supply;

	int m_reserved_minerals = 0;
	int m_reserved_gas = 0;
	int m_reserved_supply = 0;

	double m_mineral_rate = 0.0;
	double m_gas_rate = 0.0;

	void reserveTaskResource( int time, int amount, std::vector<ResourceTiming> & m_reserved_timings );
	int SkynetResourceManager::earliestAvailability( int required_amount, double free_amount, double resource_rate, const std::vector<ResourceTiming> & m_reserved_timings ) const;

	DEFINE_DEBUGGING_INTERFACE( Default );
};