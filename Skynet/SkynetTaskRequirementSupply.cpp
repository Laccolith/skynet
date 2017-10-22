#include "SkynetTaskRequirementSupply.h"

#include "ResourceManager.h"

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
	access.getResourceManager().reserveTaskSupply( time, m_amount );
}