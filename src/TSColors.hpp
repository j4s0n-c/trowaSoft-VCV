#ifndef TSCOLORS_HPP
#define TSCOLORS_HPP

#include <rack.hpp>
using namespace rack;

namespace TSColors {

//--------------------
// Wrapper for colors
//--------------------
// If colors change again, then we can just change this...
static const NVGcolor COLOR_BLACK_TRANSPARENT = componentlibrary::SCHEME_BLACK_TRANSPARENT;
static const NVGcolor COLOR_BLACK = componentlibrary::SCHEME_BLACK;
static const NVGcolor COLOR_WHITE = componentlibrary::SCHEME_WHITE;
static const NVGcolor COLOR_RED = componentlibrary::SCHEME_RED;
static const NVGcolor COLOR_ORANGE = componentlibrary::SCHEME_ORANGE;
static const NVGcolor COLOR_YELLOW = componentlibrary::SCHEME_YELLOW;
static const NVGcolor COLOR_GREEN = componentlibrary::SCHEME_GREEN;
static const NVGcolor COLOR_CYAN = componentlibrary::SCHEME_CYAN;
static const NVGcolor COLOR_BLUE = componentlibrary::SCHEME_BLUE;
static const NVGcolor COLOR_PURPLE = componentlibrary::SCHEME_PURPLE;
static const NVGcolor COLOR_LIGHT_GRAY = componentlibrary::SCHEME_LIGHT_GRAY;
static const NVGcolor COLOR_DARK_GRAY = componentlibrary::SCHEME_DARK_GRAY;
static const NVGcolor COLOR_BEIGE = nvgRGB(255, 250, 200);
static const NVGcolor COLOR_BROWN = nvgRGB(170, 110, 40);
static const NVGcolor COLOR_CORAL = nvgRGB(255, 215, 180);
static const NVGcolor COLOR_DARK_ORANGE = nvgRGB(0xFF, 0x8C, 0x00);
static const NVGcolor COLOR_LAVENDER = nvgRGB(230, 190, 255);
static const NVGcolor COLOR_LIME = nvgRGB(210, 245, 60);
static const NVGcolor COLOR_MAGENTA = nvgRGB(240, 50, 230);
static const NVGcolor COLOR_MAROON = nvgRGB(128, 0, 0);
static const NVGcolor COLOR_MINT = nvgRGB(170, 255, 195);
static const NVGcolor COLOR_NAVY = nvgRGB(0, 0, 128);
static const NVGcolor COLOR_OLIVE = nvgRGB(128, 128, 0);
static const NVGcolor COLOR_PINK = nvgRGB(250, 190, 190);
static const NVGcolor COLOR_PUMPKIN_ORANGE = nvgRGB(0xF8, 0x72, 0x17);
static const NVGcolor COLOR_TEAL = nvgRGB(0, 128, 128);
static const NVGcolor COLOR_TS_BLUE = nvgRGB(0x33, 0x66, 0xFF);
static const NVGcolor COLOR_TS_GRAY = nvgRGB(0xAA, 0xAA, 0xAB);
static const NVGcolor COLOR_TS_GREEN = nvgRGB(0x00, 0xFF, 0x00);
static const NVGcolor COLOR_TS_ORANGE = nvgRGB(0xFF, 0xA5, 0x00);
static const NVGcolor COLOR_TS_RED = nvgRGB(0xFF, 0x00, 0x00);
static const NVGcolor COLOR_TS_TEXT = nvgRGB(0xee, 0xee, 0xee);
static const NVGcolor COLOR_TS_BORDER = nvgRGB(0x66, 0x66, 0x66);



	static const int  NUM_CHANNEL_COLORS = 16;
	static const NVGcolor CHANNEL_COLORS[NUM_CHANNEL_COLORS] = {
		TSColors::COLOR_TS_RED, TSColors::COLOR_DARK_ORANGE, TSColors::COLOR_YELLOW, TSColors::COLOR_TS_GREEN,
		TSColors::COLOR_CYAN, TSColors::COLOR_TS_BLUE, TSColors::COLOR_PURPLE, TSColors::COLOR_PINK,
		TSColors::COLOR_TS_RED, TSColors::COLOR_DARK_ORANGE, TSColors::COLOR_YELLOW, TSColors::COLOR_TS_GREEN,
		TSColors::COLOR_CYAN, TSColors::COLOR_TS_BLUE, TSColors::COLOR_PURPLE, TSColors::COLOR_PINK
	};


} // end namespace

#endif // end if not defined