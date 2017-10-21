#pragma once

#include "CoreModule.h"

#include "Unit.h"
#include "UnitGroup.h"
#include "Messaging.h"

struct UnitDiscover
{
	Unit unit;
};

struct UnitMorphRenegade
{
	Unit unit;
	Player last_player;
	UnitType last_type;

	bool isRenegade() const { return last_player != nullptr; }
	bool isMorph() const { return last_type != UnitTypes::None; }
};

struct UnitDestroy
{
	Unit unit;
};

class UnitTrackerInterface : public CoreModule, public MessageReporter<UnitDiscover, UnitMorphRenegade, UnitDestroy>
{
public:
	UnitTrackerInterface( Core & core ) : CoreModule( core, "UnitTracker" ) {}

	virtual Unit getUnit( BWAPI::Unit unit ) const = 0;
	virtual UnitGroup getUnitGroup( const BWAPI::Unitset &units ) const = 0;

	virtual const UnitGroup & getGeysers() const = 0;
	virtual const UnitGroup & getMinerals() const = 0;

	virtual const UnitGroup & getAllUnits() const = 0;
	virtual const UnitGroup & getAllUnits( UnitType type, Player player ) const = 0;
	virtual const UnitGroup & getAllUnits( Player player ) const = 0;

	virtual UnitGroup getAllEnemyUnits( Player player ) const = 0;
	virtual UnitGroup getAllEnemyUnits( UnitType type, Player player ) const = 0;
};