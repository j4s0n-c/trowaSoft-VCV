#ifndef TROWASOFT_COMPONENTS_HPP
#define TROWASOFT_COMPONENTS_HPP

#include "rack.hpp"
using namespace rack;

#include <string.h>
#include <stdio.h>
#include "dsp/digital.hpp"
#include "components.hpp"
#include "trowaSoftUtilities.hpp"


//=======================================================
// trowaSoft - TurtleMonkey Components 
//=======================================================


//::: Helpers :::::::::::::::::::::::::::::::::::::::::::
// Extra colors (may be defined in future?)
#ifndef COLOR_MAGENTA
	#define COLOR_MAGENTA nvgRGB(240, 50, 230)
#endif
#ifndef COLOR_LIME
	#define COLOR_LIME nvgRGB(210, 245, 60)
#endif
#ifndef COLOR_PINK
	#define COLOR_PINK nvgRGB(250, 190, 190)
#endif
#ifndef COLOR_TEAL
	#define COLOR_TEAL nvgRGB(0, 128, 128)
#endif
#ifndef COLOR_LAVENDER
	#define COLOR_LAVENDER nvgRGB(230, 190, 255)
#endif
#ifndef COLOR_BROWN
	#define COLOR_BROWN nvgRGB(170, 110, 40)
#endif
#ifndef COLOR_BEIGE
	#define COLOR_BEIGE nvgRGB(255, 250, 200)
#endif
#ifndef COLOR_MAROON
	#define COLOR_MAROON nvgRGB(128, 0, 0)
#endif
#ifndef COLOR_MINT
	#define COLOR_MINT nvgRGB(170, 255, 195)
#endif
#ifndef COLOR_OLIVE
	#define COLOR_OLIVE nvgRGB(128, 128, 0)
#endif
#ifndef COLOR_CORAL
	#define COLOR_CORAL nvgRGB(255, 215, 180)
#endif
#ifndef COLOR_NAVY
	#define COLOR_NAVY nvgRGB(0, 0, 128)
#endif
#ifndef COLOR_DARK_ORANGE
	#define COLOR_DARK_ORANGE nvgRGB(0xFF, 0x8C, 0x00)
#endif 
#ifndef COLOR_PUMPKIN_ORANGE
	#define COLOR_PUMPKIN_ORANGE nvgRGB(0xF8, 0x72, 0x17)
#endif 

#ifndef COLOR_TS_RED
	// Orange in components.hpp is different
	#define COLOR_TS_RED nvgRGB(0xFF, 0x00, 0x00)
#endif 
#ifndef COLOR_TS_ORANGE
	// Orange in components.hpp is different
	#define COLOR_TS_ORANGE nvgRGB(0xFF, 0xA5, 0x00)
#endif 
#ifndef COLOR_TS_GREEN
	#define COLOR_TS_GREEN nvgRGB(0x00, 0xFF, 0x00)
#endif 
#ifndef COLOR_TS_BLUE
	#define COLOR_TS_BLUE nvgRGB(0x33, 0x66, 0xFF)
#endif 
#ifndef COLOR_TS_GRAY
	#define COLOR_TS_GRAY nvgRGB(0xAA, 0xAA, 0xAB)
#endif 


struct ColorValueLight : ModuleLightWidget {
	NVGcolor baseColor;
	ColorValueLight()
	{
		return;
	};
	virtual ~ColorValueLight(){};
	// Set a single color
	void setColor(NVGcolor bColor)
	{
		baseColor = bColor;
		if (baseColors.size() < 1)
		{
			baseColors.push_back(bColor);			
		}
		else
		{
			baseColors[0] = bColor;
		}
	}
};

//--------------------------------------------------------------
// SnapKnob
//--------------------------------------------------------------
struct TS_SnapKnob : RoundBlackKnob
{
	TS_SnapKnob() {
		box.size = Vec(28, 28);
	}
	void randomize() override { return; }
	void onDragEnd(EventDragEnd &e) override {
		setValue(roundf(value));
		RoundBlackKnob::onDragEnd(e);
	}	
};

struct TS_PadSwitch : MomentarySwitch {
	TS_PadSwitch() {
		return;
	}
	TS_PadSwitch(Vec size) {
		box.size = size;		
		return;
	}
	
};


