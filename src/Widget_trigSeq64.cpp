#include <string.h>
#include <stdio.h>
#include "trowaSoft.hpp"
//#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_trigSeq.hpp"

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq64Widget()
// Widget for the trowaSoft 64-step sequencer.
// @seqModule : (IN) Pointer to the sequencer module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
trigSeq64Widget::trigSeq64Widget(trigSeq* seqModule) : TSSequencerWidgetBase(seqModule)
{
	maxSteps = N64_NUM_STEPS;
	// [02/24/2018] Adjusted for 0.60 differences. Main issue is possiblity of NULL module...
	bool isPreview = this->module == NULL; // If this is null, then this isn't a real module instance but a 'Preview'?	
	if (!isPreview && seqModule == NULL)
	{
		seqModule = dynamic_cast<trigSeq*>(this->module);
	}

	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/trigSeq.svg")));
		addChild(panel);
	}
	
	this->TSSequencerWidgetBase::addBaseControls(true);
	
	// (User) Input Pads ==================================================	
	int y = 115;
	int x = 79;
	int dx = 0;
	Vec padSize = Vec(24, 24);
	Vec lSize = Vec(padSize.x - 2*dx, padSize.y - 2*dx);
	int spacing = padSize.x + 5;
	NVGcolor lightColor = TSColors::COLOR_TS_RED;
	this->numCols = N64_NUM_COLS;
	this->numRows = N64_NUM_ROWS;
	int groupId = 0;
	if (!isPreview)
	{
		numCols = seqModule->numCols;
		numRows = seqModule->numRows;
		lightColor = seqModule->voiceColors[seqModule->currentChannelEditingIx];
		groupId = seqModule->oscId; // Use this id for now since this is unique to each module instance.
	}
	int id = 0;
	padLightPtrs = new ColorValueLight**[numRows];	
	for (int r = 0; r < numRows; r++) //---------THE PADS
	{
		padLightPtrs[r] = new ColorValueLight*[numCols];
		for (int c = 0; c < numCols; c++)
		{			
			// Pad buttons:
			TS_PadSwitch* pad = dynamic_cast<TS_PadSwitch*>(createParam<TS_PadSwitch>(Vec(x,y), seqModule, TSSequencerModuleBase::CHANNEL_PARAM + id));
			pad->box.size = padSize;
			pad->momentary = false;
			//pad->box.pos = Vec(x, y);
			pad->btnId = id;
			pad->groupId = groupId;
			if (pad->getParamQuantity())
			{
				ParamQuantity* pQty = pad->getParamQuantity();
				pQty->minValue = 0;
				pQty->maxValue = 1;
				pQty->defaultValue = 0;
				pQty->setValue(0);			
			}
			addParam(pad);

			// Lights:
			TS_LightSquare* padLight = dynamic_cast<TS_LightSquare*>(TS_createColorValueLight<TS_LightSquare>(/*pos */ Vec(x + dx, y + dx),
				/*seqModule*/ seqModule,
				/*lightId*/ TSSequencerModuleBase::PAD_LIGHTS + id, // r * numCols + c
				/* size */ lSize, /* color */ lightColor));
			padLight->cornerRadius = 3.0;
			addChild(padLight);
			padLightPtrs[r][c] = padLight; // Keep the reference to this light to change color			
			id++;
			x+= spacing;
		}		
		y += spacing; // Next row
		x = 79;
	} // end loop through NxN grid
	
	if (seqModule != NULL)
	{
		seqModule->modeString = seqModule->modeStrings[seqModule->selectedOutputValueMode];
		seqModule->initialized = true;
	}
	return;
}
