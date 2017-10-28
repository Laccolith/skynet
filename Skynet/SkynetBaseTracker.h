#pragma once

#include "BaseTracker.h"

#include "TerrainAnalyser.h"
#include "SkynetBase.h"
#include "RectangleArray.h"
#include "Messaging.h"
#include "UnitTracker.h"

#include <memory>

class SkynetBaseTracker : public BaseTrackerInterface, public MessageListener<TerrainAnalysed>, public MessageListener<UnitDiscover, UnitMorphRenegade, UnitDestroy>
{
public:
	SkynetBaseTracker( Core & core );

	const std::vector<Base> &getAllBases() const override { return m_bases; }
	const std::vector<Base> &getAllBases( Player player ) const override;

	Base getBase( TilePosition position ) const override { return m_tile_to_base[position]; }

	void notify( const TerrainAnalysed & message ) override;
	void notify( const UnitDiscover & message ) override;
	void notify( const UnitMorphRenegade & message ) override;
	void notify( const UnitDestroy & message ) override;

	void update();

private:
	std::vector<Base> m_bases;
	std::vector<std::vector<Base>> m_player_bases;
	std::vector<std::unique_ptr<SkynetBase>> m_base_storage;

	RectangleArray<SkynetBase*, TILEPOSITION_SCALE> m_tile_to_base;

	DEFINE_DEBUGGING_INTERFACE( Default );
};