#pragma once

#include <vector>
#include <map>

template <class Key, class Val>
class Heap
{
public:
	Heap( bool is_min_heap = false ) : m_is_min_heap( is_min_heap ) {}

	void push( const std::pair<Key, Val> &val )
	{
		int index = m_data.size();
		if( m_mapping.insert( std::make_pair( val.first, index ) ).second )
		{
			m_data.push_back( val );
			percolate_up( index );
		}
	}

	void pop()
	{
		if( m_data.empty() )
			return;

		m_mapping.erase( m_data.front().first );
		m_data.front() = m_data.back();
		m_data.pop_back();

		if( m_data.empty() )
			return;

		auto it = m_mapping.find( m_data.front().first );
		if( it != m_mapping.end() )
		{
			it->second = 0;
			percolate_down( 0 );
		}
	}

	const std::pair<Key, Val> &top() const
	{
		return m_data.front();
	}

	bool empty() const
	{
		return m_data.empty();
	}

	bool set( const Key &key, const Val &val )
	{
		auto it = m_mapping.find( key );
		if( it == m_mapping.end() )
		{
			push( std::make_pair( key, val ) );
			return true;
		}

		int index = it->second;
		m_data[index].second = val;
		index = percolate_up( index );

		if( index >= 0 && index < (int) m_data.size() )
		{
			percolate_down( index );
			return true;
		}

		return false;
	}

	const Val &get( const Key &key ) const
	{
		auto it = m_mapping.find( key );
		int index = it->second;
		return m_data[index].second;
	}

	bool contains( const Key &key ) const
	{
		auto it = m_mapping.find( key );
		return (it != m_mapping.end());
	}

	int size() const
	{
		return m_data.size();
	}

	void clear()
	{
		m_data.clear();
		m_mapping.clear();
	}

	bool erase( const Key &key )
	{
		auto it = m_mapping.find( key );
		if( it == m_mapping.end() )
			return false;

		if( m_data.size() == 1 )
			clear();
		else
		{
			int index = it->second;
			m_data[index] = m_data.back();
			m_data.pop_back();
			m_mapping.erase( it );
			percolate_down( index );
		}

		return true;
	}

private:
	std::vector<std::pair<Key, Val>> m_data;
	std::map<Key, int> m_mapping;
	bool m_is_min_heap;

	int percolate_up( int index )
	{
		if( index < 0 || index >= (int) m_data.size() )
			return -1;

		unsigned int parent = (index - 1) / 2;
		int m = m_is_min_heap ? -1 : 1;

		while( index > 0 && m * m_data[parent].second < m * m_data[index].second )
		{
			std::swap( m_data[parent], m_data[index] );
			m_mapping.find( m_data[index].first )->second = index;
			index = parent;
			parent = (index - 1) / 2;
		}
		m_mapping.find( m_data[index].first )->second = index;

		return index;
	}

	int percolate_down( int index )
	{
		if( index < 0 || index >= (int) m_data.size() )
			return -1;

		unsigned int lchild = index * 2 + 1;
		unsigned int rchild = index * 2 + 2;
		unsigned int mchild;
		int m = m_is_min_heap ? -1 : 1;

		while( (m_data.size() > lchild && m * m_data[index].second < m * m_data[lchild].second) || (m_data.size() > rchild && m * m_data[index].second < m * m_data[rchild].second) )
		{
			mchild = lchild;
			if( m_data.size() > rchild && m * m_data[rchild].second > m * m_data[lchild].second )
				mchild = rchild;

			std::swap( m_data[mchild], m_data[index] );
			m_mapping.find( m_data[index].first )->second = index;
			index = mchild;
			lchild = index * 2 + 1;
			rchild = index * 2 + 2;
		}

		m_mapping.find( m_data[index].first )->second = index;

		return index;
	}
};