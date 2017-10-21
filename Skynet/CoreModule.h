#pragma once

#include "CoreAccess.h"

#include <string>

class CoreModule : public CoreAccess
{
public:
	CoreModule( Core & core, std::string name );
	virtual ~CoreModule() {}

	virtual bool debugCommand( const std::string & str ) { return false; }

	const std::string &getName() const { return m_name; }

private:
	std::string m_name;
};

#include "SmartEnum.h"
#include <bitset>
#define DEFINE_DEBUGGING_INTERFACE( ... ) \
	enum class Debug : unsigned int { __VA_ARGS__, Count }; \
	std::bitset<unsigned int(Debug::Count)> m_debug_flags; \
	\
	bool isDebugging( Debug d ) const \
	{ \
		return m_debug_flags[(unsigned int)d]; \
	} \
	void setDebugging( Debug d, bool enabled ) \
	{ \
		m_debug_flags[(unsigned int)d] = enabled; \
	} \
	\
	bool debugCommand( const std::string & str ) override \
	{ \
		static const auto names = SmartEnum::makeEnumNames<unsigned int>( #__VA_ARGS__ ); \
		for( unsigned int i = 0; i < names.size(); ++i ) \
		{ \
			if( str == names[i] ) \
			{ \
				m_debug_flags[i] = !m_debug_flags[i]; \
				return true; \
			} \
		} \
		\
		return false; \
	}