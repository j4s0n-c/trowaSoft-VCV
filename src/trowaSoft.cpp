#include "trowaSoft.hpp"
#include "Widget_multiScope.hpp"
#include "Widget_multiScope_Old.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_voltSeq.hpp"
#include "Module_oscCV.hpp"
#include "Module_multiOscillator.hpp"

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TROWA_PLUGIN_NAME;
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	// Sequencer Modules:
	// Add EXTERNAL_TAG for osc
	// [03/08/2018] Create model objects in module cpp files per forum topic.
	p->addModel(modelTrigSeq);
	p->addModel(modelTrigSeq64);
	p->addModel(modelVoltSeq);

	// Osc <==> CV:
	p->addModel(modelOscCV);
	
	// Scope Modules:
	p->addModel(modelMultiScope);

	// Oscillator
	p->addModel(modelMultiOscillator);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables within this file or the individual module files to reduce startup times of Rack.	
	return;
}
