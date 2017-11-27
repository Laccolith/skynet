#include "SkynetSquad.h"

void SkynetSquad::update()
{
	;
}

void SkynetSquad::add_unit( Unit unit )
{
	m_units.insert( unit );
}

void SkynetSquad::remove_unit( Unit unit )
{
	m_units.remove( unit );
}
