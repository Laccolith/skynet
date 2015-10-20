#include "DrawBuffer.h"

#include "Skynet.h"

DrawBuffer::DrawBuffer( Access & access )
	: SkynetInterface( access, "DrawBuffer" )
{
	getSkynet().registerUpdateProcess( 100000000.0f, [this]() { update(); } );
}

void DrawBuffer::update()
{
	std::lock_guard<std::mutex> lock( m_items_mutex );

	m_current_frame = BWAPI::Broodwar->getFrameCount();
	m_items.erase( std::remove_if( m_items.begin(), m_items.end(), [this]( const std::unique_ptr<BufferedItem> & item )
	{
		if( item->shouldDelete( m_current_frame ) )
			return true;

		item->draw();
		return false;
	} ), m_items.end() );
}