//--------------------------------------------------------------
// TS_PadSquare - A Square Pad button.
//--------------------------------------------------------------
struct TS_PadSquare : SVGSwitch, MomentarySwitch {	
	TS_PadSquare() 
	{
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};


//--------------------------------------------------------------
// TS_PadBtn - A wide Pad button.
//--------------------------------------------------------------
struct TS_PadBtn : SVGSwitch, MomentarySwitch {
	
	TS_PadBtn() 
	{
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_btn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_btn_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}	
};

//--------------------------------------------------------------
// TS_PadRun - A wide Pad button.
//--------------------------------------------------------------
struct TS_Pad_Run : SVGSwitch, MomentarySwitch {
	
	TS_Pad_Run() 
	{
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_run_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_run_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}	
};

//--------------------------------------------------------------
// TS_PadReset - A wide Pad button.
//--------------------------------------------------------------
struct TS_Pad_Reset : SVGSwitch, MomentarySwitch {
	
	TS_Pad_Reset() 
	{
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_reset_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ComponentLibrary/TS_pad_reset_1.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}	
};

//------------------------------------------------------------------------------------------------
// Lighted arc for light knobs
//------------------------------------------------------------------------------------------------
struct TS_LightArc : ColorValueLight {
	// The inner radius 
	float innerRadius = 22;	
	// Pointer to current angle in radians. This is the differential like from a knob.
	float* currentAngle_radians;
	// Font size for our display numbers
	int fontSize;
	// Font face
	std::shared_ptr<Font> font;
	// Numeric value to print out
	float* numericValue;
	// Buffer for our light string.
	char lightString[10];
	// The point where the angle is considered 0 degrees / radians.
	float zeroAnglePoint;
	// Format string for displaying the numeric value.
	//const char* textFormatString = "%02.0f";
	
	ValueSequencerMode* valueMode;

	
	TS_LightArc()
	{
		font = Font::load(assetPlugin(plugin, "res/Fonts/ZeroesThree-Regular.ttf"));
		fontSize = 10;
		bgColor = nvgRGBAf(0.0, 0, 0, /*alpha */ 1.0);
		baseColor = COLOR_WHITE;
		zeroAnglePoint = 1.5*NVG_PI;
	}
	

	
	void draw(NVGcontext *vg) override
	{
		float oradius = box.size.x / 2.0; // 25
		float radius = oradius - 2; // 23
		
		float angle = *currentAngle_radians;
		zeroAnglePoint = valueMode->zeroPointAngle_radians;
		int dir = (angle < zeroAnglePoint) ? NVG_CCW : NVG_CW;
		//angle += zeroAnglePoint; // Our midpoint is 270 degrees (1.5 Pi) or straight up 

		// Background - Solid
		nvgBeginPath(vg);
		nvgCircle(vg, oradius, oradius, innerRadius);
		nvgFillColor(vg, bgColor);
		nvgFill(vg);
		
		nvgStrokeWidth(vg, radius - innerRadius);
		NVGcolor borderColor = color;// bgColor;
		borderColor.a *= 0.5;//1.0;
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		
		// svg Angles go clockwise from positive x -->
		
		// // Adds an arc segment at the corner defined by the last path point, and two specified points.
		// //void nvgArcTo(NVGcontext* ctx, float x1, float y1, float x2, float y2, float radius);
		// nvgBeginPath(vg);
		// //nvgArcTo(vg, oradius, oradius, float x2, float y2, float radius);
		// // Creates new circle arc shaped sub-path. The arc center is at cx,cy, the arc radius is r,
		// // and the arc is drawn from angle a0 to a1, and swept in direction dir (NVG_CCW, or NVG_CW).
		// // Angles are specified in radians.
		// // nvgArc(NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int dir);
		// nvgArc(vg, /*cx*/ oradius, /*cy*/ oradius, radius, 
			// /*a0*/ startAngle, /*a1*/ endAngle, /*dir*/ NVG_CW);
		// //nvgFillColor(vg, COLOR_WHITE);
		// //nvgFill(vg);
		
		// Inner glow
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgCircle(vg, oradius, oradius, radius);		
		borderColor = color;
		borderColor.a = 0.25;
		nvgStrokeWidth(vg, oradius - radius);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		
		
		nvgBeginPath(vg);
		//nvgArcTo(vg, oradius, oradius, float x2, float y2, float radius);
		// Creates new circle arc shaped sub-path. The arc center is at cx,cy, the arc radius is r,
		// and the arc is drawn from angle a0 to a1, and swept in direction dir (NVG_CCW, or NVG_CW).
		// Angles are specified in radians.
		// nvgArc(NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int dir);
		nvgArc(vg, /*cx*/ oradius, /*cy*/ oradius, /*radius*/ innerRadius, 
			/*a0*/ zeroAnglePoint, /*a1*/ angle, /*dir*/ dir);
		nvgStrokeWidth(vg, oradius - innerRadius);
		borderColor = baseColor;
		borderColor.a *= 0.7;
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		
		
		//nvgFillColor(vg, color);
		//nvgFill(vg);

		// Outer glow
		nvgBeginPath(vg);
		nvgArc(vg, /*cx*/ oradius, /*cy*/ oradius, innerRadius - 3, 
			 /*a0*/ zeroAnglePoint, /*a1*/ angle, /*dir*/ dir);
		// nvgArc(vg, /*cx*/ oradius, /*cy*/ oradius, radius, 
			// /*a0*/ startAngle, /*a1*/ endAngle, /*dir*/ NVG_CW);
	
		NVGpaint paint;
		NVGcolor icol = color;
		icol.a *= 0.8;
		NVGcolor ocol = color;
		ocol.a = 0.0;
		paint = nvgRadialGradient(vg, oradius, oradius, innerRadius, oradius, icol, ocol);
		nvgStrokeWidth(vg, oradius - innerRadius + 3);	
		nvgStrokePaint(vg, paint);
		nvgStroke(vg);
					

		if (numericValue != NULL)
		{
			nvgBeginPath(vg);
			nvgGlobalCompositeOperation(vg, NVG_SOURCE_OVER);//Restore to default.
			NVGcolor textColor = COLOR_WHITE;
			nvgFontSize(vg, fontSize); 	
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			float v = valueMode->GetOutputValue(*numericValue);
			valueMode->GetDisplayString(v, lightString);
			//sprintf(lightString, textFormatString, *numericValue);				
			nvgFillColor(vg, textColor);
			nvgText(vg, oradius, oradius, lightString, NULL);
		}
		return;
	}
};


struct TS_LightedKnob : SVGKnob {
	float currentAngle;
	float differentialAngle;
	const float zeroAnglePoint = 1.5*NVG_PI;

