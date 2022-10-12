#ifndef TROWASOFT_CLIGHTS_HPP
#define TROWASOFT_CLIGHTS_HPP
//--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
// Light components broken out into this file.
//--*----*----*----*----*----*----*----*----*----*----*----*----*----*----*--
#include <rack.hpp>
using namespace rack;

#include <string.h>
#include <stdio.h>
#include "TSColors.hpp"
#include "componentlibrary.hpp"
#include "plugin.hpp"
#include "trowaSoftUtilities.hpp"
#include <color.hpp>
#include <nanovg.h>

extern Plugin* pluginInstance;

// LightWidget:
// 	bgColor
// 	color
// 	borderColor
// MultiLightWidget: A MultiLightWidget that points to a module's Light or a range of lights. Will access firstLightId, firstLightId + 1, etc. for each added color.
//  firstLightId
// ModuleLightWidget: A MultiLightWidget that points to a module's Light or a range of lights. Will access firstLightId, firstLightId + 1, etc. for each added color.


//--------------------------------------------------------------
// ColorValueLight - Sorta like the old ColorValueLight that was in Rack.
//--------------------------------------------------------------
struct ColorValueLight : ModuleLightWidget {
	enum LightShape : uint8_t {
		Circular,
		Rectangular
	};
	// The shape of the light.
	LightShape shape = LightShape::Circular;
	// Base Color (raw color with original alpha).
	NVGcolor baseColor;
	// Pixels to add for outer radius (either px or relative %).
	float outerRadiusHalo = 0.10; // 0.35
	// If the outer radius is relative or absolute units.
	bool outerRadiusRelative = true;	
	// For rectangles, the corner radius of the rectangle.
	float cornerRadius = 3.0;	
	// For rectangles, feather defines how blurry the border of the rectangle is.
	float feather = 3.0;
	// Border width.
	float borderWidth = 0.5f;
	
	ColorValueLight() : ModuleLightWidget()
	{
		bgColor = nvgRGB(0x20, 0x20, 0x20);
		borderColor = nvgRGBA(0, 0, 0, 0);
		return;
	};
	virtual ~ColorValueLight(){};
	// Set a single color
	void setColor(NVGcolor bColor)
	{
		color = bColor;
		baseColor = bColor;
		if (baseColors.size() < 1)
		{
			baseColors.push_back(bColor);			
		}
		else
		{
			baseColors[0] = bColor;
		}
		return;
	}
	
