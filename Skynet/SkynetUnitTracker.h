#pragma once

#include "UnitTrackerInterface.h"

class SkynetUnitTracker : public UnitTrackerInterface
{
public:
	SkynetUnitTracker( Access & access );

	virtual Unit getUnit( BWAPI::Unit unit ) const;
	virtual UnitGroup getUnitGroup( const BWAPI::Unitset &units ) const;

	virtual const UnitGroup &getAllUnits() const { return m_all_units; }
	virtual const UnitGroup &getAllUnits( UnitType type, Player player = BWAPI::Broodwar->self() ) const;
	virtual const UnitGroup &getAllUnits( Player player = BWAPI::Broodwar->self() ) const;

	virtual UnitGroup getAllEnemyUnits( Player player = BWAPI::Broodwar->self() ) const;
	virtual UnitGroup getAllEnemyUnits( UnitType type, Player player = BWAPI::Broodwar->self() ) const;

	void update();

private:
	std::unordered_map<BWAPI::Unit, Unit> m_units;

	std::unordered_map<Unit, Player> m_unit_to_player;
	std::unordered_map<Unit, UnitType> m_unit_to_type;

	std::unordered_map<Player, std::map<UnitType, UnitGroup>> m_player_to_type_to_units;
	std::unordered_map<Player, UnitGroup> m_player_to_units;

	UnitGroup m_all_units;
};