	TS_LightedKnob() {
		minAngle = -0.83*NVG_PI;
		maxAngle = 0.83*NVG_PI;
		box.size = Vec(50, 50);
		currentAngle = 0;
		minValue = -10;
		maxValue = 10;
		snap = false;
		return;
	}
	void randomize() override { return; }	
	void setKnobValue(float val)
	{
		value = val;
		differentialAngle = rescalef(value, minValue, maxValue, minAngle, maxAngle);
		currentAngle = zeroAnglePoint + differentialAngle;		
		this->dirty = true;
		return;
	}
	void step() override {
		// Re-transform TransformWidget if dirty
		if (dirty) {
			differentialAngle = rescalef(value, minValue, maxValue, minAngle, maxAngle);
			currentAngle = zeroAnglePoint + differentialAngle;
			tw->identity();
			// Scale SVG to box
			tw->scale(box.size.div(sw->box.size));
			// Rotate SVG
			Vec center = sw->box.getCenter();
			tw->translate(center);
			tw->rotate(currentAngle);
			tw->translate(center.neg());
		}
		FramebufferWidget::step();
	}
};


//--------------------------------------------------------------
// TS_LightString : A light with a string (message/text).
//--------------------------------------------------------------
struct TS_LightString : ColorValueLight
{
	const char * lightString;
	float cornerRadius = 3.0;	
	std::shared_ptr<Font> font;
	int fontSize;
	TS_LightString()
	{
		font = Font::load(assetPlugin(plugin, "res/Fonts/ZeroesThree-Regular.ttf"));
		fontSize = 14;
		bgColor = nvgRGBAf(0.2, 0.2, 0.2, /*alpha */ 1);
		baseColor = COLOR_WHITE;		
	}
	void draw(NVGcontext *vg) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius + 20.0;
		float radiusY = box.size.y / 2.0;
		float oradiusY = radiusY + 20.0;

		//NVGcolor backColor = bgColor;
		NVGcolor outerColor = color;
		
		// Solid
		nvgBeginPath(vg);
		//nvgCircle(vg, radius, radius, radius);
		//nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		// nvgFillColor(vg, backColor);
		// nvgFill(vg);

