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

	const std::vector<PlayerInterface*> & getAllies() const override { return m_allies; }
	const std::vector<PlayerInterface*> & getEnemies() const override { return m_enemies; }

	Race getRace() const override;

	Color getColor() const override;

	int getUpgradeLevel( UpgradeType upgrade ) const override;

	int weaponMaxRange( WeaponType weapon ) const override;

	BWAPI::Player getBWAPIPlayer() const override;

	void addAlly( PlayerInterface * ally ) { m_allies.push_back( ally ); }
	void addEnemy( PlayerInterface * enemy ) { m_enemies.push_back( enemy ); }

private:
	int m_id;
	BWAPI::Player m_player;

	std::vector<PlayerInterface*> m_allies;
	std::vector<PlayerInterface*> m_enemies;

	std::string m_name;
};