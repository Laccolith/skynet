#pragma once

#include "SquadManager.h"

#include "UnitTracker.h"

class SkynetSquad;
class SkynetSquadManager : public SquadManagerInterface, public MessageListener<UnitDiscover, UnitMorphRenegade, UnitDestroy>
{
public:
	SkynetSquadManager( Core & core );

	void update();

private:
	void notify( const UnitDiscover & message ) override;
	void notify( const UnitMorphRenegade & message ) override;
	void notify( const UnitDestroy & message ) override;

	std::vector<std::unique_ptr<SkynetSquad>> m_squads;
	std::map<Unit, SkynetSquad*> m_squad_assignments;

	std::vector<Unit> m_unassigned_units;

	DEFINE_DEBUGGING_INTERFACE( Default );
};