		// Border
		nvgStrokeWidth(vg, 1.0);
		NVGcolor borderColor = bgColor;
		borderColor.a *= 0.5;
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		
		// Inner glow
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgFillColor(vg, color);
		nvgFill(vg);

		//nvgGlobalCompositeOperation(vg, NVG_SOURCE_OVER);//Restore to default.
		
		// Outer glow
		nvgBeginPath(vg);
		//NVGcontext* ctx, float x, float y, float w, float h
		//nvgRect(vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);
		nvgRoundedRect(vg, /*x*/ radius - oradius, /*y*/ radiusY - oradiusY, /*w*/ 3*oradius, /*h*/ 2*oradiusY, cornerRadius);
		NVGpaint paint;
		NVGcolor icol = outerColor;// color;
		icol.a *= 0.5;
		NVGcolor ocol = outerColor;// color;
		ocol.a = 0.0;
		//paint = nvgRadialGradient(vg, /*center x */ radius, /*center y*/ radius, /*inner */ radius, /*outer*/oradius, icol, ocol);
		// NVGpaint nvgBoxGradient(NVGcontext* ctx, float x, float y, float w, float h,
						// float r, float f, NVGcolor icol, NVGcolor ocol);
		float feather = 3;
		// Feather defines how blurry the border of the rectangle is.
		paint = nvgBoxGradient(vg, /*x*/ 0, /*y*/ 0, /*w*/ box.size.x, /*h*/ oradiusY - 10, 
			/*r: corner radius*/ cornerRadius, /*f: feather*/ feather, 
			/*inner color*/ icol, /*outer color */ ocol);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
		
		nvgBeginPath(vg);
		nvgGlobalCompositeOperation(vg, NVG_SOURCE_OVER);//Restore to default.
		NVGcolor textColor = baseColor;
		nvgFillColor(vg, textColor);
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, font->handle);
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(vg, box.size.x / 2, box.size.y / 2, lightString, NULL);

		return;
	}	
};

//--------------------------------------------------------------
// TS_LightSquare - Square light. 
//--------------------------------------------------------------
struct TS_LightSquare : ColorValueLight 
{
	// Radius on corners
	float cornerRadius = 5.0;
	TS_LightSquare()
	{
		bgColor = nvgRGBAf(0, 0, 0, /*alpha */ 0.5);
		baseColor = COLOR_WHITE;
	}
	void draw(NVGcontext *vg) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius*1.2;

		NVGcolor backColor = bgColor;
		NVGcolor outerColor = color;
		// Solid
		nvgBeginPath(vg);
		//nvgCircle(vg, radius, radius, radius);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		nvgFillColor(vg, backColor);
		nvgFill(vg);

		// Border
		nvgStrokeWidth(vg, 1.0);
		NVGcolor borderColor = bgColor;
		borderColor.a *= 0.5;
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);

		// Inner glow
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgFillColor(vg, color);
		nvgFill(vg);

		//nvgGlobalCompositeOperation(vg, NVG_SOURCE_OVER);//Restore to default.
		
		// Outer glow
		nvgBeginPath(vg);
		//NVGcontext* ctx, float x, float y, float w, float h
		//nvgRect(vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);
		nvgRoundedRect(vg, /*x*/ radius - oradius, /*y*/ radius - oradius, /*w*/ 2*oradius, /*h*/ 2*oradius, cornerRadius);
		NVGpaint paint;
		NVGcolor icol = outerColor;// color;
		icol.a *= 0.25;
		NVGcolor ocol = outerColor;// color;
		ocol.a = 0.0;
		//paint = nvgRadialGradient(vg, /*center x */ radius, /*center y*/ radius, /*inner */ radius, /*outer*/oradius, icol, ocol);
		// NVGpaint nvgBoxGradient(NVGcontext* ctx, float x, float y, float w, float h,
						// float r, float f, NVGcolor icol, NVGcolor ocol);
		float feather = 3;
		// Feather defines how blurry the border of the rectangle is.
		paint = nvgBoxGradient(vg, /*x*/ -5, /*y*/ -5, /*w*/ oradius - 10, /*h*/ oradius - 10, 
			/*r: corner radius*/ cornerRadius, /*f: feather*/ feather, 
			/*inner color*/ icol, /*outer color */ ocol);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
		return;
	}
};


struct TS_LightRing : ColorValueLight 
{
	// The inner radius 
	float innerRadius = 6.8;
	
