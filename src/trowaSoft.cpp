#include "trowaSoft.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = "trowaSoft";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif

	p->addModel(createModel<trigSeqWidget>("trowaSoft", "trigSeq", "trigSeq", SEQUENCER_TAG));
	p->addModel(createModel<trigSeq64Widget>("trowaSoft", "trigSeq64", "trigSeq64", SEQUENCER_TAG));
    p->addModel(createModel<voltSeqWidget>("trowaSoft", "voltSeq", "voltSeq", SEQUENCER_TAG));
	
	
	
	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables within this file or the individual module files to reduce startup times of Rack.
	
	return;
}
