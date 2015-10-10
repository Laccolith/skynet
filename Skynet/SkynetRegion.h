#pragma once

#include "Region.h"

class SkynetTerrainAnalyser;
class SkynetRegion : public RegionInterface
{
public:
	SkynetRegion( WalkPosition center, int clearance, int connectivity );

	WalkPosition getCenter() const override { return m_center; }
	int getClearance() const override { return m_clearance; }
	int getConnectivity() const override { return m_connectivity; }
	int getSize() const override { return m_size; }

	const VectorUnique<Chokepoint> &getChokepoints() const override { return m_chokepoints; }
	const VectorUnique<TilePosition> &getChokepointTiles() const override { return m_choke_point_tiles; }
	const VectorUnique<BaseLocation> &getBaseLocations() const override { return m_base_locations; }

	bool isValid() const override { return m_is_valid; }

	void draw( Color color ) const override;

protected:
	friend SkynetTerrainAnalyser;

	void addChokepoint( Chokepoint chokepoint ) { m_chokepoints.insert( chokepoint ); }
	void removeChokepoint( Chokepoint chokepoint ) { m_chokepoints.remove( chokepoint ); }

	void addBase( BaseLocation baseLocation ) { m_base_locations.insert( baseLocation ); }

	void setSize( int size ) { m_size = size; }

	void markInvalid() { m_is_valid = false; }

private:
	WalkPosition m_center;
	int m_clearance;
	int m_connectivity;
	int m_size;

	VectorUnique<Chokepoint> m_chokepoints;
	VectorUnique<TilePosition> m_choke_point_tiles;
	VectorUnique<BaseLocation> m_base_locations;

	bool m_is_valid = true;
};