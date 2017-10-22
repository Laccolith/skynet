#include "SkynetTaskRequirementMineral.h"

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
	access.getResourceManager().reserveTaskMinerals( time, m_amount );
}