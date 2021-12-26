
#include "Widget_oscCVExpander.hpp"
using namespace rack;
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "Module_oscCVExpander.hpp"
#include "TSColors.hpp"

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCVExpanderWidget()
// Instantiate a oscCVExpander widget. 
// @oscExpanderModule : (IN) Pointer to the osc module.
// @expanderDirection : (IN) What direction (IN/OUT).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
oscCVExpanderWidget::oscCVExpanderWidget(oscCVExpander* oscExpanderModule, TSOSCCVExpanderDirection expanderDirection) : TSSModuleWidgetBase(oscExpanderModule, false)
{
	const int screwSize = 15;
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	bool isPreview = this->module == NULL; // If this is null, then this isn't a real module instance but a 'Preview'?	
	if (!isPreview && oscExpanderModule == NULL)
	{
		oscExpanderModule = dynamic_cast<oscCVExpander*>(this->module);
	}
	this->numberChannels = (isPreview) ? TROWA_OSCCVEXPANDER_DEFAULT_NUM_CHANNELS : oscExpanderModule->numberChannels; 
	this->expanderType = (isPreview) ? expanderDirection : oscExpanderModule->expanderType;	
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
	// Indicator - For when we are being configured.
	//////////////////////////////////////////////		
	oscCVExpanderSideIndicator* indicator = new oscCVExpanderSideIndicator(this, Vec(box.size.x, box.size.y - screwSize*2));
	indicator->box.pos = Vec(0, screwSize);
	addChild(indicator);

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
	addChild(labelArea);


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
	int dx = 28;
	int dy = 30;
	ledSize = Vec(5, 5);
	float ledYOffset = 12.5;
	xStart = (expanderType == TSOSCCVExpanderDirection::Input) ? TROWA_HORIZ_MARGIN : box.size.x - TROWA_HORIZ_MARGIN - 2*dx - ledSize.x/2;
	yStart = 98;
	
	x = xStart;
	y = yStart;
	for (int r = 0; r < numberChannels; r++)
	{
		TS_Port* port = NULL;

		// Trigger Input:
		x = xStart;
		if (colorizeChannels)
			port = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), oscExpanderModule, oscCVExpander::InputIds::CH_INPUT_START + r * 2, !plugLightsEnabled, TSColors::CHANNEL_COLORS[r % TSColors::NUM_CHANNEL_COLORS]));
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
			port = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), oscExpanderModule, oscCVExpander::InputIds::CH_INPUT_START + r * 2 + 1, !plugLightsEnabled, TSColors::CHANNEL_COLORS[r % TSColors::NUM_CHANNEL_COLORS]));
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
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(ledX, y + ledYOffset), oscExpanderModule, oscCVExpander::LightIds::CH_LIGHT_START + r * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL, ledSize, TSColors::CHANNEL_COLORS[r]));		

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
					dir = false;
				}
				if (lastConfigStatus)
				{
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
void TSOscCVExpanderTopDisplay::draw(/*in*/ const DrawArgs &args)
{
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
	return;
} // end TSOscCVExpanderTopDisplay::draw()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @args.vg : (IN) NVGcontext to draw on
// Draw labels on our widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVExpanderLabels::draw(/*in*/ const DrawArgs &args) {
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
	
	const char* labelType = (expanderType == TSOSCCVExpanderDirection::Input) ? "INPUTS" : "OUTPUTS";
	if (expanderType == TSOSCCVExpanderDirection::Input)
	{
		//--- * Inputs *---//
		// (Left hand side)		
		x = xStart + dx / 2;		
	}
	else 
	{
		//-- * Outputs *--//
		// (Right hand side)				
		x = box.size.x - dx / 2 - dx;
	}
	// TRIG:
	y = yStart;
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
	nvgText(args.vg, x, y, "TRG", NULL);
	// VAL:
	x += dx;
	y = yStart;
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
	nvgText(args.vg, x, y, "VAL", NULL);
	// Bottom (INPUTS/OUTPUTS)
	//x = xStart + dx;
	x = box.size.x / 2.0f;
	y = box.size.y - 13;
	nvgText(args.vg, x, y, labelType, NULL);
	return;
} // end TSOscCVExpanderLabels::draw()