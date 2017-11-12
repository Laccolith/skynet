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

	int availableMineralsAtTime( int time ) const override;
	int availableGasAtTime( int time ) const override;
	int availableSupplyAtTime( int time ) const override;

	int totalSupplyAtTime( int time ) const override;

	void addTaskSupplyOutput( int time, int amount, bool temporary ) override;
	void removeTaskSupplyOutput( int time, int amount ) override;

	void setMineralRate( double mineral_rate ) override { m_mineral_rate = mineral_rate; }
	void setGasRate( double gas_rate ) override { m_gas_rate = gas_rate; }

private:
	struct ResourceTiming
	{
		int time;
		int amount;
	};
	std::vector<ResourceTiming> m_task_reserved_minerals;
	std::vector<ResourceTiming> m_task_reserved_gas;
	std::vector<ResourceTiming> m_task_reserved_supply;

	std::vector<ResourceTiming> m_task_output_supply;

	int m_reserved_minerals = 0;
	int m_reserved_gas = 0;
	int m_reserved_supply = 0;

	double m_mineral_rate = 0.0;
	double m_gas_rate = 0.0;

	void reserveTaskResource( int time, int amount, std::vector<ResourceTiming> & m_reserved_timings );
	int availabilityAtTime( int time, double free_amount, double resource_rate, const std::vector<ResourceTiming> & m_reserved_timings ) const;
	int earliestAvailability( int required_amount, double free_amount, double resource_rate, int max_resource, const std::vector<ResourceTiming> & m_reserved_timings ) const;

	DEFINE_DEBUGGING_INTERFACE( Default );
};