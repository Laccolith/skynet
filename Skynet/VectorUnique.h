#pragma once

#include <vector>

template <typename T>
class VectorUnique
{
public:
	typedef std::vector<T> ContainerType;
	typedef typename ContainerType::value_type ValueType;
	typedef typename ContainerType::size_type SizeType;

	typedef typename ContainerType::const_iterator ConstIterator;

	VectorUnique() = default;
	VectorUnique( const VectorUnique &other ) : m_values( other.m_values ) {}
	VectorUnique( VectorUnique &&other ) : m_values( std::move( other.m_values ) ) {}

	VectorUnique &operator=( const VectorUnique &other ) { m_values = other.m_values; return *this; }
	VectorUnique &operator=( VectorUnique &&other ) { m_values = std::move( other.m_values ); return *this; }
	VectorUnique &operator+=( const VectorUnique &other ) { for( auto other_val : other ) insert( other_val ); return *this; }
	VectorUnique operator+( const VectorUnique &other ) const { return VectorUnique( *this ) += other; }

	SizeType size() const { return m_values.size(); }
	bool empty() const { return m_values.empty(); }

	void insert( T val )
	{
		if( std::find( m_values.begin(), m_values.end(), val ) == m_values.end() )
			m_values.push_back( val );
	}

	void remove( T val )
	{
		auto it = std::find( m_values.begin(), m_values.end(), val );
		if( it != m_values.end() )
		{
			std::swap( *it, m_values.back() );
			m_values.pop_back();
		}
	}

	void reserve( SizeType size ) { m_values.reserve( size ); }

	ConstIterator begin() const { return m_values.begin(); }
	ConstIterator end() const { return m_values.end(); }

	ConstIterator erase( ConstIterator it ) { return m_values.erase( it ); }
	ConstIterator erase( ConstIterator first, ConstIterator second ) { return m_values.erase( first, second ); }

	void clear() { m_values.clear(); }
	void swap( VectorUnique &other ) { m_values.swap( other.m_values ); }

private:
	ContainerType m_values;
};