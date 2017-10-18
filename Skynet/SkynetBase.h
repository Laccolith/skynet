#pragma once

#include "Base.h"

class SkynetBaseTracker;
class SkynetBase : public BaseInterface
{
public:
	SkynetBase() {}

	Position getCenterPosition() const override { return m_center_position; }
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

	bool isEnemyBase() const override { return BWAPI::Broodwar->self()->isEnemy( m_player ); }
	bool isMyBase() const override { return m_player == BWAPI::Broodwar->self(); }
	bool isAllyBase() const override { return BWAPI::Broodwar->self()->isAlly( m_player ); }
	bool isContested() const override { return m_is_contested; }

	void draw() const override;

protected:
	friend SkynetBaseTracker;

	Position m_center_position = Positions::None;
	BaseLocation m_base_location = nullptr;
	Region m_region = nullptr;

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