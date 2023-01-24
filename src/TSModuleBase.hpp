#ifndef TSMODULEBASE_HPP
#define TSMODULEBASE_HPP

#include <rack.hpp>
using namespace rack;

struct TSModuleBase : Module
{
	//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
	// configParam
	// Extended to include description.
	//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=	
	template <class TParamQuantity = ParamQuantity>
	TParamQuantity* configParam(int paramId, float minValue, float maxValue, float defaultValue, std::string label = "", std::string unit = "", 
		float displayBase = 0.f, float displayMultiplier = 1.f, float displayOffset = 0.f,
		std::string descr = "")
	{
		TParamQuantity* pQty = dynamic_cast<TParamQuantity*>( Module::configParam<TParamQuantity>(paramId, minValue, maxValue, defaultValue, label, unit, displayBase, displayMultiplier, displayOffset) );
		pQty->description = descr;
		return pQty;
	}
	//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
	// configSwitch
	// Extended to include description.
	//=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=	
	template <class TSwitchQuantity = SwitchQuantity>
	TSwitchQuantity* configSwitch(int paramId, float minValue, float maxValue, float defaultValue, std::string name, std::string descr, std::vector<std::string> labels) {
		TSwitchQuantity* sq = Module::configParam<TSwitchQuantity>(paramId, minValue, maxValue, defaultValue, name);
		sq->labels = labels;
		sq->description = descr;
		return sq;
	}
};


#endif // !TSMODULEBASE_HPP