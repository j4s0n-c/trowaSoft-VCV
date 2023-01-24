#ifndef TROWASOFT_MODULE_TSPARAMQUANTITY_HPP
#define TROWASOFT_MODULE_TSPARAMQUANTITY_HPP

// Extend ParamQuantity

#include <rack.hpp>
using namespace rack;

#include <string>
#include <map>

#include "trowaSoftUtilities.hpp"


// ParamQuantity with settable precision
struct TS_ParamQuantity : ParamQuantity {
	// If snap is on, what value to snap to.
	//float snapToValue = 0.0f;

	// Default in Quantity is 5.
	//int displayPrecision = 5;
	// Get the number of total decimal places for generating the display value string
	int getDisplayPrecision() override{
		return displayPrecision;
	}	
	// Set the number of total decimal places for generating the display value string
	void setDisplayPrecision(int prec) {
		displayPrecision = prec;
	}
}; // end struct TS_ParamQuantity

// Act like a switch quantity if labels have items and snap is enabled. Otherwise, act like a normal param quantity.
struct TS_SwitchQuantity : ParamQuantity {
	std::vector<std::string> labels;

	std::string getDisplayValueString() override {
		if (snapEnabled && labels.size() > 0) {
			int index = (int)std::floor(getValue() - getMinValue());
			if (!(0 <= index && index < (int)labels.size()))
				return "";
			return labels[index];
		}
		else {
			return this->ParamQuantity::getDisplayValueString();
		}
	}
	void setDisplayValueString(std::string s) override {
		if (snapEnabled && labels.size() > 0) {
			// Find label that matches string, case insensitive.
			auto it = std::find_if(labels.begin(), labels.end(), [&](const std::string& a) {
				return string::lowercase(a) == string::lowercase(s);
				});
			if (it == labels.end())
				return;
			int index = std::distance(labels.begin(), it);
			setValue(getMinValue() + index);
		}
		else {
			return this->ParamQuantity::setDisplayValueString(s);
		}
	}
};

// Enumeration param quantity
struct TS_ParamQuantityEnum : TS_ParamQuantity {
	// Map numeric to string. Keys will be multiplied by @valMult [default 10,000] to help with comparisons (precision errors) and stored as int. 
	// Values should be proper case.
	std::map<int, std::string> enumVal2String;
	// Map string to numeric. Duplicate copy kept for lookup speed. Keys should be lower-case.
	// Values will be the normal values.
	std::map<std::string, float> enumString2Val;
	// If we should transform into enums.
	bool enumOn = true;
	// Value to upscale (to int) for float keys. Should make this the max of precision.
	float valMult = 10000.f;
	
	bool roundVals = true;

	// Adds the values as possible enumerations.
	void addToEnumMap(float val, std::string str)
	{
		// Val Float (int) -> String Display
		int valKey = val *valMult;
		if (enumVal2String.find(valKey) == enumVal2String.end()) {
			// Add
			enumVal2String.insert(std::pair<int, std::string>(valKey, str));
		}
		else{
			// Replace
			enumVal2String[valKey] = str;
		}
		// String Display (lowercase) -> Val Float (float)
		std::string strKey = str;
		std::transform(strKey.begin(), strKey.end(), strKey.begin(), ::tolower);
		trim(strKey);
		if (enumString2Val.find(strKey) == enumString2Val.end()) {
			// Add
			enumString2Val.insert(std::pair<std::string, float>(strKey, val));
		}
		else {
			// Replace
			enumString2Val[strKey] = val;
		}
		return;
	}
	
	// Returns a string representation of the display value 
	// Try to find our enumeration string.
	std::string getDisplayValueString() override 
	{		
		std::string displayStr;		
		if (enumOn)
		{
			// Find closest string 			
			float val = getDisplayValue();
			if (roundVals)
				val = roundf(val);
			int valKey = val * valMult;
			bool found = false;
			if (enumVal2String.find(valKey) == enumVal2String.end()) {
				// Search for first pair that is < our lookup (backwards ordered), so the highest
				for(std::map<int,std::string>::reverse_iterator iter = enumVal2String.rbegin(); iter != enumVal2String.rend(); ++iter)
				{
					int key =  iter->first;
					if (key < valKey) {
						displayStr = iter->second;
						found = true;
						break;
					}
					else if (!found) {
						displayStr = iter->second; // Should go to the last one
					}
				} // end iterate through vals				
			}
			else {
				// Exact match
				displayStr = enumVal2String[valKey]; 
				found = true;				
			}		
		}
		else 
		{
			// Default to base getDisplayValueString()
			displayStr = this->TS_ParamQuantity::getDisplayValueString();
		}
		return displayStr;
	} // end getDisplayValueString()
	
	
	// Sets the value based on the display string.
	void setDisplayValueString(std::string str) override {
		//Quantity::setDisplayValueString(str);
//DEBUG("TS_ParamQuantityEnum::setDisplayValueString(\"%s\") - Starting...", str.c_str());
		float displayVal = 0.0f;		
		if (enumOn) 
		{
			std::string strKey = str;
			std::transform(strKey.begin(), strKey.end(), strKey.begin(), ::tolower);
			trim(strKey);
			bool found = false;
//DEBUG("TS_ParamQuantityEnum::setDisplayValueString(\"%s\") - Key is %s.", str.c_str(), strKey.c_str());		
			
			if (enumString2Val.find(strKey) == enumString2Val.end()) {
				// Search for first pair that is less than this lookup
				int lookupSize = strKey.size();
				for(std::map<std::string, float>::reverse_iterator iter = enumString2Val.rbegin(); iter != enumString2Val.rend(); ++iter)
				{
					std::string key =  iter->first;
					int keySize = key.size();
//DEBUG("TS_ParamQuantityEnum::setDisplayValueString(\"%s\") -    Search %s = %s result is %d", str.c_str(), key.c_str(), strKey.c_str(), key.compare(strKey));	
					if (keySize <= lookupSize && key.compare(strKey) < 0) {
						displayVal = iter->second;
						found = true;
						break;
					}
					else if (keySize > lookupSize && key.compare(0, lookupSize, strKey) <= 0) {
						// Lookup value is shorter than our enums.
						displayVal = iter->second;
						found = true;
						break;					
					}
					else if (!found) {
						displayVal = iter->second; // Should go to the last one
					}
				} // end iterate through vals
//DEBUG("TS_ParamQuantityEnum::setDisplayValueString(\"%s\") - Key is %s. Found CLOSEST match - %7.5f", str.c_str(), strKey.c_str(), displayVal);														
			}
			else {
				// Exact match
				displayVal = enumString2Val[strKey]; 
//DEBUG("TS_ParamQuantityEnum::setDisplayValueString(\"%s\") - Key is %s. Found exact match - %7.5f", str.c_str(), strKey.c_str(), displayVal);
				found = true;				
			}
			this->setDisplayValue(displayVal);
		}
		else 
		{
			// Default to base
			this->TS_ParamQuantity::setDisplayValueString(str);
		}		
		return;
	}	
}; // end struct TS_ParamQuantityEnum


#endif // !TROWASOFT_MODULE_TSPARAMQUANTITY_HPP
