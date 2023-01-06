#include "trowaSoft.hpp"
#include "Module_trigSeq.hpp"
#include "Module_voltSeq.hpp"
#include "Widget_multiScope.hpp"
//#include "Widget_multiScope_Old.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_voltSeq.hpp"
#include "Module_oscCV.hpp"
#include "Module_multiSeq.hpp"
#include "Module_multiOscillator.hpp"
#include "TSBlank.hpp"
#include "Module_oscCVExpander.hpp"

// The pluginInstance-wide instance of the Plugin class
Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;
	// Sequencer Modules:
	p->addModel(modelTrigSeq);	
	p->addModel(modelTrigSeq64);
	p->addModel(modelVoltSeq);
	// Sequencer Modules:
	// New trig + volt Sequencer together
	p->addModel(modelMultiSeq64);

	// Osc <==> CV:
	p->addModel(modelOscCV);
	p->addModel(modelOscCVExpanderInput);
	p->addModel(modelOscCVExpanderOutput);
	p->addModel(modelOscCVExpanderInput16);
	p->addModel(modelOscCVExpanderOutput16);
	p->addModel(modelOscCVExpanderInput32);
	p->addModel(modelOscCVExpanderOutput32);

	// Scope Modules:
	p->addModel(modelMultiScope);

	// Oscillator
	p->addModel(modelMultiOscillator);
	
	// Blank
	p->addModel(modelBlank);

	// Any other pluginInstance initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables within this file or the individual module files to reduce startup times of Rack.	
	return;
}
