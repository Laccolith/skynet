#pragma once

#include "PlayerTracker.h"

#include "SkynetPlayer.h"

class SkynetPlayerTracker : public PlayerTrackerInterface
{
public:
	SkynetPlayerTracker( Access & access );

	Player getLocalPlayer() const override { return m_local_player; }
	Player getNeutralPlayer() const override { return m_neutral_player; }
	Player getPlayer( BWAPI::Player player ) const override;

	const std::vector<Player> & getAllPlayers() const override { return m_all_players; }

private:
	std::vector<std::unique_ptr<SkynetPlayer>> m_bwapi_players;

	Player m_local_player = nullptr;
	Player m_neutral_player = nullptr;
	std::vector<Player> m_all_players;

	DEFINE_DEBUGGING_INTERFACE( Default );
};