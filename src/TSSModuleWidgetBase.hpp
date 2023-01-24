#ifndef TROWASOFT_MODULE_TSMODULEWIDGETBASE_HPP
#define TROWASOFT_MODULE_TSMODULEWIDGETBASE_HPP

#include <rack.hpp>
using namespace rack;

#include "trowaSoftComponents.hpp"

#define TS_DEFAULT_SCREW			TS_ScrewBlack	// Screw class to use for all module widgets
#define TS_DEFAULT_PORT_INPUT		TS_Port			// Default input port
#define TS_DEFAULT_PORT_OUTPUT		TS_Port			// Default output port

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSModuleWidgetBase
// Base Module Widget. Remove randomize of parameters.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSModuleWidgetBase : ModuleWidget
{
	bool randomizeParameters = false;
	// Screen size
	const int screwSize = 15;

	TSSModuleWidgetBase(Module* tsModule) : ModuleWidget()
	{
		if (this->module == NULL)
			setModule(tsModule); // v1.0 We are not supposed to set this anymore.	
		return;
	}
	TSSModuleWidgetBase(Module* tsModule, bool randomizeParams) : TSSModuleWidgetBase(tsModule)
	{
		randomizeParameters = randomizeParams;
		return;
	}

	// Add screw widgets.
	virtual void addScrews()
	{
		// Realistically we should push these screws in, but they look better at the edges plus
		// we have to redo the backgrounds if push them in
		
		// Screws:
		addChild(createWidget<TS_DEFAULT_SCREW>(Vec(0, 0)));
		addChild(createWidget<TS_DEFAULT_SCREW>(Vec(box.size.x - screwSize, 0)));
		addChild(createWidget<TS_DEFAULT_SCREW>(Vec(0, box.size.y - screwSize)));
		addChild(createWidget<TS_DEFAULT_SCREW>(Vec(box.size.x - screwSize, box.size.y - screwSize)));
	}
};

#endif // end if not defined