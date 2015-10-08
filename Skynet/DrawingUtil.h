#pragma once

#include "Types.h"

namespace DrawingUtil
{
	void drawProgressBar( Position bottomLeft, int width, int height, float progressFraction, Color innerBar = Colors::Green, Color outerBar = Colors::Black );
}