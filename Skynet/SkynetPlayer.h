#pragma once

#include "Player.h"

class SkynetPlayer : public PlayerInterface
{
public:
	SkynetPlayer( int id, BWAPI::Player player );

	int getID() const override { return m_id; }

	bool isLocalPlayer() const override;

	const std::string & getName() const override;

	bool isAlly( Player player ) const override;
	bool isEnemy( Player player ) const override;
	bool isNeutral() const override;

	Color getColor() const override;

	int getUpgradeLevel( UpgradeType upgrade ) const override;

	int weaponMaxRange( WeaponType weapon ) const override;

	BWAPI::Player getBWAPIPlayer() const override;

private:
	int m_id;
	BWAPI::Player m_player;

	std::string m_name;
};