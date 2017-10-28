#include "SkynetPlayerTracker.h"

SkynetPlayerTracker::SkynetPlayerTracker( Core & core )
	: PlayerTrackerInterface( core )
{
	m_bwapi_players.resize( BWAPI::Broodwar->getPlayers().size() );

	for( auto player : BWAPI::Broodwar->getPlayers() )
	{
		int player_id = player->getID();
		if( player_id == -1 )
		{
			player_id = m_bwapi_players.size() - 1;
		}

		if( m_bwapi_players[player_id] )
		{
			//TODO: this is an error
			break;
		}

		auto & stored_player = m_bwapi_players[player_id];

		stored_player = std::make_unique<SkynetPlayer>( player_id, player );

		m_all_players.emplace_back( stored_player.get() );

		if( player == BWAPI::Broodwar->self() )
		{
			m_local_player = stored_player.get();
		}
		else if( player == BWAPI::Broodwar->neutral() )
		{
			m_neutral_player = stored_player.get();
		}
	}
}

Player SkynetPlayerTracker::getPlayer( int player_id ) const
{
	return m_bwapi_players[player_id].get();
}

Player SkynetPlayerTracker::getPlayer( BWAPI::Player player ) const
{
	int player_id = player->getID();
	if( player_id == -1 )
		return m_bwapi_players.back().get();

	return m_bwapi_players[player_id].get();
}