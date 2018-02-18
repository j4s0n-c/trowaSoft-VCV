#include "trowaSoft.hpp"
#include "Widget_multiScope.hpp"

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
	p->addModel(createModel<trigSeqWidget>(TROWA_PLUGIN_NAME, "trigSeq", "trigSeq", SEQUENCER_TAG, EXTERNAL_TAG));
	p->addModel(createModel<trigSeq64Widget>(TROWA_PLUGIN_NAME, "trigSeq64", "trigSeq64", SEQUENCER_TAG, EXTERNAL_TAG));
    p->addModel(createModel<voltSeqWidget>(TROWA_PLUGIN_NAME, "voltSeq", "voltSeq", SEQUENCER_TAG, EXTERNAL_TAG));
	
	// Scope Modules:
	p->addModel(createModel<multiScopeWidget>(TROWA_PLUGIN_NAME, "multiScope", "multiScope", VISUAL_TAG, EFFECT_TAG, UTILITY_TAG));
	
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables within this file or the individual module files to reduce startup times of Rack.	
	return;
}
