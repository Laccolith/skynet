#include "SkynetPlayer.h"

SkynetPlayer::SkynetPlayer( int id, BWAPI::Player player )
	: m_id( id )
	, m_player( player )
	, m_name( player->getName() )
{
}

bool SkynetPlayer::isLocalPlayer() const
{
	return m_player == BWAPI::Broodwar->self();
}

const std::string & SkynetPlayer::getName() const
{
	return m_name;
}

bool SkynetPlayer::isAlly( Player player ) const
{
	return m_player->isAlly( player->getBWAPIPlayer() );
}

bool SkynetPlayer::isEnemy( Player player ) const
{
	return m_player->isEnemy( player->getBWAPIPlayer() );
}

bool SkynetPlayer::isNeutral() const
{
	return m_player->isNeutral();
}

Color SkynetPlayer::getColor() const
{
	return m_player->getColor();
}

int SkynetPlayer::getUpgradeLevel( UpgradeType upgrade ) const
{
	return m_player->getUpgradeLevel( upgrade );
}

int SkynetPlayer::weaponMaxRange( WeaponType weapon ) const
{
	return m_player->weaponMaxRange( weapon );
}

BWAPI::Player SkynetPlayer::getBWAPIPlayer() const
{
	return m_player;
}
