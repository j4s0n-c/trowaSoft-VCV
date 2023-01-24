#include "Widget_polyGen.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSModuleWidgetBase.hpp"
using namespace rack;
#include "trowaSoft.hpp"

#include "Module_polyGen.hpp"

#define PG_WIDGET_X_SPACING		60.0f
#define PG_WIDGET_Y_SPACING		44.f // was 42.0f, now 44.0f
#define PG_WIDGET_TOP_SPACE		5.0f
#define PG_WIDGET_COLUMN_OFFSET	105.0f // was 120 , then 125, now 115

#define PG_HEADING_MAIN_SHAPE	"Shape" // was "Main Shape"
#define PG_HEADING_STAR_SHAPE	"Star" // was "Star Shape"


struct polyGenLabelWidget : TransparentWidget
{
	int fontSize = 10;
	//std::shared_ptr<Font> font;
	NVGcolor borderColor = nvgRGB(0xee, 0xee, 0xee);
	NVGcolor groupTxtColor = TSColors::COLOR_BLACK;// nvgRGB(0x33, 0x33, 0x33);

	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);

	bool drawBoxes = false;
	enum HeadingLocation {
		Top,   // Heading will be on LHS (make like mulitScope)
		Right, // Heading will be centered vertically and on the right
		Left,  // Heading will be centered vertically and on the left. Not tested recently.
		Bottom // Not implemented/tested
	};

	HeadingLocation headingLocation = HeadingLocation::Top;
	
	polyGenLabelWidget()
	{
		fontSize = 10;
		
		return;
	}
	polyGenLabelWidget(Widget* parent) : polyGenLabelWidget()
	{
		box.size = parent->box.size;
		return;
	}
	
	// Simplified draw shape
	void drawShape(const DrawArgs &args, int numVertices, float xAmpl, float yAmpl, 
		float innerRadiusMult, float innerAngleMult, 
		float angleOffset_rad, float rotation_rad,
		Vec offset, bool fill = false)
	{

		Vec rotCenter = Vec(0, 0);
		yAmpl = -yAmpl; // invert Y
		Vec innerAmpl = Vec(xAmpl * innerRadiusMult, yAmpl * innerRadiusMult);
		bool useInnerVerts = (innerRadiusMult > 1.05 || innerRadiusMult < 0.95);
		//============================
		// Calculate the Buffers
		//============================
		//Vec nextPoint;
		Vec firstPoint;
		float sinrot = SINFUNC( rotation_rad );
		float cosrot = COSFUNC( rotation_rad );			
		float n = static_cast<float>(numVertices);
		float innerAngleShift = 0.5f * (1 + innerAngleMult) / n;
		nvgBeginPath(args.vg);		
		for (int v = 0; v < numVertices; v++)
		{
			Vec point;
			float pAngle = static_cast<float>(v) / n;		
			point.x = xAmpl * std::sin(pAngle * 2 * PI + angleOffset_rad) ;
			point.y = yAmpl * std::cos(pAngle * 2 * PI + angleOffset_rad) ;
			
			float x, y;
			x = (point.x - rotCenter.x) * cosrot + (point.y - rotCenter.y) * sinrot + rotCenter.x;
			y = (-point.x - rotCenter.y) * sinrot + (point.y - rotCenter.y) * cosrot + rotCenter.y;

			x += offset.x;
			y += offset.y;
			
			if ( v > 0)
			{
				nvgLineTo(args.vg, x, y);
			}
			else
			{
				nvgMoveTo(args.vg, x, y);				
				firstPoint.x = x;
				firstPoint.y = y;
			}
			
			if (useInnerVerts)
			{
				//------------------------
				// Calculate 2ndary point
				//------------------------
				pAngle += innerAngleShift;
				point.x = innerAmpl.x * std::sin(pAngle * 2 * PI + angleOffset_rad);
				point.y = innerAmpl.y * std::cos(pAngle * 2 * PI + angleOffset_rad);
				x = (point.x - rotCenter.x) * cosrot + (point.y - rotCenter.y) * sinrot + rotCenter.x;
				y = (-point.x - rotCenter.y) * sinrot + (point.y - rotCenter.y) * cosrot + rotCenter.y;				
				x += offset.x;
				y += offset.y;
				nvgLineTo(args.vg, x, y);				
			} // end if useInnerVerts
		} // end loop through vertices
		nvgLineTo(args.vg, firstPoint.x, firstPoint.y);
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
		
		if (fill)
		{
			nvgFillColor(args.vg, borderColor);
			nvgFill(args.vg);
		}

		return;
	}		
		
	void draw(/*in*/ const DrawArgs &args) override
	{
		const float addX = 7.5f; // add left padding now that I expanded the total widget size by 1 U
		std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_LABEL_FONT));
		//const float padding = 15.0f;
		fontSize = 10;		
		const float borderRadius = 4.f;
		float dx = PG_WIDGET_X_SPACING; //36.0f; // 28
		float dy = PG_WIDGET_Y_SPACING; // 38.0f
		float x, y, bx, by;
		float groupWidth, groupHeight;
		float gPadding = 2.0f;
		float xStart = 23.0f + addX;
		//float yStart = 24.0f - fontSize / 2.0f + PG_WIDGET_TOP_SPACE;
		float yStart = 22.0f - fontSize / 2.0f + 0.f;// PG_WIDGET_TOP_SPACE;
		float textBounds[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		// Labels are on Left or Right Sides
		bool groupLabelOnSides = (headingLocation == HeadingLocation::Right || headingLocation == HeadingLocation::Left);
		// Labels are on the Top or Bottom ends
		bool groupLabelOnEnds = !groupLabelOnSides;
		bool groupLabelOnRight = headingLocation == HeadingLocation::Right;
		bool groupLabelOnTop = headingLocation == HeadingLocation::Top;
		float groupLabelAngle = 0.0f;
		if (groupLabelOnSides)
		{
			groupLabelAngle = (groupLabelOnRight) ? PI / 2.0 : -PI / 2.0;
		}
		int groupRows = 2;
		
		nvgFontSize(args.vg, fontSize);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 1);
		
		nvgFillColor(args.vg, textColor);
		int groupTextAlign = NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE;
		int symbolTextAlign = NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE;		
		float headerXOffset = 0.f;
		float hX = 0.0f;
		float lineOffset = 0.f;
		float lineEndOffset = -6.f;
		if (groupLabelOnEnds)
		{
			lineOffset = -fontSize / 2.f - gPadding;
			groupTextAlign = NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE;
			headerXOffset = gPadding;
			hX = gPadding;
		}

		//Vec boxSize = Vec(110.0f, dy * 0.94f);				
		Vec boxSize = Vec(106.0f, dy * 0.96f);		
		float boxStartX = 7.0f + addX;
		float boxStartYOffset = (groupLabelOnSides) ? -dy * 0.48f : -23.f;
		x = xStart + dx / 2.0f;
		y = yStart + dy / 2.0f;
		
		int nSides = 4;
		float xAmpl = 10.0f;
		float yAmpl = 10.0f;
		
		// FREQUENCY
		nvgTextAlign(args.vg, symbolTextAlign);
		nvgFontSize(args.vg, fontSize);		
		nvgText(args.vg, x, y, "FREQ", NULL);
		y += dy;
		
		//=== MAIN SHAPE ===
		// # Sides, Angle		
		// Group Box ::::::::::::::::::::
		groupRows = 2;
		nvgBeginPath(args.vg);
		if (drawBoxes)
		{
			nvgRoundedRect(args.vg, boxStartX, y + boxStartYOffset, boxSize.x, boxSize.y * groupRows, borderRadius);
		}
		else
		{
			// Draw a line on LHS
			nvgMoveTo(args.vg, boxStartX, y + boxStartYOffset + lineOffset);
			nvgLineTo(args.vg, boxStartX, y + boxStartYOffset + groupRows * boxSize.y + lineEndOffset);
		}
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
				
		if (groupLabelOnSides)
		{
			bx = (groupLabelOnRight) ? boxStartX + boxSize.x : boxStartX;
			by = y + boxStartYOffset + boxSize.y * 0.5f * groupRows;
		}
		else
		{
			bx = boxStartX;
			by = (groupLabelOnTop) ? y + boxStartYOffset : y + boxStartYOffset + groupRows * boxSize.x;
		}
		nvgTextAlign(args.vg, groupTextAlign);
		nvgSave(args.vg);
		nvgTranslate(args.vg, bx, by);
		nvgRotate(args.vg, groupLabelAngle);
		nvgFontSize(args.vg, fontSize);				
		nvgTextBounds(args.vg, 0, 0, PG_HEADING_MAIN_SHAPE, NULL, textBounds);
		groupWidth = textBounds[2] - textBounds[0];
		groupHeight = textBounds[3] - textBounds[1];
		nvgBeginPath(args.vg);
		nvgRect(args.vg, headerXOffset + textBounds[0] - gPadding, textBounds[1] - gPadding, groupWidth + 2 * gPadding, groupHeight + 2 * gPadding);
		nvgFillColor(args.vg, borderColor);	
		nvgFill(args.vg);
		nvgFillColor(args.vg, groupTxtColor);		
		nvgText(args.vg, hX, 0, PG_HEADING_MAIN_SHAPE, NULL);
		nvgRestore(args.vg);
		nvgFillColor(args.vg, textColor);
		nvgTextAlign(args.vg, symbolTextAlign);

		// # Sides :::::::::::::::::::::::	
		// # Sides: Symbol (Diamond)
		this->drawShape(args, nSides, /*x*/ xAmpl, /*y*/ yAmpl, 
			/*innerRadiusMult*/ 1.0f, /*innerAngleMult*/ 0.0f, 
			/*angleOffset_rad*/ 0.0f, /*rotation_rad*/ 0.0f,
			/*offset*/ Vec(x, y));		
		// # Sides: Text		
		nvgFillColor(args.vg, textColor);		
		nvgFontSize(args.vg, fontSize*1.5f);		
		nvgText(args.vg, x, y, "#", NULL);
		nvgFontSize(args.vg, fontSize);		
		y += dy;

		// Offset Angle :::::::::::::::::::::::	
		// Offset Angle: Symbol (Square)
		this->drawShape(args, nSides, /*x*/ xAmpl, /*y*/ yAmpl, 
			/*innerRadiusMult*/ 1.0f, /*innerAngleMult*/ 0.0f, 
			/*angleOffset_rad*/ PI/4.0f, /*rotation_rad*/ 0.0f,
			/*offset*/ Vec(x, y));				
		// Offset Angle: Text (angle symbol)		
		nvgSave(args.vg);
		nvgTranslate(args.vg, x, y);
		nvgBeginPath(args.vg);		
		nvgMoveTo(args.vg, 4, -5);
		nvgLineTo(args.vg, -4, 5);
		nvgLineTo(args.vg, 4, 5);
		nvgStrokeWidth(args.vg, 1.5f);
		nvgStroke(args.vg);		
		nvgRestore(args.vg);				
		y += dy;
		

		//=== 2DARY VERTICES ===
		// Inner Radius, Inner Angle Offset		
		// Group Box ::::::::::::::::::::
		groupRows = 2;
		nvgBeginPath(args.vg);
		if (drawBoxes)
		{
			nvgRoundedRect(args.vg, boxStartX, y + boxStartYOffset, boxSize.x, boxSize.y * groupRows, borderRadius);
		}
		else
		{
			// Draw a line on LHS
			nvgMoveTo(args.vg, boxStartX, y + boxStartYOffset + lineOffset);
			nvgLineTo(args.vg, boxStartX, y + boxStartYOffset + groupRows * boxSize.y + lineEndOffset);
		}
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
				
		if (groupLabelOnSides)
		{
			bx = (groupLabelOnRight) ? boxStartX + boxSize.x : boxStartX;
			by = y + boxStartYOffset + boxSize.y * 0.5f * groupRows;
		}
		else
		{
			bx = boxStartX;
			by = (groupLabelOnTop) ? y + boxStartYOffset : y + boxStartYOffset + groupRows * boxSize.x;
		}
		nvgTextAlign(args.vg, groupTextAlign);
		nvgSave(args.vg);
		nvgTranslate(args.vg, bx, by);
		nvgRotate(args.vg, groupLabelAngle);
		nvgFontSize(args.vg, fontSize);				
		nvgTextBounds(args.vg, 0, 0, PG_HEADING_STAR_SHAPE, NULL, textBounds);
		groupWidth = textBounds[2] - textBounds[0];
		groupHeight = textBounds[3] - textBounds[1];
		nvgBeginPath(args.vg);
		nvgRect(args.vg, headerXOffset + textBounds[0] - gPadding, textBounds[1] - gPadding, groupWidth + 2 * gPadding, groupHeight + 2 * gPadding);
		nvgFillColor(args.vg, borderColor);	
		nvgFill(args.vg);
		nvgFillColor(args.vg, groupTxtColor);		
		nvgText(args.vg, hX, 0, PG_HEADING_STAR_SHAPE, NULL);
		nvgRestore(args.vg);
		nvgFillColor(args.vg, textColor);
		nvgTextAlign(args.vg, symbolTextAlign);
		
		// Inner Radius	
		this->drawShape(args, nSides, /*x*/ xAmpl, /*y*/ yAmpl, 
			/*innerRadiusMult*/ 0.5f, /*innerAngleMult*/ 0.0f, 
			/*angleOffset_rad*/ 0.0f, /*rotation_rad*/ 0.0f,
			/*offset*/ Vec(x, y));
		y += dy;

		// Inner Radius Angle Skew	
		this->drawShape(args, nSides, /*x*/ xAmpl, /*y*/ yAmpl, 
			/*innerRadiusMult*/ 0.5f, /*innerAngleMult*/ -0.9f, 
			/*angleOffset_rad*/ 0.0f, /*rotation_rad*/ 0.0f,
			/*offset*/ Vec(x, y));
		y += dy;
		
		
		//=== Rotation ===
		// Rotation, Rotation is Abs, Center of Rotation X, Center of Rotation Y		
		// Group Box ::::::::::::::::::::
		groupRows = 3;
		nvgBeginPath(args.vg);
		if (drawBoxes)
		{
			nvgRoundedRect(args.vg, boxStartX, y + boxStartYOffset, boxSize.x, boxSize.y * groupRows, borderRadius);
		}
		else
		{
			// Draw a line on LHS
			nvgMoveTo(args.vg, boxStartX, y + boxStartYOffset + lineOffset);
			nvgLineTo(args.vg, boxStartX, y + boxStartYOffset + groupRows * boxSize.y + lineEndOffset);
		}
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
				
		if (groupLabelOnSides)
		{
			bx = (groupLabelOnRight) ? boxStartX + boxSize.x : boxStartX;
			by = y + boxStartYOffset + boxSize.y * 0.5f * groupRows;
		}
		else
		{
			bx = boxStartX;
			by = (groupLabelOnTop) ? y + boxStartYOffset : y + boxStartYOffset + groupRows * boxSize.x;
		}
		nvgTextAlign(args.vg, groupTextAlign);
		nvgSave(args.vg);
		nvgTranslate(args.vg, bx, by);
		nvgRotate(args.vg, groupLabelAngle);
		nvgFontSize(args.vg, fontSize);				
		nvgTextBounds(args.vg, 0, 0, "Rotation", NULL, textBounds);
		groupWidth = textBounds[2] - textBounds[0];
		groupHeight = textBounds[3] - textBounds[1];
		nvgBeginPath(args.vg);
		nvgRect(args.vg, headerXOffset + textBounds[0] - gPadding, textBounds[1] - gPadding, groupWidth + 2 * gPadding, groupHeight + 2 * gPadding);
		nvgFillColor(args.vg, borderColor);	
		nvgFill(args.vg);
		nvgFillColor(args.vg, groupTxtColor);		
		nvgText(args.vg, hX, 0, "Rotation", NULL);
		nvgRestore(args.vg);
		nvgFillColor(args.vg, textColor);		
		nvgTextAlign(args.vg, symbolTextAlign);

		// Rotation
		this->drawShape(args, nSides, /*x*/ xAmpl * 0.65f, /*y*/ yAmpl * 0.65f, 
			/*innerRadiusMult*/ 0.5f, /*innerAngleMult*/ 0.0f, 
			/*angleOffset_rad*/ 0.0f, /*rotation_rad*/ 60.0f,
			/*offset*/ Vec(x, y));
		
		nvgBeginPath(args.vg);
		float endAngle = -PI/3.0f;
		//nvgArc(args.vg, x, y, xAmpl + 1.0f, PI/3.0, -PI/3.0, NVG_CCW);
		nvgArc(args.vg, x, y, xAmpl + 1.0f, PI/3.0, -PI/3.0f, NVG_CCW);		
		nvgStrokeWidth(args.vg, 1.5f);
		nvgStroke(args.vg);
		float tx = (xAmpl + 1.0f) * COSFUNC(endAngle) + x;
		float ty = (xAmpl + 1.0f) * SINFUNC(endAngle) + y;
		this->drawShape(args, 3, /*x*/ 2.5, /*y*/ 2.5, 
			/*innerRadiusMult*/ 1.0f, /*innerAngleMult*/ 0.0f, 
			/*angleOffset_rad*/ endAngle, /*rotation_rad*/ 0.0f,
			/*offset*/ Vec(tx, ty), /*fill*/ true);
		
			
		// ABS
		float ax = x + 7.0f;
		float ay = y + 20.0f;
		nvgFontSize(args.vg, 8.0f);
		// j4S0n wants this to be called spin
		nvgText(args.vg, ax, ay, "SPIN", NULL);					
		y += dy;
		
		
		// Subscript dimensions
		float subScriptFontSize = fontSize * 0.7f;
		nvgFontSize(args.vg, subScriptFontSize);
		nvgTextBounds(args.vg, 0, 0, "c", NULL, textBounds);		
		float smWidth = textBounds[2] - textBounds[0];
		//float smHeight = textBounds[3] - textBounds[1];

		float letterWidth = fontSize;
		float letterHeight = fontSize;		
		
		// X_c
		nvgFillColor(args.vg, textColor);		
		nvgFontSize(args.vg, fontSize*1.5f);		
		nvgTextBounds(args.vg, 0, 0, "X", NULL, textBounds);
		letterWidth = textBounds[2] - textBounds[0];
		letterHeight = textBounds[3] - textBounds[1];		
		
		nvgFontSize(args.vg, fontSize*1.5f);				
		nvgText(args.vg, x - smWidth / 2.0f, y, "X", NULL);
		nvgFontSize(args.vg, subScriptFontSize);		
		nvgText(args.vg, x + letterWidth / 2.0f, y + letterHeight/2.0f - 1.0f, "c", NULL);		
		y += dy;

		// Y_c
		nvgFillColor(args.vg, textColor);		
		nvgFontSize(args.vg, fontSize*1.5f);		
		nvgTextBounds(args.vg, 0, 0, "Y", NULL, textBounds);
		letterWidth = textBounds[2] - textBounds[0];
		letterHeight = textBounds[3] - textBounds[1];		
		
		nvgFontSize(args.vg, fontSize*1.5f);				
		nvgText(args.vg, x - smWidth / 2.0f, y, "Y", NULL);
		nvgFontSize(args.vg, subScriptFontSize);		
		nvgText(args.vg, x + letterWidth / 2.0f, y + letterHeight/2.0f - 1.0f, "c", NULL);		
		y += dy;
		
		
		//////////////////////////////////////////////
		// Next Column
		//////////////////////////////////////////////
		x = xStart + PG_WIDGET_COLUMN_OFFSET + dx / 2.0f;
		y = yStart + PG_WIDGET_Y_SPACING * 3 + dy/2.0f;
		boxStartX += PG_WIDGET_COLUMN_OFFSET;
		//groupLabelOnRight = true;
		//groupLabelAngle = (groupLabelOnRight) ? PI / 2.0 : -PI/2.0;
		
		//=== Radius/Amplitude ===
		// Radius / Amplitude		
		// Group Box ::::::::::::::::::::
		groupRows = 2;
		nvgBeginPath(args.vg);
		if (drawBoxes)
		{
			nvgRoundedRect(args.vg, boxStartX, y + boxStartYOffset, boxSize.x, boxSize.y * groupRows, borderRadius);
		}
		else
		{
			// Draw a line on LHS
			nvgMoveTo(args.vg, boxStartX, y + boxStartYOffset + lineOffset);
			nvgLineTo(args.vg, boxStartX, y + boxStartYOffset + groupRows * boxSize.y + lineEndOffset);
		}
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
				
		if (groupLabelOnSides)
		{
			bx = (groupLabelOnRight) ? boxStartX + boxSize.x : boxStartX;
			by = y + boxStartYOffset + boxSize.y * 0.5f * groupRows;
		}
		else
		{
			bx = boxStartX;
			by = (groupLabelOnTop) ? y + boxStartYOffset : y + boxStartYOffset + groupRows * boxSize.x;
		}
		nvgTextAlign(args.vg, groupTextAlign);
		nvgSave(args.vg);
		nvgTranslate(args.vg, bx, by);
		nvgRotate(args.vg, groupLabelAngle);
		nvgFontSize(args.vg, fontSize);				
		nvgTextBounds(args.vg, 0, 0, "Radius", NULL, textBounds);
		groupWidth = textBounds[2] - textBounds[0];
		groupHeight = textBounds[3] - textBounds[1];
		nvgBeginPath(args.vg);
		nvgRect(args.vg, headerXOffset + textBounds[0] - gPadding, textBounds[1] - gPadding, groupWidth + 2 * gPadding, groupHeight + 2 * gPadding);
		nvgFillColor(args.vg, borderColor);	
		nvgFill(args.vg);
		nvgFillColor(args.vg, groupTxtColor);		
		nvgText(args.vg, hX, 0, "Radius", NULL);
		nvgRestore(args.vg);
		nvgFillColor(args.vg, textColor);
		nvgTextAlign(args.vg, symbolTextAlign);

		// R_x
		nvgFillColor(args.vg, textColor);		
		nvgFontSize(args.vg, fontSize*1.5f);		
		nvgTextBounds(args.vg, 0, 0, "R", NULL, textBounds);
		letterWidth = textBounds[2] - textBounds[0];
		letterHeight = textBounds[3] - textBounds[1];		
		
		nvgFontSize(args.vg, fontSize*1.5f);				
		nvgText(args.vg, x - smWidth / 2.0f, y, "R", NULL);
		nvgFontSize(args.vg, subScriptFontSize);		
		nvgText(args.vg, x + letterWidth / 2.0f, y + letterHeight/2.0f - 1.0f, "x", NULL);		
		y += dy;

		// R_y
		nvgFillColor(args.vg, textColor);		
		nvgFontSize(args.vg, fontSize*1.5f);		
		nvgTextBounds(args.vg, 0, 0, "R", NULL, textBounds);
		letterWidth = textBounds[2] - textBounds[0];
		letterHeight = textBounds[3] - textBounds[1];		
		
		nvgFontSize(args.vg, fontSize*1.5f);				
		nvgText(args.vg, x - smWidth / 2.0f, y, "R", NULL);
		nvgFontSize(args.vg, subScriptFontSize);		
		nvgText(args.vg, x + letterWidth / 2.0f, y + letterHeight/2.0f - 1.0f, "y", NULL);		
		y += dy;

		//=== Offset ===
		// Offset		
		// Group Box ::::::::::::::::::::
		groupRows = 2;
		nvgBeginPath(args.vg);
		if (drawBoxes)
		{
			nvgRoundedRect(args.vg, boxStartX, y + boxStartYOffset, boxSize.x, boxSize.y * groupRows, borderRadius);
		}
		else
		{
			// Draw a line on LHS
			nvgMoveTo(args.vg, boxStartX, y + boxStartYOffset + lineOffset);
			nvgLineTo(args.vg, boxStartX, y + boxStartYOffset + groupRows * boxSize.y + lineEndOffset);
		}
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
				
		if (groupLabelOnSides)
		{
			bx = (groupLabelOnRight) ? boxStartX + boxSize.x : boxStartX;
			by = y + boxStartYOffset + boxSize.y * 0.5f * groupRows;
		}
		else
		{
			bx = boxStartX;
			by = (groupLabelOnTop) ? y + boxStartYOffset : y + boxStartYOffset + groupRows * boxSize.x;
		}
		nvgTextAlign(args.vg, groupTextAlign);
		nvgSave(args.vg);
		nvgTranslate(args.vg, bx, by);
		nvgRotate(args.vg, groupLabelAngle);
		nvgFontSize(args.vg, fontSize);				
		nvgTextBounds(args.vg, 0, 0, "Offset", NULL, textBounds);
		groupWidth = textBounds[2] - textBounds[0];
		groupHeight = textBounds[3] - textBounds[1];
		nvgBeginPath(args.vg);
		nvgRect(args.vg, headerXOffset + textBounds[0] - gPadding, textBounds[1] - gPadding, groupWidth + 2 * gPadding, groupHeight + 2 * gPadding);
		nvgFillColor(args.vg, borderColor);	
		nvgFill(args.vg);
		nvgFillColor(args.vg, groupTxtColor);		
		nvgText(args.vg, hX, 0, "Offset", NULL);
		nvgRestore(args.vg);
		nvgFillColor(args.vg, textColor);
		nvgTextAlign(args.vg, symbolTextAlign);

		// X
		nvgFillColor(args.vg, textColor);		
		nvgFontSize(args.vg, fontSize*1.5f);		
		nvgText(args.vg, x, y, "X", NULL);
		y += dy;

		// Y
		nvgFillColor(args.vg, textColor);		
		nvgFontSize(args.vg, fontSize*1.5f);		
		nvgText(args.vg, x, y, "Y", NULL);
		y += dy;
		
		
		///////////////////////////////////////
		// Inputs/Outputs
		///////////////////////////////////////
		// Output port starting x and y
		float ox = 10.0f + PG_WIDGET_COLUMN_OFFSET;
		dx = 28.0f; //36.0f;
		ox += (53.0f - 2* dx);
		fontSize = 8; // Reduce
		float pad = 0.0f;
		float oy = box.size.y - 51.0f; 
		float portSize = 30.0f;
		float bw = portSize + pad * 2;		
		float bh = portSize + pad * 2 + fontSize;		
		by = oy - pad - fontSize;
		nvgTextAlign(args.vg, symbolTextAlign);
		
		// SYNC INPUT 
		bx = ox - pad + addX;
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, bx - 1, by, bw - 4, bh, 2.0);
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
		x = bx + portSize / 2.0f;
		y = oy - fontSize / 2.0f + 2.0f;
		nvgFillColor(args.vg, textColor);		
		nvgFontSize(args.vg, fontSize);		
		nvgText(args.vg, x - 3, y, "SYNC", NULL);		
		
		// X
		bx += dx;		
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, bx, by, bw + dx * 2, bh, 2.0);
		nvgFillColor(args.vg, TSColors::COLOR_BLACK);
		nvgFill(args.vg);
		x = bx + portSize / 2.0f;
		y = oy - fontSize / 2.0f + 2.0f;
		nvgFillColor(args.vg, textColor);		
		nvgFontSize(args.vg, fontSize);		
		nvgText(args.vg, x, y, "X", NULL);

		// Y
		bx += dx;
		// nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, bx, by, bw, bh, 2.0);
		// nvgFillColor(args.vg, TSColors::COLOR_BLACK);
		// nvgFill(args.vg);
		x += dx;
		nvgFillColor(args.vg, textColor);		
		nvgFontSize(args.vg, fontSize);		
		nvgText(args.vg, x, y, "Y", NULL);		

		// SYNC OUT
		bx += dx;
		// nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, bx, by, bw, bh, 2.0);
		// nvgFillColor(args.vg, TSColors::COLOR_BLACK);
		// nvgFill(args.vg);
		x += dx;
		nvgFillColor(args.vg, textColor);		
		nvgFontSize(args.vg, fontSize);		
		nvgText(args.vg, x, y, "SYNC", NULL);		
		
		return;
	}
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// polyGenWidget()
// @polyGenModule : (IN) Pointer to the polyGen module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
polyGenWidget::polyGenWidget(polyGen* polyGenModule) : TSSModuleWidgetBase(polyGenModule, true)
{
	box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	
	
	//bool isPreview = polyGenModule == NULL;	

	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/polyGen.svg")));
		addChild(panel);
	}
	
	/////////////////////////////////////////////
	// Labels
	/////////////////////////////////////////////
	{
		FramebufferWidget* labelContainer = new FramebufferWidget();
		labelContainer->box.size = box.size;
		labelContainer->box.pos = Vec(0, 0);
		addChild(labelContainer);
		
		polyGenLabelWidget* labels = new polyGenLabelWidget();
		labels->box.size = box.size;
		labels->box.pos = Vec(0, 0);
		labelContainer->addChild(labels);
	}
	
	//const float padding = 15.0f;
	float dx = 36.0f; // 28
	//float dy = 38.0f; // 30
	float x, y;
	float xStart = 10.0f + 7.5f;
	float yStart = 22.0f; // 24.0f;

	float topSpaceY = 0.f; // PG_WIDGET_TOP_SPACE;
	
	//////////////////////////////////////////////
	// Preview
	//////////////////////////////////////////////
	// 120 size
	//polyGenShapePreviewWidget* previewWidget = new polyGenShapePreviewWidget(polyGenModule, Vec(116, 116));
	polyGenShapePreviewWidget* previewWidget = new polyGenShapePreviewWidget(polyGenModule, Vec(110.f, 115));
	x = xStart + PG_WIDGET_COLUMN_OFFSET - 5.0f; 
	y = yStart - 2.f;
	previewWidget->box.pos = Vec(x, y);	
	addChild(previewWidget);
	
	int iIds[] = { polyGen::InputIds::FREQ_INPUT, 
		polyGen::InputIds::NUM_VERTICES_INPUT, polyGen::InputIds::ANGLE_OFFSET_INPUT, polyGen::InputIds::INNER_VERTICES_RADIUS_INPUT, polyGen::InputIds::INNER_VERTICES_ANGLE_INPUT, 
		polyGen::InputIds::ROTATION_INPUT, polyGen::InputIds::X_C_ROTATION_INPUT, polyGen::InputIds::Y_C_ROTATION_INPUT };
	int pIds[] = { polyGen::ParamIds::FREQ_PARAM,
		polyGen::ParamIds::NUM_VERTICES_PARAM, polyGen::ParamIds::ANGLE_OFFSET_PARAM, polyGen::ParamIds::INNER_VERTICES_RADIUS_PARAM, polyGen::ParamIds::INNER_VERTICES_ANGLE_PARAM, 
		polyGen::ParamIds::ROTATION_PARAM, polyGen::ParamIds::X_C_ROTATION_PARAM, polyGen::ParamIds::Y_C_ROTATION_PARAM };
	int bIds[] = { -1,
		-1, -1, -1, -1, 
		polyGen::ParamIds::ROTATION_ABS_PARAM, -1, -1};
	int lIds[] = { -1,
		-1, -1, -1, -1, 
		polyGen::LightIds::ROTATION_ABS_LIGHT, -1, -1};

	x = xStart;
	y = yStart + topSpaceY;
	addInputControlGroup(8, iIds, pIds, bIds, lIds, true, Vec(x, y));	

	//-------------------
	// Inputs and Knobs
	//-------------------
	const int numInputs = 4;// 9;//polyGen::InputIds::NUM_INPUTS - 1;
	int inputIds[] = {  //polyGen::InputIds::FREQ_INPUT, // polyGen::InputIds::NUM_VERTICES_INPUT, polyGen::InputIds::INNER_VERTICES_RADIUS_INPUT, polyGen::InputIds::INNER_VERTICES_ANGLE_INPUT, 
		polyGen::InputIds::X_AMPLITUDE_INPUT, polyGen::InputIds::Y_AMPLITUDE_INPUT, polyGen::InputIds::X_OFFSET_INPUT, polyGen::InputIds::Y_OFFSET_INPUT, 
		polyGen::InputIds::ROTATION_INPUT, polyGen::InputIds::X_C_ROTATION_INPUT, polyGen::InputIds::Y_C_ROTATION_INPUT };
	int paramIds[] = { //polyGen::ParamIds::FREQ_PARAM, //polyGen::ParamIds::NUM_VERTICES_PARAM, polyGen::ParamIds::INNER_VERTICES_RADIUS_PARAM, polyGen::ParamIds::INNER_VERTICES_ANGLE_PARAM, 
		polyGen::ParamIds::X_AMPLITUDE_PARAM, polyGen::ParamIds::Y_AMPLITUDE_PARAM, polyGen::ParamIds::X_OFFSET_PARAM, polyGen::ParamIds::Y_OFFSET_PARAM, 
		polyGen::ParamIds::ROTATION_PARAM, polyGen::ParamIds::X_C_ROTATION_PARAM, polyGen::ParamIds::Y_C_ROTATION_PARAM };
	int boolParamIds[] = { //-1, //-1, -1, -1, 
		-1, -1, -1, -1, polyGen::ParamIds::ROTATION_ABS_PARAM, -1, -1 };		
	int lightIds[] =     { //-1, //-1, -1, -1, 
		-1, -1, -1, -1, polyGen::LightIds::ROTATION_ABS_LIGHT, -1, -1 };

	x = xStart + PG_WIDGET_COLUMN_OFFSET;
	y = yStart + PG_WIDGET_Y_SPACING * 3 + topSpaceY; // yStart + previewWidget->box.size.y  + padding;	
	addInputControlGroup(numInputs, inputIds, paramIds, boolParamIds, lightIds, true, Vec(x, y));


