#pragma once

#include "SkynetInterface.h"

#include "Unit.h"
#include "UnitGroup.h"

class UnitTrackerInterface : public SkynetInterface
{
public:
	UnitTrackerInterface( Access &access ) : SkynetInterface( access, "UnitTracker" ) {}

	virtual Unit getUnit( BWAPI::Unit unit ) const = 0;
	virtual UnitGroup getUnitGroup( const BWAPI::Unitset &units ) const = 0;

	const UnitGroup &getGeysers() const { return getAllUnits( BWAPI::UnitTypes::Resource_Vespene_Geyser, BWAPI::Broodwar->neutral() ); }
	const UnitGroup &getMinerals() const { return getAllUnits( BWAPI::UnitTypes::Resource_Mineral_Field, BWAPI::Broodwar->neutral() ); }

	virtual const UnitGroup &getAllUnits() const = 0;
	virtual const UnitGroup &getAllUnits( UnitType type, Player player = BWAPI::Broodwar->self() ) const = 0;
	virtual const UnitGroup &getAllUnits( Player player = BWAPI::Broodwar->self() ) const = 0;

	virtual UnitGroup getAllEnemyUnits( Player player = BWAPI::Broodwar->self() ) const = 0;
	virtual UnitGroup getAllEnemyUnits( UnitType type, Player player = BWAPI::Broodwar->self() ) const = 0;
};