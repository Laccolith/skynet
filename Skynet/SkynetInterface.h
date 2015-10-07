#pragma once

#include "Access.h"

class SkynetInterface
{
public:
	SkynetInterface( Access & access ) : m_access( access ) {}
	virtual ~SkynetInterface() {}

	void toggleDebug() { m_debug = !m_debug; }
	bool isDebugging() const { return m_debug; }

	Access & access() { return m_access; }
	const Access & access() const { return m_access; }

private:
	Access & m_access;

	bool m_debug = false;
};