	//------------------------------------------------	
	// Draw the background based off the shape.
	//------------------------------------------------	
	void drawBackground(const DrawArgs& args) override 
	{		
		nvgBeginPath(args.vg);
		if (shape == LightShape::Circular)
		{		
			// Circle
			Vec radius = Vec(box.size.x / 2.0, box.size.y / 2.0);
			nvgCircle(args.vg, radius.x, radius.y, std::min(radius.x, radius.y));
		}
		else
		{
			nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, cornerRadius);
		}
		// Background
		if (bgColor.a > 0.0) {
			nvgFillColor(args.vg, bgColor);
			nvgFill(args.vg);
		}		
		// Border
		if (borderColor.a > 0.0 && borderWidth > 0.0f) {
			nvgStrokeWidth(args.vg, borderWidth);
			nvgStrokeColor(args.vg, borderColor);
			nvgStroke(args.vg);
		}		
		return;
	}

	
	//------------------------------------------------	
	// Draw the foreground based off the shape.
	//------------------------------------------------	
	void drawLight(const DrawArgs &args) override
	{
		// Foreground
		if (color.a > 0.0) {
			nvgBeginPath(args.vg);			
			if (shape == LightShape::Circular)
			{				
				// Circle
				Vec radius = Vec(box.size.x / 2.0, box.size.y / 2.0);				
				nvgCircle(args.vg, radius.x, radius.y, std::min(radius.x, radius.y));
			}
			else
			{
				// Creates new rounded rectangle shaped sub-path.
				// void nvgRoundedRect(NVGcontext* ctx, float x, float y, float w, float h, float r);				
				nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, cornerRadius);				
			}
			// // Don't want full opacity
			// NVGcolor c = color;
			// //c.a *= 0.5f;			
			// nvgFillColor(args.vg, c);
			// nvgFill(args.vg);
			
			// Inner glow
			nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
			nvgFillColor(args.vg, color);
			nvgFill(args.vg);
			
		}		
		return;
	}
	//------------------------------------------------	
	// Draw the halo based off the shape.
	//------------------------------------------------
	void drawHalo(const DrawArgs &args) override
	{
		NVGpaint paint;
		NVGcolor icol = color::mult(color, 0.10);//colorMult(color, 0.10);
		NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
		Vec innerRadius = Vec(box.size.x / 2.0, box.size.y / 2.0);
		Vec outerRadius = innerRadius; 
		Vec delta;
		if (outerRadiusRelative)
		{
			delta = Vec((innerRadius.x*outerRadiusHalo), (innerRadius.y*outerRadiusHalo));
		}
		else
		{
			delta = Vec(outerRadiusHalo, outerRadiusHalo);
		}
		outerRadius.x += delta.x;
		outerRadius.y += delta.y;
		
		nvgBeginPath(args.vg);
		if (shape == LightShape::Circular)
		{
			float rStart = std::min(innerRadius.x, innerRadius.y); // Min of the two
			float rEnd = std::max(outerRadius.x, outerRadius.y); // Max of the two
			nvgCircle(args.vg, innerRadius.x, innerRadius.y, rEnd);			
			paint = nvgRadialGradient(args.vg, innerRadius.x, innerRadius.y, rStart, rEnd, icol, ocol);
		}
		else
		{
			// Feather defines how blurry the border of the rectangle is.
			nvgRoundedRect(args.vg, -delta.x/2.0, -delta.y/2.0, 2 * outerRadius.x, 2 * outerRadius.y, cornerRadius);			
			paint = nvgBoxGradient(args.vg, /*x*/ 0, /*y*/ 0, /*w*/ outerRadius.x * 2, /*h*/ outerRadius.y * 2, 
				/*r: corner radius*/ cornerRadius, /*f: feather*/ feather, 
				/*inner color*/ icol, /*outer color */ ocol);				
		}
		
		nvgFillPaint(args.vg, paint);
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgFill(args.vg);		
		return;
	}
};

//------------------------------------------------------------------------------------------------
// TS_LightArc - Lighted arc for light knobs
//------------------------------------------------------------------------------------------------
struct TS_LightArc : ColorValueLight {
	// The inner radius 
	float innerRadius = 22;	
	// Arc thickness (was 3.0f)
	float arcThickness = 3.0f;
	// Pointer to current angle in radians. This is the differential like from a knob.
	float* currentAngle_radians;
	// Font size for our display numbers
	int fontSize;	
	// Font face
	//std::shared_ptr<Font> font;
	std::string fontPath; // Rack v2 store font path

	// Text color
	NVGcolor textColor = TSColors::COLOR_WHITE;
	// Numeric value to print out
	//float* numericValue;
	//ParamQuantity* pValue;
	ParamWidget* paramWidget = NULL;
	
	// Buffer for our light string.
	char lightString[10];
	// The point where the angle is considered 0 degrees / radians.
	float zeroAnglePoint;
	// Pointer to the Sequencer Value mode information.
	ValueSequencerMode* valueMode;
	
	TS_LightArc()
	{
		fontPath = asset::plugin(pluginInstance, TROWA_LABEL_FONT); // Rack v2 store font path
		fontSize = 10;
		bgColor = nvgRGBAf(0.0, 0, 0, /*alpha */ 1.0);
		borderColor = nvgRGBAf(0.1, 0.1, 0.1, /*alpha */ 1.0);
		textColor = TSColors::COLOR_WHITE;
		zeroAnglePoint = TROWA_ANGLE_STRAIGHT_UP_RADIANS;
		shape = LightShape::Circular;
	}	
	
	//------------------------------------------------	
	// Draw the arc.
	//------------------------------------------------		
	void drawValueArc(const DrawArgs &args, bool drawLight)
	{
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath); // Rack v2 load font each time

		float oradius = box.size.x / 2.0; // 25
		//float radius = oradius - arcThickness; // 23
		
		float angle = *currentAngle_radians;
		zeroAnglePoint = valueMode->zeroPointAngle_radians;
		// svg Angles go clockwise from positive x -->		
		int dir = (angle < zeroAnglePoint) ? NVG_CCW : NVG_CW;
		
