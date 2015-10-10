#pragma once

#include "BaseLocation.h"

class SkynetTerrainAnalyser;
class SkynetBaseLocation : public BaseLocationInterface
{
public:
	SkynetBaseLocation( TilePosition build_location, Region region, const UnitGroup & resources );

	Position getCenterPosition() const override { return m_center_position; }
	TilePosition getBuildLocation() const override { return m_build_location; }

	Region getRegion() const override { return m_region; }

	const UnitGroup &getMinerals() const override { return m_minerals; }
	const UnitGroup &getGeysers() const override { return m_geysers; }

	bool isValid() const override { return m_is_valid; }

	void draw( Color color ) const override;

protected:
	friend SkynetTerrainAnalyser;

	void addResources( const UnitGroup & resources );

	void markInvalid() { m_is_valid = false; }

private:
	Position m_center_position;
	TilePosition m_build_location;

	Region m_region = nullptr;

	UnitGroup m_minerals;
	UnitGroup m_geysers;

	bool m_is_valid = true;
};