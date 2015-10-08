#include "SkynetInterface.h"

#include "Skynet.h"

SkynetInterface::SkynetInterface( Access & access, std::string name )
	: m_access( access )
	, m_name( std::move( name ) )
{
	getSkynet().registerInterface( *this );
}
