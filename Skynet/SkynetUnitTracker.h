#pragma once

#include "UnitTracker.h"

#include "SkynetUnit.h"

#include <array>
#include <memory>

class SkynetUnitTracker : public UnitTrackerInterface
{
public:
	SkynetUnitTracker( Core & core );

	Unit getUnit( BWAPI::Unit unit ) const override;
	UnitGroup getUnitGroup( const BWAPI::Unitset &units ) const override;

	const UnitGroup & getGeysers() const override;
	const UnitGroup & getMinerals() const override;

	const UnitGroup & getAllUnits() const override { return m_all_units; }
	const UnitGroup & getAllUnits( UnitType type, Player player ) const override;
	const UnitGroup & getAllUnits( Player player ) const override;

	const UnitGroup & getSupplyUnits( Player player ) const override;

	UnitGroup getAllEnemyUnits( Player player ) const override;
	UnitGroup getAllEnemyUnits( UnitType type, Player player ) const override;

	void update();

private:
	std::vector<std::unique_ptr<SkynetUnit>> m_bwapi_units;

	std::vector<std::array<UnitGroup, (int) UnitTypes::Enum::Unknown>> m_player_to_type_to_units;
	std::vector<UnitGroup> m_player_to_units;
	std::vector<UnitGroup> m_player_to_supply_units;

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