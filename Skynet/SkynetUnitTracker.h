#pragma once

#include "UnitTracker.h"

#include "SkynetUnit.h"

#include <array>

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
	// Are bwapi unit ids good to use as a index to a vector, rather than using this map?
	std::vector<std::unique_ptr<SkynetUnit>> m_bwapi_units;

	std::vector<std::array<UnitGroup, (int)UnitTypes::Enum::Unknown>> m_player_to_type_to_units;
	std::vector<UnitGroup> m_player_to_units;

	UnitGroup m_all_units;

	std::vector<std::unique_ptr<SkynetUnit>> m_dead_units;

	std::vector<int> m_free_ids;
	int m_current_id_counter;

	void onUnitDiscover( BWAPI::Unit unit );
	void onUnitDestroy( BWAPI::Unit unit );

	void onDiscover( Unit unit );
	void onMorphRenegade( Unit unit, Player last_player, UnitType last_type );
	void onDestroy( std::unique_ptr<SkynetUnit> & unit );

	void updateUnit( SkynetUnit * unit );

	DEFINE_DEBUGGING_INTERFACE( Default );
};