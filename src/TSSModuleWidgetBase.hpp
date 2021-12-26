#ifndef TROWASOFT_MODULE_TSMODULEWIDGETBASE_HPP
#define TROWASOFT_MODULE_TSMODULEWIDGETBASE_HPP

#include <rack.hpp>
using namespace rack;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSModuleWidgetBase
// Base Module Widget. Remove randomize of parameters.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSModuleWidgetBase : ModuleWidget 
{
	bool randomizeParameters = false;
	
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
	
	//--- v2 This is not needed anymore as randomizeEnabled is now on the ParamQuantity directly --
	// // New v1 Rack Widget base class doesn't let use override this :-(.
	// // So parameters will be randomized ALL the time, unless we override it on the ParamWidget.
	// /// TODO: Override randomize() on all ParamWidgets (i.e. make custom widgets for all widgets we use).
	// void randomizeAction()  { // <-- Not virtual :(
		// assert(module);

		// // history::ModuleChange
		// history::ModuleChange *h = new history::ModuleChange;
		// h->name = "randomize module";
		// h->moduleId = module->id;
		// h->oldModuleJ = toJson();

		// if (this->randomizeParameters)
		// {
			// // v2: No params collection now.			
			// //for (ParamWidget *param : params) {
			// std::vector<ParamWidget*> params = getParams();
			// for (ParamWidget *param : params) {
				// param->randomize();
			// }			
		// }
		// APP->engine->randomizeModule(module);

		// h->newModuleJ = toJson();
		// APP->history->push(h);				
	// }	
};

#endif // end if not defined