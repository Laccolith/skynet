#include "SkynetTaskRequirementGas.h"

#include "ResourceManager.h"

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
	access.getResourceManager().reserveTaskGas( time, m_amount );
}