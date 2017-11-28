#ifndef TROWASOFT_HPP
#define TROWASOFT_HPP

#include "rack.hpp"
#define TROWA_SCOPE_NUM_WAVEFORMS	3

//#include "trowaSoftComponents.hpp"

using namespace rack;


extern Plugin *plugin;

////////////////////
// Module Widgets
////////////////////

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSModuleWidgetBase
// Base Module Widget. Remove randomize of parameters.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSModuleWidgetBase : ModuleWidget {
	TSSModuleWidgetBase() { return; }
	void randomize() override 
	{
		// for (ParamWidget *param : params) {
			// param->randomize();
		// }
		if (module) {
			module->randomize();
		}
		return;
	}	
};


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSequencerWidgetBase
// Sequencer Widget Base Class (adds common UI controls).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSequencerWidgetBase : TSSModuleWidgetBase {
	TSSequencerWidgetBase();
	void addBaseControls();
};
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeqWidget
// Widget for the trowaSoft pad / trigger sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct trigSeqWidget : TSSequencerWidgetBase {
	trigSeqWidget();
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeqWidget
// Widget for the trowaSoft knob / voltage sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct voltSeqWidget : TSSequencerWidgetBase {
	voltSeqWidget();
};


#endif