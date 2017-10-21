#pragma once

#include "SkynetInterface.h"

#include <vector>
#include <mutex>

struct BufferedText
{
	template <typename ...ARGS>
	static void draw( const ARGS &... args ) { BWAPI::Broodwar->drawText( args... ); }
};

struct BufferedBox
{
	template <typename ...ARGS>
	static void draw( const ARGS &... args ) { BWAPI::Broodwar->drawBox( args... ); }
};

struct BufferedTriangle
{
	template <typename ...ARGS>
	static void draw( const ARGS &... args ) { BWAPI::Broodwar->drawTriangle( args... ); }
};

struct BufferedCircle
{
	template <typename ...ARGS>
	static void draw( const ARGS &... args ) { BWAPI::Broodwar->drawCircle( args... ); }
};

struct BufferedEllipse
{
	template <typename ...ARGS>
	static void draw( const ARGS &... args ) { BWAPI::Broodwar->drawEllipse( args... ); }
};

struct BufferedDot
{
	template <typename ...ARGS>
	static void draw( const ARGS &... args ) { BWAPI::Broodwar->drawDot( args... ); }
};

struct BufferedLine
{
	template <typename ...ARGS>
	static void draw( const ARGS &... args ) { BWAPI::Broodwar->drawLine( args... ); }
};

class BufferedItem;
class DrawBuffer : public SkynetInterface
{
public:
	DrawBuffer( Access & access );

	void update();

	template <typename T, typename ...ARGS>
	void drawMap( int num_frames, ARGS&&... args )
	{
		add<T>( num_frames, CoordinateType::Map, std::forward<ARGS>( args )... );
	}

	template <typename T, typename ...ARGS>
	void drawMap( int num_frames, Position pos, ARGS&&... args )
	{
		add<T>( num_frames, CoordinateType::Map, pos.x, pos.y, std::forward<ARGS>( args )... );
	}

	template <typename T, typename ...ARGS>
	void drawMap( int num_frames, Position pos_1, Position pos_2, ARGS&&... args )
	{
		add<T>( num_frames, CoordinateType::Map, pos_1.x, pos_1.y, pos_2.x, pos_2.y, std::forward<ARGS>( args )... );
	}

	template <typename T, typename ...ARGS>
	void drawMouse( int num_frames, ARGS&&... args )
	{
		add<T>( num_frames, CoordinateType::Mouse, std::forward<ARGS>( args )... );
	}

	template <typename T, typename ...ARGS>
	void drawMouse( int num_frames, Position pos, ARGS&&... args )
	{
		add<T>( num_frames, CoordinateType::Mouse, pos.x, pos.y, std::forward<ARGS>( args )... );
	}

	template <typename T, typename ...ARGS>
	void drawMouse( int num_frames, Position pos_1, Position pos_2, ARGS&&... args )
	{
		add<T>( num_frames, CoordinateType::Mouse, pos_1.x, pos_1.y, pos_2.x, pos_2.y, std::forward<ARGS>( args )... );
	}

	template <typename T, typename ...ARGS>
	void drawScreen( int num_frames, ARGS&&... args )
	{
		add<T>( num_frames, CoordinateType::Screen, std::forward<ARGS>( args )... );
	}

	template <typename T, typename ...ARGS>
	void drawScreen( int num_frames, Position pos, ARGS&&... args )
	{
		add<T>( num_frames, CoordinateType::Screen, pos.x, pos.y, std::forward<ARGS>( args )... );
	}

	template <typename T, typename ...ARGS>
	void drawScreen( int num_frames, Position pos_1, Position pos_2, ARGS&&... args )
	{
		add<T>( num_frames, CoordinateType::Screen, pos_1.x, pos_1.y, pos_2.x, pos_2.y, std::forward<ARGS>( args )... );
	}

private:
	int m_current_frame = 0;
	std::vector<std::unique_ptr<BufferedItem>> m_items;
	std::mutex m_items_mutex;

	template <typename T, typename ...ARGS>
	void add( int num_frames, ARGS&&... args )
	{
		std::lock_guard<std::mutex> lock( m_items_mutex );
		m_items.emplace_back( std::make_unique<BufferedItemImpl<T, ARGS...>>( m_current_frame + num_frames, std::forward<ARGS>( args )... ) );
	}
};

class BufferedItem
{
public:
	BufferedItem( int removal_frame ) : m_removal_frame( removal_frame ) {}
	virtual ~BufferedItem() {}

	virtual void draw() const = 0;

	bool shouldDelete( int current_frame ) const { return current_frame >= m_removal_frame; }

private:
	int m_removal_frame;
};

template<int ...> struct Seq {};

template<int N, int ...S> struct Gens : Gens<N - 1, N - 1, S...> {};

template<int ...S> struct Gens<0, S...> { typedef Seq<S...> type; };

template <class T>
struct StorageType
{
	typedef T type;
};

template <>
struct StorageType<const char*>
{
	typedef std::string type;
};

template <>
struct StorageType<char*>
{
	typedef std::string type;
};

template <class T>
struct Out
{
	static const T & out( const T & t ) { return t; }
};

template<>
struct Out<std::string>
{
	static const char * out( const std::string & str ) { return str.c_str(); }
};

template <typename T, typename ...ARGS>
class BufferedItemImpl : public BufferedItem
{
public:
	BufferedItemImpl( int removal_frame, ARGS&&... args )
		: BufferedItem( removal_frame )
		, m_arguments( std::forward<ARGS>( args )... )
	{
	}

	void draw() const override
	{
		drawImp( typename Gens<std::tuple_size<decltype(m_arguments)>::value>::type() );
	}

	template<int ...S>
	void drawImp( Seq<S...> ) const
	{
		T::draw( Out<std::tuple_element<S, decltype(m_arguments)>::type>::out( std::get<S>( m_arguments ) )... );
	}

private:
	std::tuple<typename StorageType<typename std::decay<ARGS>::type>::type...> m_arguments;
};