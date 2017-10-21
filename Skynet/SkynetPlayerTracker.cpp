#include "SkynetPlayerTracker.h"

#include "Skynet.h"

SkynetPlayerTracker::SkynetPlayerTracker( Access & access )
	: PlayerTrackerInterface( access )
{
	bool had_neutral_player = false;
	for( auto player : BWAPI::Broodwar->getPlayers() )
	{
		if( had_neutral_player )
		{
			//TODO: this is an error
			break;
		}

		int player_id = player->getID();
		if( player_id == -1 )
		{
			had_neutral_player = true;
			player_id = BWAPI::Broodwar->getPlayers().size() - 1;
		}

		m_bwapi_players.emplace_back( std::make_unique<SkynetPlayer>( m_bwapi_players.size(), player ) );

		m_all_players.emplace_back( m_bwapi_players.back().get() );

		if( player == BWAPI::Broodwar->self() )
		{
			m_local_player = m_bwapi_players.back().get();
		}
		else if( player == BWAPI::Broodwar->neutral() )
		{
			m_neutral_player = m_bwapi_players.back().get();
		}
	}
}

Player SkynetPlayerTracker::getPlayer( BWAPI::Player player ) const
{
	int player_id = player->getID();
	if( player_id == -1 )
		return m_bwapi_players.back().get();

	return m_bwapi_players[player_id].get();
}