		nvgBeginPath(args.vg);
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		
		//-- Actual Value Arc Indicator --
		// nvgArc(NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int dir);
		nvgArc(args.vg, /*cx*/ oradius, /*cy*/ oradius, /*radius*/ innerRadius, 
			/*a0*/ zeroAnglePoint, /*a1*/ angle, /*dir*/ dir);			
		// Stroke the value arc.
		nvgStrokeWidth(args.vg, oradius - innerRadius);
		nvgStrokeColor(args.vg, baseColor); // Original Raw Color 		
		// if (drawLight)
			// nvgStrokeColor(args.vg, color);
		// else
			// nvgStrokeColor(args.vg, baseColor); // Original Raw Color 
		nvgStroke(args.vg);
		
		if (drawLight)
		{
			// Outer glow (outer)
			nvgBeginPath(args.vg);
			nvgArc(args.vg, /*cx*/ oradius, /*cy*/ oradius, oradius, /*a0*/ zeroAnglePoint, /*a1*/ angle, /*dir*/ dir);	
			NVGpaint paint;
			NVGcolor icol = color;
			icol.a *= 0.8;		
			NVGcolor ocol = color;
			ocol.a = 0.0;
			paint = nvgRadialGradient(args.vg, oradius, oradius, innerRadius, oradius, icol, ocol);
			nvgStrokeWidth(args.vg, 2);	
			nvgStrokePaint(args.vg, paint);
			nvgStroke(args.vg);			

			// Put the text on the Light Layer so it still shows in the dark:
			if (paramWidget != NULL && paramWidget->getParamQuantity())
			{
				nvgBeginPath(args.vg);
				nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);//Restore to default.
				nvgFontSize(args.vg, fontSize); 	
				nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
				float v = valueMode->GetOutputValue(paramWidget->getParamQuantity()->getValue()); //(*numericValue);
				valueMode->GetDisplayString(v, lightString);
				nvgFillColor(args.vg, textColor);
				nvgText(args.vg, oradius, oradius, lightString, NULL);
			}	
		
		}
		
		// // Outer glow
		// nvgBeginPath(args.vg);
		// nvgArc(args.vg, /*cx*/ oradius, /*cy*/ oradius, innerRadius - 3, 
			 // /*a0*/ zeroAnglePoint, /*a1*/ angle, /*dir*/ dir);	
		// NVGpaint paint;
		// NVGcolor icol = color;
		// icol.a *= 0.8;
		// NVGcolor ocol = color;
		// ocol.a = 0.0;
		// paint = nvgRadialGradient(args.vg, oradius, oradius, innerRadius, oradius, icol, ocol);
		// nvgStrokeWidth(args.vg, oradius - innerRadius + 3);	
		// nvgStrokePaint(args.vg, paint);
		// nvgStroke(args.vg);
		
	
		return;
	}
	
	
	//------------------------------------------------	
	// Draw the background.
	//------------------------------------------------	
	void drawBackground(const DrawArgs& args) override 
	{
		nvgBeginPath(args.vg);
		// Circle
		//Vec radius = Vec(box.size.x / 2.0, box.size.y / 2.0);
		float oradius = box.size.x / 2.0; // 25
		float radius = oradius - arcThickness; // 23		
		nvgCircle(args.vg, oradius, oradius, innerRadius);
		
		// Background
		if (bgColor.a > 0.0) {
			nvgFillColor(args.vg, bgColor);
			nvgFill(args.vg);
		}
		// Border
		if (borderColor.a > 0.0) {
			nvgStrokeWidth(args.vg, radius - innerRadius);
			NVGcolor bColor = borderColor;
			bColor.a *= 0.5; // Border should be a little lighter
			nvgStrokeColor(args.vg, bColor);
			nvgStroke(args.vg);
		}
		else {
			// Use the baseColor as the border but lighter
			nvgStrokeWidth(args.vg, radius - innerRadius);
			NVGcolor bColor = baseColor;
			bColor.a *= 0.5; // Border should be a little lighter
			nvgStrokeColor(args.vg, bColor);
			nvgStroke(args.vg);			
		}
		
		// Draw the base value bar:
		drawValueArc(args, false);
		
		return;
	}
	
	//------------------------------------------------	
	// Draw the foreground based off the shape.
	//------------------------------------------------	
	void drawLight(const DrawArgs &args) override
	{
		float oradius = box.size.x / 2.0; // 25
		float radius = oradius - arcThickness; // 23

		// Foreground
		nvgBeginPath(args.vg);			
				
		// Inner glow
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgCircle(args.vg, oradius, oradius, radius);		
		//NVGcolor bColor = color;
		//bColor.a = 0.25;
		//nvgStrokeWidth(args.vg, oradius - radius);
		//nvgStrokeColor(args.vg, bColor);
		//nvgStroke(args.vg);
		nvgFillColor(args.vg, color);
		nvgFill(args.vg);
		
		// // Inner glow
		// nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		// nvgFillColor(args.vg, color);
		// nvgFill(args.vg);	

		// Draw the arc.
		drawValueArc(args, true);		
		return;
	}	
	
	
	// void draw(const DrawArgs &args) override
	// {
		// float oradius = box.size.x / 2.0; // 25
		// float radius = oradius - arcThickness; // 23
		
		// float angle = *currentAngle_radians;
		// zeroAnglePoint = valueMode->zeroPointAngle_radians;
		// int dir = (angle < zeroAnglePoint) ? NVG_CCW : NVG_CW;

		// // Background - Solid
		// nvgBeginPath(args.vg);
		// nvgCircle(args.vg, oradius, oradius, innerRadius);
		// nvgFillColor(args.vg, bgColor);
		// nvgFill(args.vg);
		
		// nvgStrokeWidth(args.vg, radius - innerRadius);
		// NVGcolor borderColor = color;// bgColor;
		// borderColor.a *= 0.5;//1.0;
		// nvgStrokeColor(args.vg, borderColor);
		// nvgStroke(args.vg);
		
		// // svg Angles go clockwise from positive x -->
		
		// // Inner glow
		// nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		// nvgCircle(args.vg, oradius, oradius, radius);		
		// borderColor = color;
		// borderColor.a = 0.25;
		// nvgStrokeWidth(args.vg, oradius - radius);
		// nvgStrokeColor(args.vg, borderColor);
		// nvgStroke(args.vg);
		
		
		// nvgBeginPath(args.vg);
		// //nvgArcTo(args.vg, oradius, oradius, float x2, float y2, float radius);
		// // Creates new circle arc shaped sub-path. The arc center is at cx,cy, the arc radius is r,
		// // and the arc is drawn from angle a0 to a1, and swept in direction dir (NVG_CCW, or NVG_CW).
		// // Angles are specified in radians.
		// // nvgArc(NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int dir);
		// nvgArc(args.vg, /*cx*/ oradius, /*cy*/ oradius, /*radius*/ innerRadius, 
			// /*a0*/ zeroAnglePoint, /*a1*/ angle, /*dir*/ dir);
		// nvgStrokeWidth(args.vg, oradius - innerRadius);
		// borderColor = baseColor;
		// borderColor.a *= 0.7;
		// nvgStrokeColor(args.vg, borderColor);
		// nvgStroke(args.vg);
		
		// // Outer glow
		// nvgBeginPath(args.vg);
		// nvgArc(args.vg, /*cx*/ oradius, /*cy*/ oradius, innerRadius - 3, 
			 // /*a0*/ zeroAnglePoint, /*a1*/ angle, /*dir*/ dir);
	
		// NVGpaint paint;
		// NVGcolor icol = color;
		// icol.a *= 0.8;
		// NVGcolor ocol = color;
		// ocol.a = 0.0;
		// paint = nvgRadialGradient(args.vg, oradius, oradius, innerRadius, oradius, icol, ocol);
		// nvgStrokeWidth(args.vg, oradius - innerRadius + 3);	
		// nvgStrokePaint(args.vg, paint);
		// nvgStroke(args.vg);
					
		// //if (numericValue != NULL)
		// //if (pValue != NULL)
		// if (paramWidget != NULL && paramWidget->getParamQuantity())
		// {
			// nvgBeginPath(args.vg);
			// nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);//Restore to default.
			// nvgFontSize(args.vg, fontSize); 	
			// nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			// float v = valueMode->GetOutputValue(paramWidget->getParamQuantity()->getValue()); //(*numericValue);
			// valueMode->GetDisplayString(v, lightString);
			// nvgFillColor(args.vg, textColor);
			// nvgText(args.vg, oradius, oradius, lightString, NULL);
		// }
		// return;
	// }
}; // end TS_LightArc