	TS_LightRing()
	{
		bgColor = nvgRGBAf(0, 0, 0, /*alpha */ 0.2);
		baseColor = COLOR_WHITE;
	}
	void draw(NVGcontext *vg) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius + 10.0;
		

		// Solid
		nvgBeginPath(vg);
		nvgCircle(vg, radius, radius, radius);

		// Border
		nvgStrokeWidth(vg, radius - innerRadius);
		NVGcolor borderColor = bgColor;
		borderColor.a *= 1.0;
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);

		// Inner glow
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		//nvgFillColor(vg, color);
		//nvgFill(vg);

		// Outer glow
		nvgBeginPath(vg);
		nvgRect(vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);
		NVGpaint paint;
		NVGcolor icol = color;
		icol.a *= 0.8;
		NVGcolor ocol = color;
		ocol.a = 0.0;
		paint = nvgRadialGradient(vg, radius, radius, innerRadius, oradius, icol, ocol);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
		return;
	}
};

// struct TinyBlackKnob : RoundBlackKnob {
	// TinyBlackKnob() {
		// box.size = Vec(20, 20);		
		// // //		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/RoundBlack.svg")));
		// // setSVG(SVG::load(assetPlugin(plugin, "res/ComponentLibrary/RoundBlack.svg")));
		// // box.size = Vec(20, 20);
	// }
	// void randomize() override { return; }	
// };

struct TS_Port : SVGPort {
	NVGcolor negColor;
	NVGcolor posColor;
	
	TS_Port() : SVGPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/ComponentLibrary/TS_Port.svg"));
		background->wrap();
		box.size = background->box.size;
		if (plugLight)
		{
			negColor = plugLight->baseColors[1];
			posColor = plugLight->baseColors[0];
		}
	}
	void disableLights()
	{
		// Save our colors:
		negColor = plugLight->baseColors[1];
		posColor = plugLight->baseColors[0];		
		plugLight->baseColors[0] = nvgRGBAf(0,0,0,0);
		plugLight->baseColors[1] = nvgRGBAf(0,0,0,0);		
	}
	void enableLights()
	{
		plugLight->baseColors[1] = negColor;
		plugLight->baseColors[0] = posColor;		
	}
	void setLightColor(NVGcolor color)
	{
		negColor = color;
		posColor = color;
		plugLight->baseColors[0] = color;
		plugLight->baseColors[1] = color;
	}
	void setLightColor(NVGcolor negativeColor, NVGcolor positiveColor)
	{
		negColor = negativeColor;
		posColor = positiveColor;
		plugLight->baseColors[1] = negativeColor;
		plugLight->baseColors[2] = positiveColor;		
	}	
};

struct TS_Panel : Panel 
{
	NVGcolor borderColor = COLOR_BLACK;
	int borderWidth = 0;
	int borderTop = 0;
	int borderLeft = 0;
	int borderRight = 0;
	int borderBottom = 0;
	
	void setBorderWidth(int top, int right, int bottom, int left)
	{
		borderTop = top;
		borderLeft = left;
		borderRight = right;
		borderBottom = bottom;
		return;
	}
	
	void draw(NVGcontext *vg) override
	{
		nvgBeginPath(vg);
		nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);

		// Background color
		if (backgroundColor.a > 0) {
			nvgFillColor(vg, backgroundColor);
			nvgFill(vg);
		}

		// Background image
		if (backgroundImage) {
			int width, height;
			nvgImageSize(vg, backgroundImage->handle, &width, &height);
			NVGpaint paint = nvgImagePattern(vg, 0.0, 0.0, width, height, 0.0, backgroundImage->handle, 1.0);
			nvgFillPaint(vg, paint);
			nvgFill(vg);
		}

		// Border		
		if (borderWidth > 0)
		{
			nvgBeginPath(vg);
			nvgRect(vg, borderWidth / 2.0, borderWidth / 2.0, box.size.x - borderWidth, box.size.y - borderWidth);
			nvgStrokeColor(vg, borderColor);
			nvgStrokeWidth(vg, borderWidth);
			nvgStroke(vg);			
		}
		int x, y;
		
