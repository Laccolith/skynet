#pragma once

#include "TerrainAnalyser.h"

#include "SkynetRegion.h"
#include "SkynetChokepoint.h"

#include "RectangleArray.h"

class SkynetTerrainAnalyser : public TerrainAnalyserInterface
{
public:
	SkynetTerrainAnalyser( Access & access );

	void update();

	const std::vector<Region> &getRegions() const override { return m_regions; }
	const std::vector<Chokepoint> &getChokepoints() const override { return m_chokepoints; }

	Region getRegion( WalkPosition pos ) const override { return m_tile_to_region[clampToMap(pos)]; }
	int getClearance( WalkPosition pos ) const override { return m_tile_clearance[clampToMap( pos )]; }
	int getConnectivity( WalkPosition pos ) const override { return m_tile_connectivity[clampToMap( pos )]; }

	WalkPosition clampToMap( WalkPosition pos ) const
	{
		if( pos.x >= m_map_width ) pos.x = m_map_width - 1;
		else if( pos.x < 0 ) pos.x = 0;

		if( pos.y >= m_map_height ) pos.y = m_map_height - 1;
		else if( pos.y < 0 ) pos.y = 0;

		return pos;
	}

private:
	int m_map_width;
	int m_map_height;

	bool m_analysed = false;

	std::vector<Region> m_regions;
	std::vector<std::unique_ptr<SkynetRegion>> m_region_storage;
	std::vector<Chokepoint> m_chokepoints;
	std::vector<std::unique_ptr<SkynetChokepoint>> m_chokepoint_storage;

	std::vector<bool> m_connectivity_to_small_obstacles;

	RectangleArray<Region, WALKPOSITION_SCALE> m_tile_to_region;
	RectangleArray<int, WALKPOSITION_SCALE> m_tile_clearance;
	RectangleArray<int, WALKPOSITION_SCALE> m_tile_connectivity;
	RectangleArray<WalkPosition, WALKPOSITION_SCALE> m_tile_to_closest_obstacle;

	int m_region_connectivity_count;

	void analyse();

	void calculateConnectivity();
	void calculateWalkTileClearance();
	void calculateRegions();
	std::pair<WalkPosition, WalkPosition> findChokePoint( WalkPosition center ) const;
	void createBases();
	void finaliseConnectivity();
};