#ifndef TROWASOFT_HPP
#define TROWASOFT_HPP

#include "rack.hpp"
using namespace rack;

#define TROWA_PLUGIN_NAME	"trowaSoft"
extern Plugin *plugin;

// An internal version number (integer) value. Simple int value for quick/dirty easy comparison.
#define TROWA_INTERNAL_VERSION_INT		7 
// 7: 0.5.5.2


#include "TSSModuleWidgetBase.hpp"
#include "TSSequencerWidgetBase.hpp"

/////////////////////////////////////////////////////////////////////////////////
// Module Widgets
/////////////////////////////////////////////////////////////////////////////////

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
	// Create context menu with shifting.
	Menu *createContextMenu() override;
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq64Widget
// Widget for the trowaSoft 64-step sequencer.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct trigSeq64Widget : TSSequencerWidgetBase {
	trigSeq64Widget();
};

#endif