#if TS_POLYGEN_MOD_ENABLED
	TS_RoundBlackKnob* knobPtr = NULL;	
	int modPids[3][2] = 
	{	
		{ polyGen::ParamIds::X_AMP_MOD_MULT_PARAM,  		polyGen::ParamIds::Y_AMP_MOD_MULT_PARAM }, 
		{ polyGen::ParamIds::X_OFFSET_MOD_AMPL_MULT_PARAM,  polyGen::ParamIds::Y_OFFSET_MOD_AMPL_MULT_PARAM }, 		
		{ polyGen::ParamIds::X_OFFSET_MOD_FREQ_MULT_PARAM, 	polyGen::ParamIds::Y_OFFSET_MOD_FREQ_MULT_PARAM },		
	};
	bool allowSnap[3][2] = 
	{
		{ false, false }, 
		{ false, false }, 
		{ false, false }		
	};
	y = yStart;	
	for (int i = 0; i < 3; i++)
	{
		x = xStart + dx * 3.0f;
		knobPtr = dynamic_cast<TS_RoundBlackKnob*>(createParam<TS_RoundBlackKnob>(Vec(x, y), polyGenModule, modPids[i][0]));
		knobPtr->allowRandomize = true;
		knobPtr->snap = allowSnap[i][0];
		addParam(knobPtr);
		
		x += dx;
		knobPtr = dynamic_cast<TS_RoundBlackKnob*>(createParam<TS_RoundBlackKnob>(Vec(x, y), polyGenModule, modPids[i][1]));
		knobPtr->allowRandomize = true;
		knobPtr->snap = allowSnap[i][1];
		addParam(knobPtr);
		
		y += dy;
	}
	//-------------------
	// Output
	//-------------------
	// Mod Output
	x = xStart + dx * 3.0f;	
	addOutput(TS_createOutput<TS_DEFAULT_PORT_OUTPUT>(Vec(x, y), polyGenModule, polyGen::OutputIds::X_MOD_OUTPUT, !plugLightsEnabled));
	x += dx;
	addOutput(TS_createOutput<TS_DEFAULT_PORT_OUTPUT>(Vec(x, y), polyGenModule, polyGen::OutputIds::Y_MOD_OUTPUT, !plugLightsEnabled));
