#pragma once

#include "SkynetInterface.h"

#include "Player.h"

class PlayerTrackerInterface : public SkynetInterface
{
public:
	PlayerTrackerInterface( Access &access ) : SkynetInterface( access, "PlayerTracker" ) {}

	virtual Player getLocalPlayer() const = 0;
	virtual Player getNeutralPlayer() const = 0;
	virtual Player getPlayer( BWAPI::Player player ) const = 0;

	virtual const std::vector<Player> & getAllPlayers() const = 0;
};