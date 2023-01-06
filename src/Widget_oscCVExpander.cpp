
#include "Widget_oscCVExpander.hpp"
using namespace rack;
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "Module_oscCVExpander.hpp"
#include "TSColors.hpp"

#define COL_RACK_GRID_WIDTH		6	// Number of RACK_GRID_WIDTH a single column of channel ports are.

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCVExpanderWidget()
// Instantiate a oscCVExpander widget. 
// @oscExpanderModule : (IN) Pointer to the osc module.
// @expanderDirection : (IN) What direction (IN/OUT).
// @numChannels: (IN) Specify the number of channels. Essential for previews where the module is NULL.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
oscCVExpanderWidget::oscCVExpanderWidget(oscCVExpander* oscExpanderModule, TSOSCCVExpanderDirection expanderDirection, int numChannels) : TSSModuleWidgetBase(oscExpanderModule, false)
{
	const int screwSize = 15;
	bool isPreview = this->module == NULL; // If this is null, then this isn't a real module instance but a 'Preview'?	
	//if (!isPreview && oscExpanderModule == NULL)
	//{
	//	oscExpanderModule = dynamic_cast<oscCVExpander*>(this->module);
	//}
	if (isPreview)
	{
		this->numberChannels = numChannels;
		this->expanderType = expanderDirection;
	}
	else
	{
		this->numberChannels = oscExpanderModule->numberChannels;
		this->expanderType = oscExpanderModule->expanderType;
	}
	this->numberColumns = numberChannels / channelsPerColumn;
	box.size = Vec(COL_RACK_GRID_WIDTH * numberColumns * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	bandXStart = (expanderType == TSOSCCVExpanderDirection::Input) ? box.size.x - bandWidth : 0.f;
	lastConfigStatus = false;

	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		if (expanderDirection == TSOSCCVExpanderDirection::Input)
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/cvOSC.svg")));
		else
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/OSCcv.svg")));			
		addChild(panel);
	}

	//////////////////////////////////////////////
	// Framebuffer
	//////////////////////////////////////////////
	// Cache widgets that don't change or don't change that often.
	fbw = new FramebufferWidget();
	fbw->box.size = box.size;
	addChild(fbw);
		
	//////////////////////////////////////////////
	// Indicator - For when we are being configured.
	//////////////////////////////////////////////		
	oscCVExpanderSideIndicator* indicator = new oscCVExpanderSideIndicator(this, Vec(box.size.x, box.size.y - screwSize*2));
	indicator->box.pos = Vec(0, screwSize);
	//addChild(indicator);
	fbw->addChild(indicator);

	//////////////////////////////////////////////
	// Top display
	//////////////////////////////////////////////			
	Vec topScreenSize = Vec(box.size.x - 2*TROWA_HORIZ_MARGIN, 48);
	TSOscCVExpanderTopDisplay* display = new TSOscCVExpanderTopDisplay(this);
	display->box.pos = Vec(TROWA_HORIZ_MARGIN, 24);
	display->box.size = topScreenSize;
	addChild(display);
	
	//////////////////////////////////////////////
	// Labels
	//////////////////////////////////////////////
	TSOscCVExpanderLabels* labelArea = new TSOscCVExpanderLabels(this->expanderType);
	labelArea->box.pos = Vec(TROWA_HORIZ_MARGIN, topScreenSize.y + 24);
	labelArea->box.size = Vec(box.size.x - TROWA_HORIZ_MARGIN * 2, box.size.y - labelArea->box.pos.y - 15);
	labelArea->numberColumns = numberColumns;
	//addChild(labelArea);
	fbw->addChild(labelArea);

	//////////////////////////////////////////////
	// Connection/Indicator Lights
	//////////////////////////////////////////////		
	int xStart = TROWA_HORIZ_MARGIN;
	int yStart = 20;	
	int x = xStart;
	int y = yStart;
	Vec ledSize = Vec(10, 10);
	
	//-- Main Indicator Lights --
	int lvl = (isPreview) ? 0 : oscExpanderModule->lvlFromMaster - 1;
	if (lvl < 0)
		lvl = 0;
	NVGcolor color; 
	// Master connection:
	ledSize = Vec(12, 12);
	y = topScreenSize.y + 27;
	x = (box.size.x - ledSize.x) / 2;
	color = TSColors::CHANNEL_COLORS[lvl % TSColors::NUM_CHANNEL_COLORS];	
	lightMasterConnected = dynamic_cast<ColorValueLight*>(TS_createColorValueLight<ColorValueLight>(Vec(x, y - 1), oscExpanderModule, oscCVExpander::LightIds::MASTER_CONNECTED_LIGHT, ledSize, color));
	addChild(lightMasterConnected);
	
	// Left connection:
	ledSize = Vec(10, 10);
	x = xStart;
	color = TSColors::CHANNEL_COLORS[(lvl - 1 + TSColors::NUM_CHANNEL_COLORS) % TSColors::NUM_CHANNEL_COLORS];
	lightLeftConnected = dynamic_cast<ColorValueLight*>(TS_createColorValueLight<ColorValueLight>(Vec(x, y), oscExpanderModule, oscCVExpander::LightIds::LEFT_CONNECTED_LIGHT, ledSize, color));
	addChild(lightLeftConnected);
		
	// Right connection:
	ledSize = Vec(10, 10);	
	x = box.size.x - TROWA_HORIZ_MARGIN - ledSize.x;
	color = TSColors::CHANNEL_COLORS[(lvl + 1) % TSColors::NUM_CHANNEL_COLORS];
	lightRightConnected = dynamic_cast<ColorValueLight*>(TS_createColorValueLight<ColorValueLight>(Vec(x, y), oscExpanderModule, oscCVExpander::LightIds::RIGHT_CONNECTED_LIGHT, ledSize, color));
	addChild(lightRightConnected);
	
	
	
	// Inputs / Outputs
	int colDx = COL_RACK_GRID_WIDTH * RACK_GRID_WIDTH;
	int dx = 28;
	int dy = 30;
	ledSize = Vec(5, 5);
	float ledYOffset = 12.5;
	//xStart = (expanderType == TSOSCCVExpanderDirection::Input) ? TROWA_HORIZ_MARGIN : box.size.x - TROWA_HORIZ_MARGIN - 2*dx - ledSize.x/2;
	xStart = (expanderType == TSOSCCVExpanderDirection::Input) ? TROWA_HORIZ_MARGIN : colDx - TROWA_HORIZ_MARGIN - 2 * dx - ledSize.x / 2;

	yStart = 98;

	x = xStart;
	y = yStart;

	// Save our start position for later
	colStartPos = Vec(x, y);
	rowDy = dy; // Save row height for later.

	// Fix OSCcv lights not lighting up on message received:
	int lightOffset = (expanderType == TSOSCCVExpanderDirection::Input) ? 0 : 1; // For our LightIds, we still reserve 2 per channel in case we want to use them.
	for (int r = 0; r < numberChannels; r++)
	{
		TS_Port* port = NULL;

		if (r > 0 && r % channelsPerColumn == 0)
		{
			xStart += colDx; // Go to the next column
			y = yStart;
		}

		NVGcolor chColor = TSColors::CHANNEL_COLORS[r % TSColors::NUM_CHANNEL_COLORS];

		// Trigger Input:
		x = xStart;
		if (colorizeChannels)
			port = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), oscExpanderModule, oscCVExpander::InputIds::CH_INPUT_START + r * 2, !plugLightsEnabled, chColor));
		else
			port = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), oscExpanderModule, oscCVExpander::InputIds::CH_INPUT_START + r * 2, !plugLightsEnabled));				
		if (expanderType == TSOSCCVExpanderDirection::Input)
		{
			addInput(port);			
		}
		else
		{
			port->type = engine::Port::OUTPUT;			
			addOutput(port);			
		}
		

		// Value input:
		x += dx;
		if (colorizeChannels)
			port = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), oscExpanderModule, oscCVExpander::InputIds::CH_INPUT_START + r * 2 + 1, !plugLightsEnabled, chColor));
		else
			port = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), oscExpanderModule, oscCVExpander::InputIds::CH_INPUT_START + r * 2 + 1, !plugLightsEnabled));					
		if (expanderType == TSOSCCVExpanderDirection::Input)
		{
			addInput(port);			
		}
		else
		{
			port->type = engine::Port::OUTPUT;
			addOutput(port);	
		}
		
		// Light (to indicate when we send/recv OSC)
		float ledX = (expanderType == TSOSCCVExpanderDirection::Input) ? xStart - ledSize.x : x + dx + ledSize.x/2.0;		
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(ledX, y + ledYOffset), oscExpanderModule, oscCVExpander::LightIds::CH_LIGHT_START + r * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL + lightOffset, ledSize, chColor));

		y += dy;
	} // end ports
	
	addChild(createWidget<ScrewBlack>(Vec(0, 0)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - screwSize, 0)));
	addChild(createWidget<ScrewBlack>(Vec(0, box.size.y - screwSize)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - screwSize, box.size.y - screwSize)));
	
	return;
}



