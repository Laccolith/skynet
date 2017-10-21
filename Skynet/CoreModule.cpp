#include "CoreModule.h"

CoreModule::CoreModule( Core & core, std::string name )
	: CoreAccess( core )
	, m_name( std::move( name ) )
{
	core.registerModule( *this );
}
