#include "DrawBuffer.h"

#include "CoreModule.h"
#include "Types.h"

DrawBuffer::DrawBuffer( Core & core )
	: CoreModule( core, "DrawBuffer" )
{
	core.registerUpdateProcess( 100000000.0f, [this]() { update(); } );
}

void DrawBuffer::update()
{
	std::lock_guard<std::mutex> lock( m_items_mutex );

	m_items.erase( std::remove_if( m_items.begin(), m_items.end(), [this]( const std::unique_ptr<BufferedItem> & item )
	{
		if( item->update() )
			return true;

		item->draw();
		return false;
	} ), m_items.end() );
}