void oscCVExpanderWidget::step()
{
	if (this->module != NULL)
	{
		oscCVExpander* thisModule = dynamic_cast<oscCVExpander*>(this->module);				
		if (thisModule)
		{
			if (thisModule->lvlFromMaster > 0)
			{
				// Adjust light colors ====================
				int lvl = thisModule->lvlFromMaster - 1;
				if (lvl < 0)
					lvl = 0;
				NVGcolor color = TSColors::CHANNEL_COLORS[lvl % TSColors::NUM_CHANNEL_COLORS];
				if (lightMasterConnected)
				{
					lightMasterConnected->setColor(color);
				}	
				expanderColor = color; // Save our color.
				
				if (lightRightConnected)
				{
					color = thisModule->getColor(lvl + 1, false);
					//(lvl < 1 && expanderType == TSOSCCVExpanderDirection::Input) ? TSColors::COLOR_WHITE : TSColors::CHANNEL_COLORS[(lvl + 1) % TSColors::NUM_CHANNEL_COLORS];				
					lightRightConnected->setColor(color);
				}
							
				if (lightLeftConnected)
				{
					color = thisModule->getColor(lvl + 1, true);
					//color = (lvl < 1 && expanderType == TSOSCCVExpanderDirection::Output) ? TSColors::COLOR_WHITE : TSColors::CHANNEL_COLORS[(lvl - 1 + TSColors::NUM_CHANNEL_COLORS) % TSColors::NUM_CHANNEL_COLORS];
					lightLeftConnected->setColor(color);
				}
				
				// Show if we are being configured
				const float da = 500.f;
				const float maxa = 1.0f;
				const float mina = 0.2f;
				if (lastConfigStatus != thisModule->beingConfigured)
				{
					lastConfigStatus = thisModule->beingConfigured;
					lastConfigA = (thisModule->beingConfigured) ? maxa : 0.0f;
					// Redraw our indicator
					fbw->setDirty();
					dir = false;
				}
				if (lastConfigStatus)
				{
					// Redraw our indicator
					fbw->setDirty();
					if (thisModule->beingConfigured)
					{
						this->configColumnIx = thisModule->configureColIx;
						/// TODO: Add check for which channel is being configured.
					}
					else
					{
						this->configColumnIx = -1; // Not being configured.
					}
					if (dir)
					{
						lastConfigA += da * APP->engine->getSampleTime();
					}
					else
					{
						lastConfigA -= da * APP->engine->getSampleTime();					
					}
					if (lastConfigA > maxa)
					{
						lastConfigA = maxa;
						dir = false;
					}
					else if (lastConfigA < mina)
					{
						lastConfigA = mina;
						dir = true;
					}
				}				
			} // end if master
			else
			{
				// We have a Module, but no master.
				expanderColor = defaultExpanderColor;
				lastConfigStatus = false;
				dir = false;
			}
		} // end if master
		else
		{
			// No Module
			lastConfigStatus = false;
			dir = false;
			expanderColor =  TSColors::CHANNEL_COLORS[0];			
		}
	}	
	Widget::step();
	return;
}




