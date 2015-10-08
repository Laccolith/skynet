#include "DrawingUtil.h"

void DrawingUtil::drawProgressBar( Position bottomLeft, int width, int height, float progressFraction, Color innerBar, Color outerBar )
{
	BWAPI::Broodwar->drawBoxMap( bottomLeft.x, bottomLeft.y - height, bottomLeft.x + width, bottomLeft.y, outerBar, true );

	int progressWidth = static_cast<int>(progressFraction * float( width ));

	BWAPI::Broodwar->drawBoxMap( bottomLeft.x + 1, bottomLeft.y - height + 1, bottomLeft.x + progressWidth - 1, bottomLeft.y - 1, innerBar, true );
}