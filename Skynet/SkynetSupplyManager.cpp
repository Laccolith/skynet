#include "SkynetSupplyManager.h"

#include "ControlTask.h"
#include "PlayerTracker.h"
#include "ResourceManager.h"
#include "TaskManager.h"
#include "ControlTaskFactory.h"

SkynetSupplyManager::SkynetSupplyManager( Core & core )
	: SupplyManagerInterface( core )
{
	core.registerUpdateProcess( 5.0f, [this]() { update(); } );

	m_build_priority = getTaskManager().createPriorityGroup( "Build Supply", 90.0 );
}

void SkynetSupplyManager::update()
{
	if( !m_can_build )
		return;

	int supply_check_time = getPlayerTracker().getLocalPlayer()->getRace().getSupplyProvider().buildTime() + 150;

	if( getResourceManager().totalSupplyAtTime( supply_check_time ) < 400 )
	{
		int current_supply = BWAPI::Broodwar->self()->supplyUsed();
		int available_supply = getResourceManager().availableSupplyAtTime( supply_check_time );

		if( available_supply <= (current_supply > 120 ? 16 : current_supply > 60 ? 8 : 0) )
		{
			if( !m_build_task || m_build_task->isInProgress() )
				m_build_task = getControlTaskFactory().createBuildControlTask( m_build_priority, getPlayerTracker().getLocalPlayer()->getRace().getSupplyProvider() );
		}
	}
}
