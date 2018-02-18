#ifndef TROWASOFT_MODULE_TSSEQUENCERWIDGETBASE_HPP
#define TROWASOFT_MODULE_TSSEQUENCERWIDGETBASE_HPP

#include "rack.hpp"
using namespace rack;

#include "TSSModuleWidgetBase.hpp"
//#include "TSSequencerModuleBase.hpp"
#include "TSOSCConfigWidget.hpp"

struct TSSeqDisplay;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSequencerWidgetBase
// Sequencer Widget Base Class (adds common UI controls).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSequencerWidgetBase : TSSModuleWidgetBase {
	// Top digital display for sequencer.
	TSSeqDisplay *display;
	// OSC configuration widget.
	TSOSCConfigWidget* oscConfigurationScreen;
	// Instantiate a widget.
	TSSequencerWidgetBase();
	// Step
	void step() override;
	// Add base controls.
	void addBaseControls() { addBaseControls(false); }
	// Add base controls.
	void addBaseControls(bool addGridLines);
	// Create context menu with common adds to sequencers.
	Menu *createContextMenu() override;
};
#endif