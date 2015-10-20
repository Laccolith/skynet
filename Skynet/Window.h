#pragma once

#include "Types.h"

#include <memory>
#include <string>

class WindowImpl;
class Window
{
public:
	Window();
	Window( std::string title, int width, int height );
	~Window();

	void addBox( int left, int top, int right, int bottom, Color color );
	void addBox( Position top_left, Position bottom_right, Color color );

private:
	std::unique_ptr<WindowImpl> m_impl;
};