#endif
	
	
	//-------------------
	// Output
	//-------------------
	// 106.0f 15.0f
	x = xStart + PG_WIDGET_COLUMN_OFFSET;
	dx = 28.0f; // was 36
	x += (53.0f - 2*dx);
	y = box.size.y - 50.0f; //y = box.size.y - 51.0f;
	// Separate input a little more
	addInput(TS_createInput<TS_DEFAULT_PORT_INPUT>(Vec(x - 3, y), polyGenModule, polyGen::InputIds::SYNC_INPUT, !plugLightsEnabled));	
	x += dx;	
	addOutput(TS_createOutput<TS_DEFAULT_PORT_OUTPUT>(Vec(x, y), polyGenModule, polyGen::OutputIds::X_OUTPUT, !plugLightsEnabled));
	x += dx;
	addOutput(TS_createOutput<TS_DEFAULT_PORT_OUTPUT>(Vec(x, y), polyGenModule, polyGen::OutputIds::Y_OUTPUT, !plugLightsEnabled));
	x += dx;	
	addOutput(TS_createOutput<TS_DEFAULT_PORT_OUTPUT>(Vec(x, y), polyGenModule, polyGen::OutputIds::SYNC_OUTPUT, !plugLightsEnabled));

	///// dx was 36
	// x = xStart + PG_WIDGET_COLUMN_OFFSET;
	// x += (53.0f - dx);
	// y = box.size.y - 51.0f; 
	// addOutput(TS_createOutput<TS_DEFAULT_PORT_OUTPUT>(Vec(x, y), polyGenModule, polyGen::OutputIds::X_OUTPUT, !plugLightsEnabled));
	// x += dx;
	// addOutput(TS_createOutput<TS_DEFAULT_PORT_OUTPUT>(Vec(x, y), polyGenModule, polyGen::OutputIds::Y_OUTPUT, !plugLightsEnabled));

	
	//-------------------	
	// Screws:
	//-------------------	
	this->TSSModuleWidgetBase::addScrews();

	return;
} // end polyGenWidget()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// addInputControlGroup()
// Add group of user controls + cv ports.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void polyGenWidget::addInputControlGroup(int numInputs, int inputIds[], int paramIds[], int boolIds[], int lightIds[], bool verticalLayout, Vec startLoc)
{	
	polyGen* polyGenModule = dynamic_cast<polyGen*>(module);
	float dx = (verticalLayout) ? PG_WIDGET_X_SPACING : 36.0f; // 28
	float dy = PG_WIDGET_Y_SPACING; // 38.0f
	float xStart = startLoc.x; //10.0f;	
	float yStart = startLoc.y; // previewWidget->box.pos.y + previewWidget->box.size.y + 10.0f;
	float x, y;
	x = xStart;
	y = yStart;
	TS_RoundBlackKnob* knobPtr = NULL;		
	for (int i = 0; i < numInputs; i++)
	{
		// Input Port
		if (verticalLayout)
			x = xStart;
		else
			y = yStart;	
		
		Vec pos1 = Vec(x, y);				
		if (verticalLayout)
			x += dx;
		else
			y += dy;
		Vec pos2 = Vec(x, y);
		
		// Vertical Layout: Port > Knob in row
		// Horizonal Layout: Knob > Port in column
		Vec* pos = (verticalLayout) ? &pos1 : &pos2;
		addInput(TS_createInput<TS_DEFAULT_PORT_INPUT>(*pos, polyGenModule, inputIds[i], !plugLightsEnabled));		
		
		pos = (verticalLayout) ? &pos2 : &pos1;
		knobPtr = dynamic_cast<TS_RoundBlackKnob*>(createParam<TS_RoundBlackKnob>(*pos, polyGenModule, paramIds[i]));
		//knobPtr->allowRandomize = true;
		switch (paramIds[i])
		{
			case polyGen::ParamIds::NUM_VERTICES_PARAM:
				knobPtr->snap = true;			
			break;
			default:
				knobPtr->snap = false;
			break;
		}
		
		addParam(knobPtr);
		
		if (boolIds[i] > -1)
		{
			int size = 13;
			float bx = x;
			float by = y;
			if (verticalLayout)
			{
				// We only have one now put underneath
				bx += -30.0f;// -10.0f;
				//x += 36.0f;
				size = 10;
				by += 25.0f;// 15;
			}
			else
			{
				by += dy;
				//y += dy;
			}
			Vec ledBtnSize = Vec(size, size); // LED button size			
			float buttonY = by + 5.0f; // y + 5.0f;
			TS_PadSwitch* btn = dynamic_cast<TS_PadSwitch*>(createParam<TS_PadSwitch>(Vec(bx, buttonY), polyGenModule, boolIds[i]));
			//TS_LEDButton* btn = dynamic_cast<TS_LEDButton*>(createParam<TS_LEDButton>(Vec(bx, buttonY ), polyGenModule, boolIds[i]));	
			btn->momentary = false;
			btn->setSize(ledBtnSize);
			addParam(btn);
			if (lightIds[i] > -1)
			{
				//Vec(bx + 2.5, buttonY + 2.5)
				ColorValueLight* light = TS_createColorValueLight<ColorValueLight>(Vec(bx, buttonY), polyGenModule,
					lightIds[i], ledBtnSize, TSColors::COLOR_TS_BLUE);
				addChild(light);				
			}
		}
		if (verticalLayout)
			y += dy;
		else
			x += dx;
	} // end loop through inputs
	return;	
} // end addInputControlGroup()