//--------------------------------------------------------------
// TS_LightString - A light with a string (message/text).
//--------------------------------------------------------------
struct TS_LightString : ColorValueLight
{
	const char * lightString;
	// Font size 
	int fontSize;	
	// Font face
	//std::shared_ptr<Font> font;
	std::string fontPath; // Rack v2 store font path
	// Text color
	NVGcolor textColor = TSColors::COLOR_WHITE;
	float alphaAdjustment = -1;
	TS_LightString()
	{	
		fontPath = asset::plugin(pluginInstance, TROWA_LABEL_FONT); // Rack v2 store font path
		fontSize = 14;
		bgColor = nvgRGBAf(0.1, 0.1, 0.1, /*alpha */ 1);
		borderColor = nvgRGBAf(0.3, 0.3, 0.3, /*alpha */ 1);
		textColor = TSColors::COLOR_WHITE;
		shape = LightShape::Rectangular;
		borderWidth = 1.0f;
		return;
	}
	
	void drawLight(const DrawArgs &args) override
	{
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath); // Rack v2 load font each time
		
		// Give some transparency to the color a little ...
		NVGcolor c = color;
		color.a *= 0.85;
		ColorValueLight::drawLight(args);
		color = c;
		
		if (lightString != NULL)
		{
			nvgBeginPath(args.vg);
			nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);//Restore to default.			
			nvgFillColor(args.vg, textColor);
			nvgFontSize(args.vg, fontSize);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			nvgText(args.vg, box.size.x / 2, box.size.y / 2, lightString, NULL);
		} // end if there is a text string to render.
		return;
	}
	
	
	// void draw(const DrawArgs &args) override
	// {
		// ColorValueLight::draw(args);
		// // if (lightString != NULL)
		// // {
			// // nvgBeginPath(args.vg);
			// // nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);//Restore to default.
			// // NVGcolor textColor = baseColor;
			// // nvgFillColor(args.vg, textColor);
			// // nvgFontSize(args.vg, fontSize);
			// // nvgFontFaceId(args.vg, font->handle);
			// // nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			// // nvgText(args.vg, box.size.x / 2, box.size.y / 2, lightString, NULL);
		// // } // end if there is a text string to render.
		// return;
	// }
	
	// void draw(const DrawArgs &args) override
	// {
		// float radius = box.size.x / 2.0;
		// float oradius = radius + 20.0;
		// float radiusY = box.size.y / 2.0;
		// float oradiusY = radiusY + 20.0;

		// NVGcolor outerColor = color;
		
		// // Solid
		// nvgBeginPath(args.vg);
		// // Border
		// nvgStrokeWidth(args.vg, 1.0);
		// NVGcolor borderColor = bgColor;
		// borderColor.a *= 0.5;
		// nvgStrokeColor(args.vg, borderColor);
		// nvgStroke(args.vg);
		
		// // Inner glow
		// nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		// nvgFillColor(args.vg, color);
		// nvgFill(args.vg);

		// // Outer glow
		// nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, /*x*/ radius - oradius, /*y*/ radiusY - oradiusY, /*w*/ 3*oradius, /*h*/ 2*oradiusY, cornerRadius);
		// NVGpaint paint;
		// NVGcolor icol = outerColor;// color;
		// icol.a *= 0.5;
		// NVGcolor ocol = outerColor;// color;
		// ocol.a = 0.0;
		// //float feather = 3;
		// // Feather defines how blurry the border of the rectangle is.
		// paint = nvgBoxGradient(args.vg, /*x*/ 0, /*y*/ 0, /*w*/ box.size.x, /*h*/ oradiusY - 10, 
			// /*r: corner radius*/ cornerRadius, /*f: feather*/ feather, 
			// /*inner color*/ icol, /*outer color */ ocol);
		// nvgFillPaint(args.vg, paint);
		// nvgFill(args.vg);
				
		// nvgBeginPath(args.vg);
		// nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);//Restore to default.
		// NVGcolor textColor = baseColor;
		// nvgFillColor(args.vg, textColor);
		// nvgFontSize(args.vg, fontSize);
		// nvgFontFaceId(args.vg, font->handle);
		// nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		// if (lightString != NULL)
			// nvgText(args.vg, box.size.x / 2, box.size.y / 2, lightString, NULL);
		// return;
	// }	
}; // end TS_LightString

