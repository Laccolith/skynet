#include "SkynetSquadManager.h"

SkynetSquadManager::SkynetSquadManager( Core & core )
	: SquadManagerInterface( core )
{
	core.registerUpdateProcess( 8.0f, [this]() { update(); } );
}

void SkynetSquadManager::update()
{
}
