#pragma once

#include "TerrainAnalyser.h"

#include "SkynetRegion.h"
#include "SkynetChokepoint.h"
#include "SkynetBaseLocation.h"

#include "RectangleArray.h"

#include <future>

class SkynetTerrainAnalyser : public TerrainAnalyserInterface
{
public:
	SkynetTerrainAnalyser( Access & access );

	void update();

	const std::vector<Region> &getRegions() const override { return m_data.m_regions; }
	const std::vector<Chokepoint> &getChokepoints() const override { return m_data.m_chokepoints; }
	const std::vector<BaseLocation> &getBaseLocations() const override { return m_data.m_base_locations; }

	Region getRegion( WalkPosition pos ) const override { return m_data.m_tile_to_region[clampToMap(pos)]; }
	int getClearance( WalkPosition pos ) const override { return m_data.m_tile_clearance[clampToMap( pos )]; }
	int getConnectivity( WalkPosition pos ) const override { return m_data.m_tile_connectivity[clampToMap( pos )]; }
	WalkPosition getClosestObstacle( WalkPosition pos ) const override { return m_data.m_tile_to_closest_obstacle[clampToMap( pos )]; }

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

		int m_request = 0;

		Data & operator=( Data && other)
		{
			m_analysed = other.m_analysed;

			m_regions = std::move( other.m_regions );
			m_region_storage = std::move( other.m_region_storage );
			m_chokepoints = std::move( other.m_chokepoints );
			m_chokepoint_storage = std::move( other.m_chokepoint_storage );
			m_base_locations = std::move( other.m_base_locations );
			m_base_location_storage = std::move( other.m_base_location_storage );

			m_connectivity_to_small_obstacles = std::move( other.m_connectivity_to_small_obstacles );

			m_tile_to_region = std::move( other.m_tile_to_region );
			m_tile_clearance = std::move( other.m_tile_clearance );
			m_tile_connectivity = std::move( other.m_tile_connectivity );
			m_tile_to_closest_obstacle = std::move( other.m_tile_to_closest_obstacle );

			m_request = other.m_request;

			return *this;
		}
	};

	Data m_data;

	int m_reprocess_request = 0;

	struct Process
	{
		Process( WalkPosition map_size, Data & data, const UnitGroup & resources );

		WalkPosition m_map_size;
		Data & m_data;
		UnitGroup m_resources;

		void calculateConnectivity();
		void calculateWalkTileClearance();
		void calculateRegions();
		std::pair<WalkPosition, WalkPosition> findChokePoint( WalkPosition center ) const;
		void createBases();
	};

	std::future<std::unique_ptr<Data>> m_async_future;

	UnitGroup getResources();
	void check_data();

	std::vector<std::unique_ptr<SkynetRegion>> m_old_regions;
	std::vector<std::unique_ptr<SkynetChokepoint>> m_old_chokepoints;
	std::vector<std::unique_ptr<SkynetBaseLocation>> m_old_base_locations;
};