//===============================================================================
// polyGenShapePreviewWidget
//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args.vg : (IN) NVGcontext to draw on
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void polyGenShapePreviewWidget::draw(/*in*/ const DrawArgs &args)
{	
	this->drawBackground(args);
	return;
} // end polyGenShapePreviewWidget::draw()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// drawLayer()
// @args.vg : (IN) NVGcontext to draw on
// @layer : (IN) The layer (layer 1 is light layer)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void polyGenShapePreviewWidget::drawLayer(/*in*/ const DrawArgs& args, /*in*/ int layer)
{
	if (layer != DrawLayer::LightLayer)
	{
		return;
	}
	int numVertices = TS_POLYGEN_VERTICES_DEF;
	float innerRadiusMult = TS_POLYGEN_INNER_RADIUS_MULT_DEF;
	float innerAngleMult = TS_POLYGEN_INNER_OFFSET_DEG_DEF;
	float xAmpl = TS_POLYGEN_AMPL_DEF;
	float yAmpl = TS_POLYGEN_AMPL_DEF;
	bool useInnerVerts = false;
	float rotation_rad = 0.0f;
	float angleOffset_rad = 0.0f;
	float in_range[2] = { TS_CV_INPUT_MIN_DEF, TS_CV_INPUT_MAX_DEF };
	Vec rotCenter = Vec(0, 0);
	float xOff = 0.0f;
	float yOff = 0.0f;

	if (module != NULL)
	{
		numVertices = module->numVertices;
		xAmpl = module->xAmpl;
		yAmpl = module->yAmpl;
		innerRadiusMult = module->innerRadiusMult;
		innerAngleMult = module->innerAngleMult;
		useInnerVerts = module->useInnerVerts;
		rotation_rad = module->rotation_rad;
		angleOffset_rad = module->angleOffset_rad;
		rotCenter.x = module->xCRot;
		rotCenter.y = module->yCRot;
		xOff = module->xOffset;
		yOff = module->yOffset;
		// Maybe don't show rotation if it is constantly changing???		
		in_range[0] = module->outputVoltageRange[0];
		in_range[1] = module->outputVoltageRange[1];
	}

	// Screen:
	drawBackground(args);

#if DEBUG_POLY
	DEBUG_COUNT++;
	bool printDebug = (DEBUG_COUNT >= 100);
	if (printDebug)
		DEBUG_COUNT = 0;
#endif	

	// Calculate canvase box dimension
	float canvasCenterX = box.size.x / 2.0f; // Center (0, 0)
	float canvasCenterY = box.size.y / 2.0f; // Center (0, 0)
	float dim = 0.0f;
	// Center and make sure to scale
	if (box.size.y > box.size.x)
	{
		dim = box.size.x;
	}
	else
	{
		dim = box.size.y;
	}
	dim -= padding * 2;
	// Rescale amplitudes to fit in box
	float canvasRadius = dim / 2.0f;
	xAmpl = rescale(xAmpl, in_range[0], in_range[1], -canvasRadius, canvasRadius);
	yAmpl = rescale(-yAmpl, in_range[0], in_range[1], -canvasRadius, canvasRadius); // invert Y
	rotCenter.x = rescale(rotCenter.x, in_range[0], in_range[1], -canvasRadius, canvasRadius);
	rotCenter.y = rescale(-rotCenter.y, in_range[0], in_range[1], -canvasRadius, canvasRadius);	 // invert Y
	xOff = rescale(xOff, in_range[0], in_range[1], -canvasRadius, canvasRadius);
	yOff = rescale(-yOff, in_range[0], in_range[1], -canvasRadius, canvasRadius); // invert Y
	Vec innerAmpl = Vec(xAmpl * innerRadiusMult, yAmpl * innerRadiusMult);

	//============================
	// Calculate the Buffers
	//============================
	Vec nextPoint;
	bool nextPointCalc = false;
	float sinrot = SINFUNC(rotation_rad);
	float cosrot = COSFUNC(rotation_rad);
	float n = static_cast<float>(numVertices);
	float innerAngleShift = 0.5f * (1 + innerAngleMult) / n;
	int ix = 0;
	Vec maxVals = Vec(0, 0);

	for (int v = 0; v < numVertices; v++)
	{
		Vec point;
		float pAngle = static_cast<float>(v) / n;
		if (nextPointCalc)
		{
			point = nextPoint;
		}
		else
		{
			point.x = xAmpl * std::sin(pAngle * 2 * PI + angleOffset_rad);
			point.y = yAmpl * std::cos(pAngle * 2 * PI + angleOffset_rad);
		}
		// Non-Rotated
		rawBuffer[ix].x = point.x;
		rawBuffer[ix].y = point.y;
		if (point.x < 0 && -point.x > maxVals.x)
			maxVals.x = -point.x;
		else if (point.x > maxVals.x)
			maxVals.x = point.x;
		if (point.y < 0 && -point.y > maxVals.y)
			maxVals.y = -point.y;
		else if (point.y > maxVals.y)
			maxVals.y = point.y;

		// Rotate
		rotatedBuffer[ix].x = (point.x - rotCenter.x) * cosrot + (point.y - rotCenter.y) * sinrot + rotCenter.x;
		rotatedBuffer[ix].y = (-point.x - rotCenter.y) * sinrot + (point.y - rotCenter.y) * cosrot + rotCenter.y;
		ix++;

		if (useInnerVerts)
		{
			//------------------------
			// Calculate 2ndary point
			//------------------------
#if TS_POLYGEN_IRADIUS_REL_2_MID_POINT
			// Find amplitude/radius of mid point between the two corners
			int v2 = v + 1;
			if (v2 >= numVertices)
			{
				nextPoint = rawBuffer[0];
			}
			else
			{
				float p2Angle = static_cast<float>(v2) / n;
				nextPoint.x = xAmpl * std::sin(p2Angle * 2 * PI + angleOffset_rad);
				nextPoint.y = yAmpl * std::cos(p2Angle * 2 * PI + angleOffset_rad);
				nextPointCalc = true;
			}
			float midX = point.x + (nextPoint.x - point.x) * 0.5f;
			float midY = point.y + (nextPoint.y - point.y) * 0.5f;
			float ampl = std::sqrt(midX * midX + midY * midY) * innerRadiusMult;
			innerAmpl.x = ampl * sgn(xAmpl);
			innerAmpl.y = ampl * sgn(yAmpl);
#endif
			pAngle += innerAngleShift;
			point.x = innerAmpl.x * std::sin(pAngle * 2 * PI + angleOffset_rad);
			point.y = innerAmpl.y * std::cos(pAngle * 2 * PI + angleOffset_rad);
			if (point.x < 0 && -point.x > maxVals.x)
				maxVals.x = -point.x;
			else if (point.x > maxVals.x)
				maxVals.x = point.x;
			if (point.y < 0 && -point.y > maxVals.y)
				maxVals.y = -point.y;
			else if (point.y > maxVals.y)
				maxVals.y = point.y;

			// Non-Rotated
			rawBuffer[ix].x = point.x;
			rawBuffer[ix].y = point.y;

			// Rotate
			rotatedBuffer[ix].x = (point.x - rotCenter.x) * cosrot + (point.y - rotCenter.y) * sinrot + rotCenter.x;
			rotatedBuffer[ix].y = (-point.x - rotCenter.y) * sinrot + (point.y - rotCenter.y) * cosrot + rotCenter.y;
			ix++;
		} // end if useInnerVerts
	} // end loop through vertices


	int numPoints = (useInnerVerts) ? numVertices * 2 : numVertices;
	float pad = padding - lineWidth;
	nvgSave(args.vg);

	//=============================================================
	// Draw main preview (Rotated and Translated)
	//=============================================================
	Rect b = Rect(Vec(pad, pad), box.size.minus(Vec(pad * 2, pad * 2)));
	nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
	Vec offset = Vec(canvasCenterX + xOff, canvasCenterY + yOff);
	drawShape(args, rotatedBuffer, numPoints, offset, /*mult*/ 1.0, lineWidth, lineColor);
	nvgResetScissor(args.vg);

	//=============================================================
	// Draw secondary small preview (pre-Rotation and translation)
	//=============================================================
	//float xRatio = (xAmpl < 0) ? -xAmpl / canvasRadius : xAmpl / canvasRadius;
	//float yRatio = (yAmpl < 0) ? -yAmpl / canvasRadius : yAmpl / canvasRadius;
	//float smMult = 0.3f / ((xRatio > yRatio) ? xRatio : yRatio);
	float maxVal = (maxVals.x > maxVals.y) ? maxVals.x : maxVals.y;
	float smMult = 0.3f * canvasRadius / maxVal;
	//offset.x = 0.85f * dim + padding; // Right
	//offset.y = 0.85f * dim + padding; // Bottom	
	offset.x = 0.85f * box.size.x - padding; // Right
	offset.y = 0.85f * box.size.y - padding; // Bottom	
	//b = Rect(Vec(offset.x - 0.15f * dim, offset.y - 0.15f * dim), Vec(0.3 * dim, 0.3 * dim));
	//nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
	drawShape(args, rawBuffer, numPoints, offset, /*mult*/ smMult, lineWidth, secondColor);
	//nvgResetScissor(args.vg);
	nvgRestore(args.vg);
	return;
} // end polyGenShapePreviewWidget::drawLayer()

