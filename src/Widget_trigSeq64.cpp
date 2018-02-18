#include <string.h>
#include <stdio.h>
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_trigSeq.hpp"

#define N64_NUM_STEPS	64
#define N64_NUM_ROWS	 8
#define N64_NUM_COLS	 (N64_NUM_STEPS/N64_NUM_ROWS)



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// 64SeqWidget
// Widget for the trowaSoft 64-step sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
trigSeq64Widget::trigSeq64Widget() : TSSequencerWidgetBase()
{
	trigSeq *module = new trigSeq(N64_NUM_STEPS, N64_NUM_ROWS, N64_NUM_COLS);
	setModule(module);
	
	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/trigSeq.svg")));
		addChild(panel);
	}
	
	TSSequencerWidgetBase::addBaseControls(true);
	
	// (User) Input Pads ==================================================	
	int y = 115;
	int x = 79;
	int dx = 0;
	Vec padSize = Vec(24, 24);
	Vec lSize = Vec(padSize.x - 2*dx, padSize.y - 2*dx);
	int spacing = padSize.x + 5;
	
	for (int r = 0; r < module->numRows; r++) //---------THE PADS
	{
		for (int c = 0; c < module->numCols; c++)
		{			
			// Pad buttons:
			TS_PadSwitch* pad = new TS_PadSwitch(padSize);
			pad->box.pos = Vec(x, y);
			pad->btnId = r * module->numCols + c;
			pad->module = module;
			pad->groupId = module->oscId; // For now this is unique.
			pad->paramId = TSSequencerModuleBase::CHANNEL_PARAM + r*module->numCols + c;
			pad->setLimits(0, 1);
			pad->setDefaultValue(0);
			pad->value = 0;
			addParam(pad);
			//addParam(createParam<TS_PadSquare>(Vec(x, y), module, TSSequencerModuleBase::CHANNEL_PARAM + r*TROWA_SEQ_STEP_NUM_COLS + c, 0.0, 1.0, 0.0));
			// Keep a reference to our pad lights so we can change the colors
			module->padLightPtrs[r][c] = dynamic_cast<TS_LightSquare*>(TS_createColorValueLight<TS_LightSquare>(/*pos */ Vec(x+dx, y+dx), 
				/*module*/ module, 
				/*lightId*/ TSSequencerModuleBase::PAD_LIGHTS + r*module->numCols + c,
				/* size */ lSize, /* color */ module->voiceColors[module->currentChannelEditingIx]));
			addChild( module->padLightPtrs[r][c] );
			
			module->lights[TSSequencerModuleBase::PAD_LIGHTS + r*module->numCols + c].value = 0;

			x+= spacing;
		}		
		y += spacing; // Next row
		x = 79;
	} // end loop through NxN grid
	
	module->modeString = module->modeStrings[module->selectedOutputValueMode];
	module->initialized = true;
	return;
}
