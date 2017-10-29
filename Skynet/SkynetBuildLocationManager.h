#pragma once

#include "BuildLocationManager.h"
#include "BaseTracker.h"
#include "UnitTracker.h"
#include "Messaging.h"
#include "RectangleArray.h"

class SkynetBuildLocation;
class SkynetBuildLocationManager : public BuildLocationManagerInterface, public MessageListener<BasesRecreated>, public MessageListener<UnitDestroy>
{
public:
	SkynetBuildLocationManager( Core & core );

	void notify( const BasesRecreated & message );
	void notify( const UnitDestroy & message );

	void preUpdate();
	void postUpdate();

	std::pair<int, TilePosition> choosePosition( int time, UnitType unit_type ) const;
	void reservePosition( int time, TilePosition position , UnitType unit_type );
	void freeReservation( TilePosition position, UnitType unit_type );

	std::unique_ptr<BuildLocation> createBuildLocation( UnitType unit_type ) override;
	void onBuildLocationDestroyed( SkynetBuildLocation * build_location );

private:
	struct ReservedPosition
	{
		TilePosition position;
		UnitType unit_type;
	};
	std::vector<ReservedPosition> m_reserved_positions;

	std::map<Base, std::vector<TilePosition>> m_base_to_build_positions;
	std::vector<Base> m_base_order_normal;

	struct TileInfo
	{
		bool is_permanently_reserved = false;
		int m_resouce_reserved = 0;
		bool is_build_planned = false;
	};
	RectangleArray<TileInfo, TILEPOSITION_SCALE> m_tile_info;

	std::map<Unit, std::vector<TilePosition>> m_resources;

	struct PowerTimes
	{
		int small_building_time = max_time;
		int medium_building_time = max_time;
		int large_building_time = max_time;
	};
	RectangleArray<PowerTimes, TILEPOSITION_SCALE> m_build_power_times;
	int m_earliest_power_time = max_time;

	VectorUnique<SkynetBuildLocation*> m_build_locations;

	void addToPowerTime( TilePosition tile_position, int time );
};
