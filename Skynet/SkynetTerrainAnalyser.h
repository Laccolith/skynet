#pragma once

#include "TerrainAnalyser.h"

#include "SkynetRegion.h"
#include "SkynetChokepoint.h"
#include "SkynetBaseLocation.h"
#include "UnitTracker.h"

#include "RectangleArray.h"

#include <future>

class SkynetTerrainAnalyser : public TerrainAnalyserInterface, public MessageListener<UnitDiscover, UnitMorphRenegade, UnitDestroy>
{
public:
	SkynetTerrainAnalyser( Core & core );

	void update();

	const std::vector<Region> & getRegions( bool static_data ) const override { return (static_data ? m_processed_static_data : m_processed_dynamic_data).m_regions; }
	const std::vector<Chokepoint> & getChokepoints( bool static_data ) const override { return (static_data ? m_processed_static_data : m_processed_dynamic_data).m_chokepoints; }
	const std::vector<BaseLocation> & getBaseLocations( bool static_data ) const override { return (static_data ? m_processed_static_data : m_processed_dynamic_data).m_base_locations; }

	Region getRegion( WalkPosition pos, bool static_data ) const override { return (static_data ? m_processed_static_data : m_processed_dynamic_data).m_tile_to_region[clampToMap( pos )]; }
	int getClearance( WalkPosition pos, bool static_data ) const override { return (static_data ? m_processed_static_data : m_processed_dynamic_data).m_tile_clearance[clampToMap( pos )]; }
	int getConnectivity( WalkPosition pos, bool static_data ) const override { return (static_data ? m_processed_static_data : m_processed_dynamic_data).m_tile_connectivity[clampToMap( pos )]; }
	WalkPosition getClosestObstacle( WalkPosition pos, bool static_data ) const override { return (static_data ? m_processed_static_data : m_processed_dynamic_data).m_tile_to_closest_obstacle[clampToMap( pos )]; }

	std::pair<Chokepoint, Chokepoint> getTravelChokepoints( Region start, Region end, bool static_data ) const override;
	int getGroundDistance( WalkPosition start, WalkPosition end, bool static_data ) const override;

	WalkPosition clampToMap( WalkPosition pos ) const
	{
		if( pos.x >= m_map_size.x ) pos.x = m_map_size.x - 1;
		else if( pos.x < 0 ) pos.x = 0;

		if( pos.y >= m_map_size.y ) pos.y = m_map_size.y - 1;
		else if( pos.y < 0 ) pos.y = 0;

		return pos;
	}

private:
	WalkPosition m_map_size;

	struct Data
	{
		WalkPosition m_map_size;

		std::vector<std::pair<UnitType, TilePosition>> m_buildings_cache;

		bool m_analysed = false;

		std::vector<Region> m_regions;
		std::vector<std::unique_ptr<SkynetRegion>> m_region_storage;
		std::vector<Chokepoint> m_chokepoints;
		std::vector<std::unique_ptr<SkynetChokepoint>> m_chokepoint_storage;
		std::vector<BaseLocation> m_base_locations;
		std::vector<std::unique_ptr<SkynetBaseLocation>> m_base_location_storage;

		std::vector<bool> m_connectivity_to_small_obstacles;

		RectangleArray<SkynetRegion *, WALKPOSITION_SCALE> m_tile_to_region;
		RectangleArray<int, WALKPOSITION_SCALE> m_tile_clearance;
		RectangleArray<int, WALKPOSITION_SCALE> m_tile_connectivity;
		RectangleArray<WalkPosition, WALKPOSITION_SCALE> m_tile_to_closest_obstacle;
		RectangleArray<int> m_chokepoint_distances;

		int m_request = 0;

		float m_walkability_time_seconds = 0.0f;
		float m_connectivity_time_seconds = 0.0f;
		float m_clearance_time_seconds = 0.0f;
		float m_regions_time_seconds = 0.0f;

		Data & operator=( Data && other ) = default;

		void process( UnitGroup resources, bool debug_region );

		void calculateWalkability();
		void calculateConnectivity();
		void calculateClearance();
		void calculateRegions( bool debug_region );
		std::pair<WalkPosition, WalkPosition> findChokePoint( WalkPosition center ) const;
		int calculateShortestDistance( Chokepoint start_chokepoint, Chokepoint end_chokepoint ) const;
		void createBases( const UnitGroup & resources );
	};

	Data m_processed_static_data;
	Data m_processed_dynamic_data;
	Data m_processing_dynamic_data;

	UnitGroup m_buildings;

	int m_reprocess_request = 1;

	std::future<void> m_async_future;

	UnitGroup getResources();
	void checkDynamicData();

	bool tryLoadData( Data & data );
	void saveData( Data & data );

	void notify( const UnitDiscover & message ) override;
	void notify( const UnitMorphRenegade & message ) override;
	void notify( const UnitDestroy & message ) override;

	std::vector<std::unique_ptr<SkynetRegion>> m_old_regions;
	std::vector<std::unique_ptr<SkynetChokepoint>> m_old_chokepoints;
	std::vector<std::unique_ptr<SkynetBaseLocation>> m_old_base_locations;

	DEFINE_DEBUGGING_INTERFACE( Default, RegionAnalysis, StaticData );
};