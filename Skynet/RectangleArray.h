#pragma once

#include "Types.h"

template <typename T, int Scale = POSITION_SCALE>
class RectangleArray
{
public:
	RectangleArray( size_t width, size_t height, T val = T() )
		: m_width( width )
		, m_height( height )
		, m_data( new T[m_width * m_height] )
	{
		fill( val );
	}

	RectangleArray()
		: m_width( 0 )
		, m_height( 0 )
		, m_data( nullptr )
	{
	}

	~RectangleArray()
	{
		delete[] m_data;
	}

	RectangleArray( const RectangleArray &other )
		: m_width( other.m_width )
		, m_height( other.m_height )
		, m_data( new T[m_width * m_height] )
	{
		std::copy( other.m_data, other.m_data + (m_width * m_height), other.m_data );
	}

	RectangleArray( RectangleArray &&other )
		: m_width( other.m_width )
		, m_height( other.m_height )
		, m_data( other.m_data )
	{
		other.m_data = nullptr;
	}

	RectangleArray &operator=( const RectangleArray &other )
	{
		RectangleArray copy( other );

		std::swap( m_width, copy.m_width );
		std::swap( m_height, copy.m_height );
		std::swap( m_data, copy.m_data );

		return *this;
	}

	RectangleArray &operator=( RectangleArray &&other )
	{
		m_width = other.m_width;
		m_height = other.m_height;
		m_data = other.m_data;

		other.m_data = nullptr;

		return *this;
	}

	void resize( size_t width, size_t height, T val = T() )
	{
		delete[] m_data;

		m_width = width;
		m_height = height;

		m_data = new T[m_width * m_height];
		fill( val );
	}

	void fill( T val = T() )
	{
		std::fill( m_data, m_data + (m_width * m_height), val );
	}

	inline const T &at( BWAPI::Point<int, Scale> pos ) const
	{
		return m_data[pos.x + (pos.y * m_width)];
	}

	inline T &at( BWAPI::Point<int, Scale> pos )
	{
		return m_data[pos.x + (pos.y * m_width)];
	}

	inline const T &operator[]( BWAPI::Point<int, Scale> pos ) const
	{
		return m_data[pos.x + (pos.y * m_width)];
	}

	inline T &operator[]( BWAPI::Point<int, Scale> pos )
	{
		return m_data[pos.x + (pos.y * m_width)];
	}

	void set( BWAPI::Point<int, Scale> pos, T val )
	{
		m_data[pos.x + (pos.y * m_width)] = val;
	}

private:
	size_t m_width;
	size_t m_height;
	T* m_data;
};