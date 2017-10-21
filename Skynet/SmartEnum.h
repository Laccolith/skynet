#pragma once

#include "StringUtils.h"

#include <string>
#include <vector>

namespace SmartEnum
{
	template <typename U>
	inline std::vector<std::string> makeEnumNames( std::string enum_values_string )
	{
		std::vector<std::string> names;

		U current_enum_value = 0;
		while( !enum_values_string.empty() )
		{
			std::string current_enum_entry = StringUtils::extractEntry( enum_values_string );

			auto equal_sign_pos = current_enum_entry.find( '=' );
			if( equal_sign_pos != std::string::npos )
			{
				current_enum_value = std::stoi( current_enum_entry.substr( equal_sign_pos + 1 ) );
				current_enum_entry.erase( equal_sign_pos );
			}

			StringUtils::trimWhitespace( current_enum_entry );

			names.resize( current_enum_value + 1 );
			names[current_enum_value] = std::move( current_enum_entry );

			current_enum_value++;
		}

		return names;
	}

	template <typename T, typename U>
	std::vector<T> makeEnumValues( std::string enum_values_string )
	{
		std::vector<T> values;

		U current_enum_value = 0;
		while( !enum_values_string.empty() )
		{
			std::string current_enum_entry = StringUtils::extractEntry( enum_values_string );

			auto equal_sign_pos = current_enum_entry.find( '=' );
			if( equal_sign_pos != std::string::npos )
			{
				current_enum_value = std::stoi( current_enum_entry.substr( equal_sign_pos + 1 ) );
			}

			values.push_back( static_cast<T>(current_enum_value) );

			current_enum_value++;
		}

		return values;
	}
}

template <typename T>
inline const std::vector<T> & values()
{
	static_assert(false, "values called with non enum type");
	static std::vector<T> list;
	return list;
}

#define SMART_ENUM( TYPE, UNDERLYING_TYPE, ... ) \
	enum class TYPE : UNDERLYING_TYPE { __VA_ARGS__ }; \
	\
	template <> \
	inline static const std::vector<TYPE> & values<TYPE>() \
	{ \
		static const auto list = SmartEnum::makeEnumValues<TYPE, UNDERLYING_TYPE>( #__VA_ARGS__ ); \
		return list; \
	} \
	\
	inline static const std::string & toString( TYPE value ) \
	{ \
		static const auto names = SmartEnum::makeEnumNames<UNDERLYING_TYPE>( #__VA_ARGS__ ); \
		return names.at( (UNDERLYING_TYPE)value ); \
	}