void oscCVExpanderWidget::onDragEnd(const event::DragEnd &e)
{
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;	
	ModuleWidget::onDragEnd(e);
	if (this->module != NULL)
	{
		oscCVExpander* thisModule = dynamic_cast<oscCVExpander*>(this->module);	
		thisModule->calcMasterDistance();
	}
	return;
} // end onDragEnd()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args.vg : (IN) NVGcontext to draw on
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCVExpanderSideIndicator::draw(/*in*/ const DrawArgs &args)
{
	if (parent->module)
	{
		if (parent->lastConfigStatus)
		{
			nvgSave(args.vg);
			nvgScissor(args.vg, 0, 0, box.size.x, box.size.y);

			NVGcolor color =  parent->expanderColor; 
			color.a = parent->lastConfigA;
			//nvgRGBA(parent->expanderColor.r, parent->expanderColor.g, parent->expanderColor.b, parent->lastConfigA*255);
			nvgBeginPath(args.vg);
			// nvgRect(args.vg, 0, 0.0, box.size.x, box.size.y);
			// nvgFillColor(args.vg, color);
			// nvgFill(args.vg);			
			nvgRect(args.vg, strokeWidth/2.f, strokeWidth/2.f, box.size.x-strokeWidth, box.size.y-strokeWidth);			
			nvgStrokeWidth(args.vg, strokeWidth);
			nvgStrokeColor(args.vg, color);
			nvgStroke(args.vg);

			int configColIx = parent->configColumnIx;
			if (parent->numberColumns > 1 && configColIx > -1)
			{
				// Highlight the column being edited
				color.a = 0.6;
				int colDx = COL_RACK_GRID_WIDTH * RACK_GRID_WIDTH;
				float margin = 0.f;
				//const int dx = 28;
				const int screwSize = 15;
				//float x = parent->colStartPos.x + configColIx * colDx - margin - screwSize + strokeWidth;
				float x = configColIx * colDx + 1.0f; 	// Actually just fill put the entire column (minus the column divider line)
				float y = parent->colStartPos.y - margin - screwSize + 1.0f;// +2.0f;
				//float colWidth = dx * 3.0f - strokeWidth;
				//Vec cSize = Vec(colWidth + margin * 2.f, parent->rowDy * parent->channelsPerColumn + margin * 2.f);
				// Actually just fill put the entire column
				Vec cSize = Vec(colDx - 2.0f, parent->rowDy * parent->channelsPerColumn + margin * 2.f);
				nvgBeginPath(args.vg);
				nvgRect(args.vg, x, y, cSize.x, cSize.y);
				nvgFillColor(args.vg, color);
				nvgFill(args.vg);
			}


			nvgRestore(args.vg);			
		}
	}
	return;
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step()
// Calculate scrolling and stuff?
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVExpanderTopDisplay::step() {
	bool isPreview = parentWidget->module == NULL;
	displayName = std::string("XP1-293"); // Something random
	oscCVExpander* thisModule = NULL;
	bool connectedToMaster = false;
	if (!isPreview)
	{
		thisModule = dynamic_cast<oscCVExpander*>(parentWidget->module);
		displayName = thisModule->displayName;
		connectedToMaster = thisModule->lvlFromMaster > 0;
	}
	if (displayName.compare(lastName) != 0 || lastConnectedToMaster != connectedToMaster)
	{
		if (parentWidget->numberChannels > 16)
		{
			// We have more room, repeat it here.
			sprintf(scrollingMsg, "%s  -  %s  -  %s  -  %s  -  ",
				displayName.c_str(), (connectedToMaster) ? "Master Found" : "No Connection",
				displayName.c_str(), (connectedToMaster) ? "Master Found" : "No Connection");
		}
		else
			sprintf(scrollingMsg, "%s  -  %s  -  ", displayName.c_str(), (connectedToMaster) ? "Master Found" : "No Connection");
	}

	dt += 100.0 / APP->engine->getSampleRate();
	if (dt > scrollTime_sec) 
	{
		dt = 0.0f;
		if (static_cast<size_t>(scrollIx) == strlen(scrollingMsg) - 1)
			scrollIx = 0;
		else
			scrollIx++;
	}
	lastName = displayName;
	lastConnectedToMaster = connectedToMaster;
	TransparentWidget::step(); // parent whatever he does
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args.vg : (IN) NVGcontext to draw on
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVExpanderTopDisplay::drawLayer(/*in*/ const DrawArgs &args, int layer)
{
	if (layer == 1)
	{
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath); // Rack v2 load font each time
		std::shared_ptr<Font> labelFont = APP->window->loadFont(labelFontPath); // Rack v2 load font each time

		if (font == nullptr)
			return;
		if (labelFont == nullptr)
			return;

		
		// Background Colors:
		NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);

		// Screen:
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		nvgFillColor(args.vg, backgroundColor);
		nvgFill(args.vg);
		nvgStrokeWidth(args.vg, 1.0);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);

		if (!showDisplay)
			return;

		int margin = 9;
		Rect b = Rect(Vec(0, 0), Vec(box.size.x - margin, box.size.y));
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

		int x, y;

		nvgFontSize(args.vg, fontSize);
		nvgFontFaceId(args.vg, labelFont->handle);
		nvgTextLetterSpacing(args.vg, 1); // 2.5;			
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);	
		x = box.size.x / 2.f;
		y = 11;
		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
		nvgFillColor(args.vg, textColor);
		nvgText(args.vg, x, y, directionName.c_str(), NULL);	

		nvgFontSize(args.vg, fontSize);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 1); // 2.5;		
		nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);	
		x = margin;	
		y += 15;
		nvgFillColor(args.vg, parentWidget->expanderColor);	
		//nvgText(args.vg, x, y, displayName.c_str(), NULL);	
		// Start (left on screen) of scrolling message:
		const char * subStr = scrollingMsg + scrollIx;
		nvgText(args.vg, x, y, subStr, NULL);
		// Get circular wrap (right part of screen) - start of message again:
		float txtBounds[4] = { 0,0,0,0 };
		float nextX = nvgTextBounds(args.vg, x, y, subStr, NULL, txtBounds);
		x = nextX; // +24
		if (x < b.size.x) {
			// Wrap the start of the string around
			nvgText(args.vg, x, y, scrollingMsg, subStr);
		}			
		nvgResetScissor(args.vg);		
	}
	this->Widget::drawLayer(args, layer);
	
	return;
} // end TSOscCVExpanderTopDisplay::draw()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args.vg : (IN) NVGcontext to draw on
// Draw labels on our widget.
// Also now draw divider lines for multi-column expanders.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVExpanderLabels::draw(/*in*/ const DrawArgs &args) {
	std::shared_ptr<Font> font = APP->window->loadFont(fontPath); // Rack v2 load font each time
	if (font == nullptr)
		return;
	// Default Font:
	nvgFontSize(args.vg, fontSize);
	nvgFontFaceId(args.vg, font->handle);
	nvgTextLetterSpacing(args.vg, 1);

	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
	nvgFillColor(args.vg, textColor);
	nvgFontSize(args.vg, fontSize);

	float x, y;
	int dx;
	int xStart, yStart;
	xStart = 0;
	yStart = 25; // 17
	dx = 28;

	//TROWA_HORIZ_MARGIN
	//colDx - TROWA_HORIZ_MARGIN - 2 * dx - ledSize.x / 2
	int colDx = COL_RACK_GRID_WIDTH * RACK_GRID_WIDTH;
	const char* labelType = (expanderType == TSOSCCVExpanderDirection::Input) ? "INPUTS" : "OUTPUTS";
	if (expanderType == TSOSCCVExpanderDirection::Input)
	{
		//--- * Inputs *---//
		// (Left hand side)		
		//x = xStart + dx / 2;		
		xStart += dx / 2;
	}
	else 
	{
		//-- * Outputs *--//
		// (Right hand side)				
		//x = box.size.x - dx / 2 - dx;
		xStart += colDx - 2*TROWA_HORIZ_MARGIN - dx / 2 - dx;
	}
	NVGcolor gridColor = textColor;// nvgRGB(0x44, 0x44, 0x44);
	// Now we may allow > 1 column of channels.
	for (int c = 0; c < numberColumns; c++)
	{
		// TRIG:
		x = xStart;
		y = yStart;
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgText(args.vg, x, y, "TRG", NULL);
		// VAL:
		x += dx;
		y = yStart;
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgText(args.vg, x, y, "VAL", NULL);

		// Draw Divider Line if we have > 1 column
		if (c > 0)
		{
			nvgBeginPath(args.vg);
			float lx = colDx * c - box.pos.x;
			nvgMoveTo(args.vg, /*start x*/ lx, /*start y*/ yStart + 2.f);
			nvgLineTo(args.vg, /*x*/ lx, /*y*/ box.size.y - 26.f);
			nvgStrokeWidth(args.vg, 1.0);
			nvgStrokeColor(args.vg, gridColor);
			nvgStroke(args.vg);
		}

		xStart += colDx;
	}

	// Bottom (INPUTS/OUTPUTS)
	//x = xStart + dx;
	x = box.size.x / 2.0f;
	y = box.size.y - 13;
	nvgText(args.vg, x, y, labelType, NULL);
	return;
} // end TSOscCVExpanderLabels::draw()