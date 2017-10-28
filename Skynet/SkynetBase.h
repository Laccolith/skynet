#pragma once

#include "Base.h"

#include "CoreAccess.h"

class SkynetBaseTracker;
class SkynetBase : public BaseInterface, public CoreAccess
{
public:
	SkynetBase( CoreAccess & core_access, Position center_position, Region region, BaseLocation base_location );
	SkynetBase( CoreAccess & core_access, Region region );

	Position getCenterPosition() const override { return m_center_position; }
	TilePosition getBuildPosition() const override { return m_build_position; }
	BaseLocation getLocation() const override { return m_base_location; }
	Region getRegion() const override { return m_region; }

	const UnitGroup &getMinerals() const override { return m_minerals; }
	const UnitGroup &getGeysers() const override { return m_geysers; }
	const UnitGroup &getRefineries() const override { return m_refineries; }

	Unit getResourceDepot() const override { return m_resource_depot; }
	bool isActive( int frame_time = 0 ) const override { return m_active_time >= 0 && frame_time >= m_active_time; }

	bool isStartLocation() const override { return m_is_start_location; }

	const UnitGroup &getBuildings() const override { return m_buildings; }

	Player getPlayer() const override { return m_player; }

	bool isEnemyBase() const override;
	bool isMyBase() const override;
	bool isAllyBase() const override;
	bool isContested() const override { return m_is_contested; }

	void draw() const override;

	void update();

	void mark_as_start_location() { m_is_start_location = true; }

	void add_building( Unit building );
	void remove_building( Unit building );

private:
	Position m_center_position;
	TilePosition m_build_position;
	Region m_region;
	BaseLocation m_base_location = nullptr;

	UnitGroup m_minerals;
	UnitGroup m_geysers;
	UnitGroup m_refineries;

	Unit m_resource_depot = nullptr;

	int m_active_time = -1;

	bool m_is_contested = false;

	bool m_is_start_location = false;

	UnitGroup m_buildings;
	Player m_player = nullptr;
};