//--------------------------------------------------------------
// TS_LightSquare - Square light. 
//--------------------------------------------------------------
struct TS_LightSquare : ColorValueLight 
{
	// Radius on corners
	//float cornerRadius = 5.0;
	TS_LightSquare()
	{
		bgColor = nvgRGBAf(0, 0, 0, /*alpha */ 0.5);
		//baseColor = TSColors::COLOR_WHITE;
		this->shape = LightShape::Rectangular;
		this->cornerRadius = 3.0;
	}
	// void draw(const DrawArgs &args) override
	// {
		// float radius = box.size.x / 2.0;
		// float oradius = radius*1.1;

		// NVGcolor backColor = bgColor;
		// NVGcolor outerColor = color;
		// // Solid
		// nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		// nvgFillColor(args.vg, backColor);
		// nvgFill(args.vg);

		// // Border
		// nvgStrokeWidth(args.vg, 1.0);
		// NVGcolor borderColor = bgColor;
		// borderColor.a *= 0.5;
		// nvgStrokeColor(args.vg, borderColor);
		// nvgStroke(args.vg);

		// // Inner glow
		// nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		// nvgFillColor(args.vg, color);
		// nvgFill(args.vg);

		// // Outer glow
		// nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, /*x*/ radius - oradius, /*y*/ radius - oradius, /*w*/ 2*oradius, /*h*/ 2*oradius, cornerRadius);
		// NVGpaint paint;
		// NVGcolor icol = outerColor;// color;
		// icol.a *= 0.25;
		// NVGcolor ocol = outerColor;// color;
		// ocol.a = 0.0;
		// float feather = 2;
		// // Feather defines how blurry the border of the rectangle is. // Fixed 01/19/2018, made it too tiny before
		// paint = nvgBoxGradient(args.vg, /*x*/ radius - oradius, /*y*/ radius - oradius, /*w*/ 2 * oradius, /*h*/ 2 * oradius,  //args.vg, /*x*/ -5, /*y*/ -5, /*w*/ 2*oradius + 10, /*h*/ 2*oradius + 10, 
			// /*r: corner radius*/ cornerRadius, /*f: feather*/ feather, 
			// /*inner color*/ icol, /*outer color */ ocol);
		// nvgFillPaint(args.vg, paint);
		// nvgFill(args.vg);
		// return;
	// }
}; // end TS_LightSquare

