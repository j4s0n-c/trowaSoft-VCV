#ifndef TROWASOFT_HPP
#define TROWASOFT_HPP

#include <rack.hpp>
using namespace rack;

#define TROWA_PLUGIN_NAME	"trowaSoft"
extern Plugin *pluginInstance;

// An internal version number (integer) value. Simple int value for quick/dirty easy comparison.
#define TROWA_INTERNAL_VERSION_INT		17  //17: 1.0.3

//After Rack v1.0 https://github.com/VCVRack/Rack/issues/266
//In the vMAJOR.MINOR.REVISION scheme, vMAJOR is the compatibility verison with Rack and MINOR.REVISION is the revision of your plugin. 
//For example, v1.* plugins are all compatible with Rack v1.*.

// 7: 0.5.5.2
// 8: 0.6.5.2dev - For Rack 0.6.0dev
// 9: 0.6.1
//10: 0.6.2
//11: 0.6.3
//12: 0.6.4
//13: 0.6.5
//14: 1.0.0
//15: 1.0.1
//16: 1.0.2
//17: 1.0.3

#endif