		if (borderTop > 0)
		{
			// Line at top
			nvgBeginPath(vg);
			x = 0;
			y = 0;
			nvgMoveTo(vg, /*start x*/ x, /*start y*/ y); // Top Left
			x = box.size.x;
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Top Right
			nvgStrokeColor(vg, borderColor);
			nvgStrokeWidth(vg, borderTop);
			nvgStroke(vg);				
		}
		if (borderRight > 0)
		{
			nvgBeginPath(vg);
			x = box.size.x;
			y = 0;
			nvgMoveTo(vg, /*x*/ x, /*y*/ y); // Top Right					
			y = box.size.y;// - borderRight;
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Bottom Right								
			nvgStrokeColor(vg, borderColor);
			nvgStrokeWidth(vg, borderRight);
			nvgStroke(vg);							
		}
		if (borderBottom > 0)
		{
			nvgBeginPath(vg);
			x = box.size.x;// - borderBottom;
			y = box.size.y;// - borderBottom;
			nvgMoveTo(vg, /*x*/ x, /*y*/ y); // Bottom Right					
			x = 0;// borderBottom / 2.0;
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Bottom Left			
			nvgStrokeColor(vg, borderColor);
			nvgStrokeWidth(vg, borderBottom);
			nvgStroke(vg);										
		}
		if (borderLeft > 0)
		{
			nvgBeginPath(vg);
			x = 0;//borderLeft / 2.0;
			y = box.size.y;// - borderLeft;
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Bottom Left					
			y = 0;//borderLeft / 2.0;
			nvgLineTo(vg, /*x*/ x, /*y*/ y); // Top Left						
			nvgStrokeColor(vg, borderColor);
			nvgStrokeWidth(vg, borderLeft);
			nvgStroke(vg);													
		}


		Widget::draw(vg);
	}
};

struct ModuleResizeHandle : Widget {
	float minWidth;
	bool right = false;
	float dragX;
	Rect originalBox;
	ModuleResizeHandle(float _minWidth) {
		box.size = Vec(RACK_GRID_WIDTH * 1, RACK_GRID_HEIGHT);
		minWidth = _minWidth;
	}
	void onMouseDown(EventMouseDown &e) override {
		if (e.button == 0) {
			e.consumed = true;
			e.target = this;
		}
	}
	void onDragStart(EventDragStart &e) override {
		dragX = gRackWidget->lastMousePos.x;
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();
		originalBox = m->box;
	}
	void onDragMove(EventDragMove &e) override {
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();

		float newDragX = gRackWidget->lastMousePos.x;
		float deltaX = newDragX - dragX;

		Rect newBox = originalBox;
		if (right) {
			newBox.size.x += deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		}
		else {
			newBox.size.x -= deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
			newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
		}
		gRackWidget->requestModuleBox(m, newBox);
	}
};


//:::-:::-:::-:::- Helpers -:::-:::-:::-:::
template <class TModuleLightWidget>
ColorValueLight * TS_createColorValueLight(Vec pos,  Module *module, int lightId, Vec size, NVGcolor lightColor) {
	ColorValueLight *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = lightId;	
	//light->value = value;
	light->box.size = size;
	light->setColor(lightColor);
	//light->baseColor = lightColor;
	return light;
}
template <class TModuleLightWidget>
ColorValueLight * TS_createColorValueLight(Vec pos, Module *module, int lightId, Vec size, NVGcolor lightColor, NVGcolor backColor) {
	ColorValueLight *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = lightId;	
	//light->value = value;
	light->box.size = size;
	//light->baseColor = lightColor;
	light->setColor(lightColor);	
	light->bgColor = backColor;
	return light;
}

template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	port->disableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId, NVGcolor lightColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	port->setLightColor(lightColor);
	return port;
}
template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId, NVGcolor negColor, NVGcolor posColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	port->setLightColor(negColor, posColor);
	return port;
}
template <class TPort>
TS_Port* TS_createInput(Vec pos, Module *module, int inputId, bool disableLight) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::INPUT;
	port->portId = inputId;
	if (disableLight)
		port->disableLights();
	return port;
}


template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = inputId;
	port->disableLights();
	return port;
}
template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId, NVGcolor lightColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = inputId;
	port->setLightColor(lightColor);
	return port;
}
template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId, NVGcolor negColor, NVGcolor posColor) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = inputId;
	port->setLightColor(negColor, posColor);
	return port;
}
template <class TPort>
TS_Port* TS_createOutput(Vec pos, Module *module, int inputId, bool disableLight) {
	TS_Port *port = new TPort();
	port->box.pos = pos;
	port->module = module;
	port->type = Port::OUTPUT;
	port->portId = inputId;
	if (disableLight)
		port->disableLights();
	return port;
}



#endif // end if not defined