//--------------------------------------------------------------
// TS_LightRectangle - Square light. 
//--------------------------------------------------------------
struct TS_LightRectangle : ColorValueLight 
{
	// Radius on corners
	float cornerRadius = 2.0;
	TS_LightRectangle()
	{
		bgColor = nvgRGBAf(0, 0, 0, /*alpha */ 0.5);
		//baseColor = TSColors::COLOR_WHITE;
		this->shape = LightShape::Rectangular;		
	}
	// void draw(const DrawArgs &args) override
	// {
		// Vec radius = Vec(box.size.x / 2.0f, box.size.y / 2.0f);
		// Vec oradius = Vec(radius.x * 1.1, radius.y * 1.1);
		
		// NVGcolor backColor = bgColor;
		// NVGcolor outerColor = color;
		// // Solid
		// nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		// nvgFillColor(args.vg, backColor);
		// nvgFill(args.vg);

		// // Border
		// nvgStrokeWidth(args.vg, 1.0);
		// NVGcolor borderColor = bgColor;
		// borderColor.a *= 0.5;
		// nvgStrokeColor(args.vg, borderColor);
		// nvgStroke(args.vg);

		// // Inner glow
		// nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		// nvgFillColor(args.vg, color);
		// nvgFill(args.vg);

		// // Outer glow
		// nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, /*x*/ radius.x - oradius.x, /*y*/ radius.y - oradius.y, /*w*/ 2*oradius.x, /*h*/ 2*oradius.y, cornerRadius);
		// NVGpaint paint;
		// NVGcolor icol = outerColor;// color;
		// icol.a *= 0.25;
		// NVGcolor ocol = outerColor;// color;
		// ocol.a = 0.0;
		// float feather = 2;
		// // Feather defines how blurry the border of the rectangle is. 
		// paint = nvgBoxGradient(args.vg, /*x*/ radius.x - oradius.y, /*y*/ radius.y - oradius.y, /*w*/ 2 * oradius.x, /*h*/ 2 * oradius.y,   
			// /*r: corner radius*/ cornerRadius, /*f: feather*/ feather, 
			// /*inner color*/ icol, /*outer color */ ocol);
		// nvgFillPaint(args.vg, paint);
		// nvgFill(args.vg);
		// return;
	// }
}; // end TS_LightRectangle

