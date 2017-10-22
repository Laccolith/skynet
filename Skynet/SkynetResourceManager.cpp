#include "SkynetResourceManager.h"

#include "SkynetTaskRequirement.h"
#include "Types.h"

SkynetResourceManager::SkynetResourceManager( Core & core )
	: ResourceManagerInterface( core )
{
	core.registerUpdateProcess( 2.0f, [this]() { update(); } );
}

void SkynetResourceManager::update()
{
	m_task_reserved_minerals.clear();
	m_task_reserved_gas.clear();
}

void SkynetResourceManager::reserveTaskResource( int time, int amount, std::vector<ResourceTiming> & m_reserved_timings )
{
	auto it = std::upper_bound( m_reserved_timings.begin(), m_reserved_timings.end(), time, []( int time, auto & item ) { return item.time >= time; } );
	if( it != m_reserved_timings.end() && it->time == time )
		it->amount -= amount;
	else
		m_reserved_timings.insert( it, ResourceTiming{ time, amount } );
}

void SkynetResourceManager::reserveTaskMinerals( int time, int amount )
{
	reserveTaskResource( time, amount, m_task_reserved_minerals );
}

void SkynetResourceManager::reserveTaskGas( int time, int amount )
{
	reserveTaskResource( time, amount, m_task_reserved_gas );
}

void SkynetResourceManager::reserveTaskSupply( int time, int amount )
{
}

int SkynetResourceManager::earliestAvailability( int required_amount, double free_amount, double resource_rate, const std::vector<ResourceTiming> & m_reserved_timings ) const
{
	bool currently_available = false;
	if( free_amount >= required_amount )
	{
		free_amount -= required_amount;
		currently_available = true;
	}

	int earliest_time = 0;
	int last_time = 0;
	for( auto & time_point : m_reserved_timings )
	{
		double previous_free_amount = free_amount;

		free_amount -= time_point.amount;

		int time_passed = time_point.time - last_time;
		free_amount += time_passed * resource_rate;

		if( !currently_available )
		{
			if( free_amount >= required_amount )
			{
				if( resource_rate <= 0 )
					return requirement_max_time;

				free_amount -= required_amount;
				previous_free_amount -= required_amount;
				currently_available = true;
				earliest_time = last_time + int( std::ceil( -previous_free_amount / resource_rate ) );
			}
		}
		else
		{
			if( free_amount < 0 )
			{
				currently_available = false;
				free_amount += required_amount;
			}
		}

		last_time = time_point.time;
	}

	if( !currently_available )
	{
		if( resource_rate <= 0 )
			return requirement_max_time;

		free_amount -= required_amount;
		earliest_time = last_time + int(std::ceil( -free_amount / resource_rate ) );
	}

	return earliest_time;
}

int SkynetResourceManager::earliestMineralAvailability( int amount ) const
{
	return earliestAvailability( amount, BWAPI::Broodwar->self()->minerals(), m_mineral_rate, m_task_reserved_minerals );
}

int SkynetResourceManager::earliestGasAvailability( int amount ) const
{
	return earliestAvailability( amount, BWAPI::Broodwar->self()->gas(), m_gas_rate, m_task_reserved_gas );
}

int SkynetResourceManager::earliestSupplyAvailability( int amount ) const
{
	return 0;
}
