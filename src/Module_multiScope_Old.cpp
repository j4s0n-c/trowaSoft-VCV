#include "Features.hpp"

#if !USE_NEW_SCOPE

#include <string.h>
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
//#include "dsp/digital.hpp"
#include "Module_multiScope_Old.hpp"
#include "Widget_multiScope_Old.hpp"

// multiScope model.
Model *modelMultiScope = createModel<multiScope, multiScopeWidget>(/*slug*/ "multiScope");


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiScope()
// Multi scope.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiScope::multiScope() // : Module(multiScope::NUM_PARAMS, multiScope::NUM_INPUTS, multiScope::NUM_OUTPUTS, multiScope::NUM_LIGHTS)
{
	config(multiScope::NUM_PARAMS, multiScope::NUM_INPUTS, multiScope::NUM_OUTPUTS, multiScope::NUM_LIGHTS);
	initialized = false;
	firstLoad = true;
	float initColorKnobs[4] = { -10, -3.33, 3, 7.2 };

	// Conifgure Parameters:
	//configParam(/*paramId*/ , /*minVal*/, /*maxVal*/, /*defVal*/, /*label*/, /*unit*/, /*displayBase*/, /*displayMultiplier*/, /*displayOffset*/)	
	configParam(/*id*/ multiScope::INFO_DISPLAY_TOGGLE_PARAM, /*minVal*/ 0, /*maxVal*/ 1, /*defVal*/ 1, /*label*/ "Toggle Display");
	for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
	{
		waveForms[wIx] = new TSWaveform();
		waveForms[wIx]->setHueFromKnob(initColorKnobs[wIx]);
		
		float defaultHue = initColorKnobs[wIx];
		float defaultFillHue = initColorKnobs[wIx];
		
		// Configure Parameters:
		// COLOR_PARAM,
		// EXTERNAL_PARAM = COLOR_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		// ROTATION_PARAM = EXTERNAL_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		// ROTATION_MODE_PARAM = ROTATION_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		// TIME_PARAM = ROTATION_MODE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		// TRIG_PARAM = TIME_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		// X_POS_PARAM = TRIG_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		// X_SCALE_PARAM = X_POS_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		// Y_POS_PARAM = X_SCALE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		// Y_SCALE_PARAM = Y_POS_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,
		// LINK_XY_SCALE_PARAM = Y_SCALE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,	   // Force Scale X = Scale Y.
		// OPACITY_PARAM = LINK_XY_SCALE_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,	   // Alpha channel
		// LISSAJOUS_PARAM = OPACITY_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,		   // For now always true
		// INFO_DISPLAY_TOGGLE_PARAM = LISSAJOUS_PARAM + TROWA_SCOPE_NUM_WAVEFORMS,		
		//configParam(/*id*/ multiScope::COLOR_PARAM + wIx, /*minVal*/ TROWA_SCOPE_HUE_KNOB_MIN, /*maxVal*/ TROWA_SCOPE_HUE_KNOB_MAX, /*defVal*/ rescale(defaultHue, 0, 1.0, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX), "Hue");		
		configParam(/*id*/ multiScope::COLOR_PARAM + wIx, /*minVal*/ TROWA_SCOPE_HUE_KNOB_MIN, /*maxVal*/ TROWA_SCOPE_HUE_KNOB_MAX, 
			/*defVal*/ rescale(defaultHue, 0, 1.0, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX), 
			/*label*/ "Hue", /*unit*/ " degrees", /*displayBase*/ 0, /*displayMultiplier*/ 18.0f, /*displayOffset*/ 180.0f);					
		configParam(/*id*/ multiScope::EXTERNAL_PARAM + wIx, /*minVal*/ 0, /*maxVal*/ 1, /*defVal*/ 0);		 // Not really used	
		configParam(/*id*/ multiScope::ROTATION_PARAM + wIx, TROWA_SCOPE_ROT_KNOB_MIN, TROWA_SCOPE_ROT_KNOB_MAX, 0, "Rotation");
		configParam(/*id*/ multiScope::ROTATION_MODE_PARAM + wIx, 0, 1, 0, "Rotation Absolute");
		configParam(/*id*/ multiScope::TIME_PARAM + wIx, TROWA_SCOPE_TIME_KNOB_MIN, TROWA_SCOPE_TIME_KNOB_MAX, TROWA_SCOPE_TIME_KNOB_DEF, "Time Scale");
		configParam(/*id*/ multiScope::TRIG_PARAM + wIx, TROWA_SCOPE_TIME_KNOB_MIN, TROWA_SCOPE_TIME_KNOB_MAX, TROWA_SCOPE_TIME_KNOB_DEF);	 // Not really used			
		configParam(/*id*/ multiScope::X_POS_PARAM + wIx, /*minVal*/ TROWA_SCOPE_POS_KNOB_MIN, /*maxVal*/ TROWA_SCOPE_POS_KNOB_MAX, /*defVal*/ TROWA_SCOPE_POS_X_KNOB_DEF, /*label*/ "X-Position");
		configParam(/*id*/ multiScope::X_SCALE_PARAM + wIx, /*minVal*/ TROWA_SCOPE_SCALE_KNOB_MIN, /*maxVal*/ TROWA_SCOPE_SCALE_KNOB_MAX, /*defVal*/ 1.0, /*label*/ "X-Scale");		
		configParam(/*id*/ multiScope::Y_POS_PARAM + wIx, /*minVal*/ TROWA_SCOPE_POS_KNOB_MIN, /*maxVal*/ TROWA_SCOPE_POS_KNOB_MAX, /*defVal*/ TROWA_SCOPE_POS_Y_KNOB_DEF, /*label*/ "Y-Position");
		configParam(/*id*/ multiScope::Y_SCALE_PARAM + wIx, /*minVal*/ TROWA_SCOPE_SCALE_KNOB_MIN, /*maxVal*/ TROWA_SCOPE_SCALE_KNOB_MAX, /*defVal*/ 1.0, /*label*/ "Y-Scale");
		configParam(/*id*/ multiScope::LINK_XY_SCALE_PARAM + wIx, /*minVal*/ 0, /*maxVal*/ 1, /*defVal*/ 0, /*label*/ "Link X-Y");
		configParam(/*id*/ multiScope::OPACITY_PARAM + wIx, /*minVal*/ TROWA_SCOPE_MIN_OPACITY, /*maxVal*/ TROWA_SCOPE_MAX_OPACITY, /*defVal*/ TROWA_SCOPE_MAX_OPACITY, /*label*/ "Opacity");
		configParam(/*id*/ multiScope::LISSAJOUS_PARAM + wIx, 0, 1, 1, "Toggle Lissajous Mode");	
	}
	return;
} // end multiScope()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// ~multiScope()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiScope::~multiScope()
{
	// Clean our stuff
	for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
	{
		delete waveForms[wIx];
	}
	return;
} // end multiScope()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// process()
// [Previously step(void)]
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiScope::process(const ProcessArgs &args) {
	if (!initialized)
		return;

	TSWaveform* waveForm = NULL;
	for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
	{			
		waveForm = waveForms[wIx]; // tmp pointer
		// Lissajous:
		if (waveForm->lissajousTrigger.process(params[multiScope::LISSAJOUS_PARAM + wIx].getValue()))
		{
			waveForm->lissajous = !waveForm->lissajous;
		}
		lights[multiScope::LISSAJOUS_LED + wIx].value = waveForm->lissajous;

		// Compute Color:
		float hue = 0;
		if(inputs[multiScope::COLOR_INPUT+wIx].isConnected()){
			hue = clamp(rescale(inputs[multiScope::COLOR_INPUT+wIx].getVoltage(), TROWA_SCOPE_HUE_INPUT_MIN_V, TROWA_SCOPE_HUE_INPUT_MAX_V, 0.0, 1.0), 0.0, 1.0);
		} else {
			hue = rescale(params[multiScope::COLOR_PARAM+wIx].getValue(), TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX, 0.0, 1.0);					
		}
		waveForm->colorChanged = hue != waveForm->waveHue || firstLoad; 
		if (waveForm->colorChanged)
		{
			waveForm->waveHue = hue;
			// nvgHSLA(waveForm->waveHue, 0.5, 0.5, 0xdd);		
			waveForm->waveColor = HueToColor(waveForm->waveHue); // Base Color (opacity full)
#if TROWA_SCOPE_USE_COLOR_LIGHTS
			// Change the light color:
			waveForm->waveLight->setColor(waveForm->waveColor);
#endif
		}

		// Opacity:
		if (inputs[multiScope::OPACITY_INPUT + wIx].isConnected())
		{
			waveForm->waveOpacity = clamp(rescale(inputs[multiScope::OPACITY_INPUT + wIx].getVoltage(), TROWA_SCOPE_OPACITY_INPUT_MIN, TROWA_SCOPE_OPACITY_INPUT_MAX, TROWA_SCOPE_MIN_OPACITY, TROWA_SCOPE_MAX_OPACITY),
									 TROWA_SCOPE_MIN_OPACITY, TROWA_SCOPE_MAX_OPACITY);
		}
		else
		{
			waveForm->waveOpacity = params[multiScope::OPACITY_PARAM + wIx].getValue();
		}
		
		// Compute rotation:
		waveForm->rotKnobValue = params[multiScope::ROTATION_PARAM+wIx].getValue();
		if (waveForm->rotModeTrigger.process(params[multiScope::ROTATION_MODE_PARAM+wIx].getValue()))
		{
			waveForm->rotMode = !waveForm->rotMode;
		}
		lights[multiScope::ROT_LED+wIx].value = waveForm->rotMode;		
		float rot = 0;
		float rotRate = 0;
		if (waveForm->rotMode)
		{
			// Absolute position:
			rot = rescale(params[multiScope::ROTATION_PARAM+wIx].getValue() + inputs[multiScope::ROTATION_INPUT+wIx].getVoltage(), 0, 10, 0, NVG_PI);
		}
		else
		{
			// Differential rotation
			rotRate = rescale(params[multiScope::ROTATION_PARAM+wIx].getValue() + inputs[multiScope::ROTATION_INPUT+wIx].getVoltage(), 0, 10, 0, 0.5);
		}
		waveForm->rotAbsValue = rot;
		waveForm->rotDiffValue = rotRate;
		
		// Compute time:
		float deltaTime = powf(2.0, params[TIME_PARAM+wIx].getValue() + inputs[TIME_INPUT+wIx].getVoltage());
		int frameCount = (int)ceilf(deltaTime * args.sampleRate);
		// Add frame to buffer
		if (waveForm->bufferIndex < BUFFER_SIZE) {
			if (++(waveForm->frameIndex) > frameCount) {
				waveForm->frameIndex = 0;
				waveForm->bufferX[waveForm->bufferIndex] = inputs[X_INPUT+wIx].getVoltage();
				waveForm->bufferY[waveForm->bufferIndex] = inputs[Y_INPUT+wIx].getVoltage();
				waveForm->bufferPenOn[waveForm->bufferIndex] = (!inputs[PEN_ON_INPUT + wIx].isConnected() || inputs[PEN_ON_INPUT + wIx].getVoltage() > 0.1); // Allow some noise?
				waveForm->bufferIndex++;
			}
		}
		else {
			if (waveForm->lissajous)
			{
				// Reset
				waveForm->bufferIndex = 0;
				waveForm->frameIndex = 0;
			}
			else
			{
				// Just show stuff (no trigger inputs)
				waveForm->frameIndex++;
				float holdTime = 0.1;
				if (waveForm->frameIndex >= args.sampleRate * holdTime) {
					waveForm->bufferIndex = 0; 
					waveForm->frameIndex = 0;
				}
			}
		}
	} // end loop through waveforms
	firstLoad = false;
	return;
} // end step()

#endif // end if use old scope