//--------------------------------------------------------------
// TS_LightRing - Light to be used around ports.
//--------------------------------------------------------------
struct TS_LightRing : ColorValueLight 
{
	// The inner radius 
	float innerRadius = 6.8;
	
	TS_LightRing()
	{
		bgColor = nvgRGBAf(0, 0, 0, /*alpha */ 0.2);
		//baseColor = TSColors::COLOR_WHITE;
	}
	void draw(const DrawArgs &args) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius + 10.0;
		

		// Solid
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, radius, radius, radius);

		// Border
		nvgStrokeWidth(args.vg, radius - innerRadius);
		NVGcolor borderColor = bgColor;
		borderColor.a *= 1.0;
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);

		// Inner glow
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		//nvgFillColor(args.vg, color);
		//nvgFill(args.vg);

		// Outer glow
		nvgBeginPath(args.vg);
		nvgRect(args.vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);
		NVGpaint paint;
		NVGcolor icol = color;
		icol.a *= (module != NULL) ? module->lights[firstLightId].value : 0;
		//icol.a *= value;
		NVGcolor ocol = color;
		ocol.a = 0.0;
		paint = nvgRadialGradient(args.vg, radius, radius, innerRadius, oradius, icol, ocol);
		nvgFillPaint(args.vg, paint);
		nvgFill(args.vg);
		return;
	}
};

//--------------------------------------------------------------
// TS_LightMeter - Square light meter. 
//--------------------------------------------------------------
struct TS_LightMeter : ColorValueLight 
{
	float meterValue = 0.0f;
	ParamWidget* paramWidget = NULL;
	ValueSequencerMode* valueMode = NULL;
	
	float innerGlowAlphaAdj = 1.0f;	
	bool useSeparateValueColor = false;
	NVGcolor valueColor;
	
	int id = 0;
	TS_LightMeter()
	{
		bgColor = nvgRGBAf(0, 0, 0, /*alpha */ 0.5);
		//baseColor = TSColors::COLOR_WHITE;
		shape = LightShape::Rectangular;
		cornerRadius = 3.0f;
		outerRadiusHalo = 0.15f;
	}
	
	// To use a different color to represent the value than the light flashing...
	void setValueColor(NVGcolor valColor, bool diffColorEnabled = true)
	{
		useSeparateValueColor = diffColorEnabled;
		valueColor = valColor;
	}
	
	//------------------------------------------------	
	// Draw the background.
	//------------------------------------------------	
	void drawBackground(const DrawArgs& args) override 
	{
		if (!visible)
			return;
		
		this->ColorValueLight::drawBackground(args);
		return;
	}
	
