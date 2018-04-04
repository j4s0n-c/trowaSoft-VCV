#include "trowaSoft.hpp"
#include "Widget_multiScope.hpp"
#include "Widget_multiScope_Old.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_voltSeq.hpp"
//#include "Widget_trowaTrack.hpp"
#include "Module_oscCV.hpp"
//#include "TSMidiOutTest.hpp"

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

	//// Tracker:
	////p->addModel(Model::create<trowaTrack, trowaTrackWidget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "trowaTrack", /*name*/ "trowaTrack", /*Tags*/ SEQUENCER_TAG, EXTERNAL_TAG));
	//p->addModel(modelTrowaTrack);
	//p->addModel(modelTrowaTrack);

	// Osc <==> CV:
	p->addModel(modelOscCV);
	//p->addModel(modelTSMidiOutTest);
	// Scope Modules:
	//p->addModel(Model::create<multiScope, multiScopeWidget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "multiScope", /*name*/ "multiScope", /*Tags*/ VISUAL_TAG, EFFECT_TAG, UTILITY_TAG));
	p->addModel(modelMultiScope);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables within this file or the individual module files to reduce startup times of Rack.	
	return;
}
