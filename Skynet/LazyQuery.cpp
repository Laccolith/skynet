#include "LazyQuery.h"

#include "PlayerTracker.h"

Value<Race> LazyQuery::enemyRace()
{
	return Value<Race>( []( CoreAccess & access ) -> bool
	{
		return access.getPlayerTracker().getEnemyPlayer()->getRace();
	} );
}

Value<int> LazyQuery::numEnemies()
{
	return Value<int>( []( CoreAccess & access ) -> int
	{
		return access.getPlayerTracker().getLocalPlayer()->getEnemies().size();
	} );
}