	void drawValueIndicator(const DrawArgs& args)
	{
		
		// Light =========================		
		float height = 0.0f;
		float y = 0.0f;
		meterValue = 0.0f;		
		if (paramWidget != NULL && paramWidget->getParamQuantity())
		{
			float v = valueMode->GetOutputValue(paramWidget->getParamQuantity()->getValue()); //(*numericValue);
			float min = valueMode->outputVoltageMin;//minDisplayValue;
			float max = valueMode->outputVoltageMax;//maxDisplayValue;
			if (valueMode->isBoolean)
			{
				meterValue = v > 0;
				height = (v > 0) ? box.size.y : 0;
				y = 0;
			}
			else
			{
				meterValue = rescale(clamp(v, min, max), min, max, 0.0f, 1.0f);
				height = meterValue * box.size.y;
				//halo = height / 2.0f * 0.1;
					y = box.size.y - height;				
			}
		}		
		
		if (meterValue > 0)
		{
			NVGcolor lightColor = color;
			if (useSeparateValueColor)
			{
				lightColor = valueColor;
			}
			else
			{	
				float v = (module != NULL) ? module->lights[firstLightId].value : 0.0f;
				// Show the value indicator light but if the light is active (i.e. current step), then reduce the brightness
				// so that the light of the current step can show through.
				lightColor.r *= 0.9f;
				lightColor.g *= 0.9f;
				lightColor.b *= 0.9f;
				lightColor.a = 1.0f - v;				
			}
			
			if (lightColor.a > 0)
			{
				// Value Indication		
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 0.0, y, box.size.x, height, cornerRadius);		
				//NVGcolor valueColor = color;
				//lightColor.a = 1.0f; // Full Opacity
				//nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
				nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
				nvgFillColor(args.vg, lightColor);
				nvgFill(args.vg);	
			}
		}		
		return;
	}
	
	//------------------------------------------------	
	// Draw the light.
	//------------------------------------------------	
	void drawLight(const DrawArgs& args) override 
	{
		if (!visible)
			return;
		
		drawValueIndicator(args);
		
		// Draw if we are active
		this->ColorValueLight::drawLight(args);			
		return;
	}
	
	void drawHalo(const DrawArgs &args) override
	{
		if (!visible)
			return;
		
		this->ColorValueLight::drawHalo(args);
		return;
	}
	
	
	// void draw(const DrawArgs &args) override
	// {
		// if (!visible)
			// return;
		// float radius = box.size.x / 2.0;
		// //float oradius = radius*1.1;
		// //float halo = 0.1 * radius;

		// NVGcolor backColor = bgColor;
		// NVGcolor outerColor = color;
		
		// // Background =========================
		// // Solid
		// nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		// nvgFillColor(args.vg, backColor);
		// nvgFill(args.vg);

		// // Border
		// nvgStrokeWidth(args.vg, 1.0);
		// NVGcolor borderColor = bgColor;
		// borderColor.a *= 0.5;
		// nvgStrokeColor(args.vg, borderColor);
		// nvgStroke(args.vg);
		
		// // Light =========================		
		// float height = box.size.y;
		// float y = 0.0f;
		// meterValue = 1.0f;		
		// if (paramWidget != NULL && paramWidget->getParamQuantity())
		// {
			// float v = valueMode->GetOutputValue(paramWidget->getParamQuantity()->getValue()); //(*numericValue);
			// float min = valueMode->outputVoltageMin;//minDisplayValue;
			// float max = valueMode->outputVoltageMax;//maxDisplayValue;
			// if (valueMode->isBoolean)
			// {
				// meterValue = v > 0;
				// height = (v > 0) ? box.size.y : 0;
				// y = 0;
			// }
			// else
			// {
				// meterValue = rescale(clamp(v, min, max), min, max, 0.0f, 1.0f);
				// height = meterValue * box.size.y;
				// //halo = height / 2.0f * 0.1;
				// y = box.size.y - height;				
			// }
		// }		
		
		// NVGcolor lightColor = color;
		// if (!valueMode->isBoolean || meterValue > 0)
		// {
			// // Value Indication		
			// nvgBeginPath(args.vg);
			// nvgRoundedRect(args.vg, 0.0, y, box.size.x, height, cornerRadius);		
			// NVGcolor valueColor = color;
			// valueColor.a = 1.0f;
			// nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);		
			// nvgFillColor(args.vg, valueColor);
			// nvgFill(args.vg);
			// lightColor.a *= innerGlowAlphaAdj;
		// }
		
		// // Inner glow
		// nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, 0.0, 0, box.size.x, box.size.y, cornerRadius);		
		// nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		// nvgFillColor(args.vg, lightColor);
		// nvgFill(args.vg);
		
		
		// float offset = radius*0.1;
		// nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, -offset, y - offset, box.size.x + offset, height + offset, cornerRadius);				
		// NVGpaint paint;
		// NVGcolor icol = outerColor;// color;
		// icol.a *= 0.25;
		// NVGcolor ocol = outerColor;// color;
		// ocol.a = 0.0;
		// float feather = 2;
		// paint = nvgBoxGradient(args.vg, -offset, y - offset, box.size.x + offset, height + offset,   
			// /*r: corner radius*/ cornerRadius, /*f: feather*/ feather, 
			// /*inner color*/ icol, /*outer color */ ocol);
		// nvgFillPaint(args.vg, paint);
		// nvgFill(args.vg);

		// return;
	// }
	
}; // end TS_LightMeter

/////////////////////////////////////////////
//:::-:::-:::-:::- Helpers -:::-:::-:::-::://
/////////////////////////////////////////////
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
	light->box.size = size;
	light->setColor(lightColor);	
	light->bgColor = backColor;
	return light;
}

#endif // !TROWASOFT_CLIGHTS_HPP