void polyGenShapePreviewWidget::drawBackground(const DrawArgs& args) {
	// Screen:
	nvgBeginPath(args.vg);
	nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
	nvgFillColor(args.vg, backgroundColor);
	nvgFill(args.vg);
	nvgStrokeWidth(args.vg, 1.0);
	nvgStrokeColor(args.vg, borderColor);
	nvgStroke(args.vg);
	return;
}

void polyGenShapePreviewWidget::drawShape(const DrawArgs &args, Vec* buffer, int bufferLen, Vec offset, float mult, float lWidth, NVGcolor lColor)
{
	nvgBeginPath(args.vg);		
	for (int v = 0; v < bufferLen; v++)
	{		
		if (v > 0)
			nvgLineTo(args.vg, mult*buffer[v].x + offset.x, mult*buffer[v].y + offset.y);
		else
		{
			nvgMoveTo(args.vg, mult*buffer[v].x + offset.x, mult*buffer[v].y + offset.y);			
		}
	} // end loop through vertices
	nvgLineTo(args.vg, mult*buffer[0].x + offset.x, mult*buffer[0].y + offset.y); // Complete shape
	nvgStrokeWidth(args.vg, lWidth);
	nvgStrokeColor(args.vg, lColor);	
	nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
	nvgStroke(args.vg);		
	return;
}
