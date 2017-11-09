#pragma once

#include "CoreModule.h"

#include "Player.h"

class PlayerTrackerInterface : public CoreModule
{
public:
	PlayerTrackerInterface( Core & core ) : CoreModule( core, "PlayerTracker" ) {}

	virtual Player getLocalPlayer() const = 0;
	virtual Player getEnemyPlayer() const = 0;
	virtual Player getNeutralPlayer() const = 0;
	virtual Player getPlayer( int player_id ) const = 0;
	virtual Player getPlayer( BWAPI::Player player ) const = 0;

	virtual const std::vector<Player> & getAllPlayers() const = 0;
};