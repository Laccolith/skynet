#pragma once

#include "CoreAccess.h"

#include <functional>

class Condition
{
public:
	Condition( std::function<bool( CoreAccess & access )> test_function )
		: m_test_function( std::move( test_function ) )
	{
	}

	Condition operator&&( const Condition &other ) const
	{
		return Condition( [left = *this, right = other]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) && right.evaluate( access );
		} );
	}

	Condition operator||( const Condition &other ) const
	{
		return Condition( [left = *this, right = other]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) || right.evaluate( access );
		} );
	}

	Condition operator!() const
	{
		return Condition( [left = *this]( CoreAccess & access ) -> bool
		{
			return !left.evaluate( access );
		} );
	}

	bool evaluate( CoreAccess & access ) const
	{
		return m_test_function( access );
	}

private:
	std::function<bool( CoreAccess & access )> m_test_function;
};

template <typename T>
class Value
{
public:
	Value( std::function<T( CoreAccess & access )> test_function )
		: m_test_function( std::move( test_function ) )
	{
	}

	Condition operator==( T value ) const
	{
		return Condition( [left = *this, value]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) == value;
		} );
	}

	Condition operator!=( T value ) const
	{
		return Condition( [left = *this, value]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) != value;
		} );
	}

	Condition operator>( T value ) const
	{
		return Condition( [left = *this, value]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) > value;
		} );
	}

	Condition operator>=( T value ) const
	{
		return Condition( [left = *this, value]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) >= value;
		} );
	}

	Condition operator<( T value ) const
	{
		return Condition( [left = *this, value]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) < value;
		} );
	}

	Condition operator<=( T value ) const
	{
		return Condition( [left = *this, value]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) <= value;
		} );
	}

	template <typename O>
	Condition operator==( const Value<O> & other ) const
	{
		return Condition( [left = *this, right = other]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) == right.evaluate( access );
		} );
	}

	template <typename O>
	Condition operator!=( const Value<O> & other ) const
	{
		return Condition( [left = *this, right = other]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) != right.evaluate( access );
		} );
	}

	template <typename O>
	Condition operator>( const Value<O> & other ) const
	{
		return Condition( [left = *this, right = other]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) > right.evaluate( access );
		} );
	}

	template <typename O>
	Condition operator>=( const Value<O> & other ) const
	{
		return Condition( [left = *this, right = other]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) >= right.evaluate( access );
		} );
	}

	template <typename O>
	Condition operator<( const Value<O> & other ) const
	{
		return Condition( [left = *this, right = other]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) < right.evaluate( access );
		} );
	}

	template <typename O>
	Condition operator<=( const Value<O> & other ) const
	{
		return Condition( [left = *this, right = other]( CoreAccess & access ) -> bool
		{
			return left.evaluate( access ) <= right.evaluate( access );
		} );
	}

	T evaluate( CoreAccess & access ) const
	{
		return m_test_function( access );
	}

private:
	std::function<T( CoreAccess & access )> m_test_function;
};