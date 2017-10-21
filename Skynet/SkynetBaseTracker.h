#pragma once

#include "BaseTracker.h"

#include "TerrainAnalyser.h"
#include "SkynetBase.h"
#include "RectangleArray.h"

#include <memory>

class SkynetBaseTracker : public BaseTrackerInterface, public MessageListener<TerrainAnalysed>
{
public:
	SkynetBaseTracker( Core & core );

	const std::vector<Base> &getAllBases() const override { return m_bases; }

	Base getBase( TilePosition position ) const override { return m_tile_to_base[position]; }

	void notify( const TerrainAnalysed & message ) override;

	void update();

private:
	std::vector<Base> m_bases;
	std::vector<std::unique_ptr<SkynetBase>> m_base_storage;

	RectangleArray<Base, TILEPOSITION_SCALE> m_tile_to_base;

	DEFINE_DEBUGGING_INTERFACE( Default );
};