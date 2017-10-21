#pragma once

#include "Types.h"

class PlayerInterface
{
public:
	virtual int getID() const = 0;

	virtual bool isLocalPlayer() const = 0;

	virtual const std::string & getName() const = 0;

	virtual bool isAlly( Player player ) const = 0;
	virtual bool isEnemy( Player player ) const = 0;
	virtual bool isNeutral() const = 0;

	virtual Color getColor() const = 0;

	virtual int getUpgradeLevel( UpgradeType upgrade ) const = 0;

	virtual int weaponMaxRange( WeaponType weapon ) const = 0;

	virtual BWAPI::Player getBWAPIPlayer() const = 0;

	virtual ~PlayerInterface() {}
};