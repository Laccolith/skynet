#include "SkynetTaskRequirementResource.h"

#include "ResourceManager.h"

SkynetTaskRequirementMineral::SkynetTaskRequirementMineral( int amount )
	: m_amount( amount )
{
}

int SkynetTaskRequirementMineral::getEarliestTime( CoreAccess & access )
{
	return access.getResourceManager().earliestMineralAvailability( m_amount );
}

void SkynetTaskRequirementMineral::reserveTime( CoreAccess & access, int time )
{
	m_reserved = time <= 0;
	access.getResourceManager().reserveTaskMinerals( time, m_amount );
}

void SkynetTaskRequirementMineral::freeReserved( CoreAccess & access )
{
	if( m_reserved )
		access.getResourceManager().freeTaskMinerals( m_amount );
}

SkynetTaskRequirementGas::SkynetTaskRequirementGas( int amount )
	: m_amount( amount )
{
}

int SkynetTaskRequirementGas::getEarliestTime( CoreAccess & access )
{
	return access.getResourceManager().earliestGasAvailability( m_amount );
}

void SkynetTaskRequirementGas::reserveTime( CoreAccess & access, int time )
{
	m_reserved = time <= 0;
	access.getResourceManager().reserveTaskGas( time, m_amount );
}

void SkynetTaskRequirementGas::freeReserved( CoreAccess & access )
{
	if( m_reserved )
		access.getResourceManager().freeTaskGas( m_amount );
}

SkynetTaskRequirementSupply::SkynetTaskRequirementSupply( int amount )
	: m_amount( amount )
{
}

int SkynetTaskRequirementSupply::getEarliestTime( CoreAccess & access )
{
	return access.getResourceManager().earliestSupplyAvailability( m_amount );
}

void SkynetTaskRequirementSupply::reserveTime( CoreAccess & access, int time )
{
	m_reserved = time <= 0;
	access.getResourceManager().reserveTaskSupply( time, m_amount );
}

void SkynetTaskRequirementSupply::freeReserved( CoreAccess & access )
{
	if( m_reserved )
		access.getResourceManager().freeTaskSupply( m_amount );
}