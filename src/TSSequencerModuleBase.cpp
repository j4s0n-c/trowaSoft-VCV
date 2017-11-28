#include <chrono>
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"


void TSSequencerModuleBase::copy(int patternIx, int gateIx)
{
	copySourceGateIx = gateIx;
	copySourcePatternIx = patternIx;
	if (copySourceGateIx == TROWA_SEQ_COPY_GATEIX_ALL)
	{
		// Copy entire pattern (all gates/triggers/voices)
		for (int g = 0; g < TROWA_SEQ_NUM_TRIGS; g++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				copyBuffer[g][s] = triggerState[copySourcePatternIx][g][s];
			}
		}		
	}
	else
	{
		// Copy just the gate:
		for (int s = 0; s < maxSteps; s++)
		{
			copyBuffer[copySourceGateIx][s] = triggerState[copySourcePatternIx][copySourceGateIx][s];
		}		
	}
	return;
}

// Paste our current clipboard Pattern/Gate to the currently selected Pattern/Gate.
bool TSSequencerModuleBase::paste() 
{
	if (copySourcePatternIx < 0) // Nothing to copy
		return false;
	if (copySourcePatternIx == currentPatternEditingIx)
	{
		// For some weird reason copySourceGateIx == currentPatternEditingIx will always return true even if it obviously isn't...
		//if (copySourceGateIx == currentPatternEditingIx || copySourceGateIx == TROWA_SEQ_COPY_GATEIX_ALL)
		if (copySourceGateIx == TROWA_SEQ_COPY_GATEIX_ALL)	
			return false; // This is the SAME as the target, nothing to copy
	}
	
	if (copySourceGateIx == TROWA_SEQ_COPY_GATEIX_ALL)
	{
		// Copy entire pattern (all gates/triggers/voices)
		for (int g = 0; g < TROWA_SEQ_NUM_TRIGS; g++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				triggerState[currentPatternEditingIx][g][s] = copyBuffer[g][s];//triggerState[copySourcePatternIx][g][s];
			}
		}
	}
	else
	{
		// Copy just the gate:
		for (int s = 0; s < maxSteps; s++)
		{
			triggerState[currentPatternEditingIx][currentTriggerEditingIx][s] = copyBuffer[copySourceGateIx][s];// triggerState[copySourcePatternIx][copySourceGateIx][s];
		}		
	}
	return true;
}

// Get the inputs shared between our Sequencers.
void TSSequencerModuleBase::getStepInputs(bool* pulse, bool* reloadMatrix, bool* valueModeChanged) 
{
	// Run
	if (runningTrigger.process(params[RUN_PARAM].value)) {
		running = !running;
	}
	//runningLight = running ? 1.0 : 0.0;
	lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;

	bool nextStep = false;
	if (running) 
	{
		if (inputs[EXT_CLOCK_INPUT].active) 
		{
			// External clock input
			if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value)) 
			{
				realPhase = 0.0;
				nextStep = true;
				//using namespace std::chrono;				
				// std::chrono::high_resolution_clock::time_point thisTime = std::chrono::high_resolution_clock::now();
				// if (lastStepWasExternalClock)
				// {
					// double sec = duration_cast<duration<double>>(thisTime - lastExternalStepTime).count(); 
					// currentBPM = roundf(15.0 / sec);
				// }				
				// Save our last values for external clock
				//lastExternalStepTime = thisTime;
				lastStepWasExternalClock = true;
			}
		}
		else 
		{
			// Internal clock
			lastStepWasExternalClock = false;
			float clockTime = 1.0;
			float input = 0.0;
			if (inputs[BPM_INPUT].active)
			{
				// Use whatever voltage we are getting (-10 TO 10 input)
				//input = inputs[BPM_INPUT].value;
				input = rescalef(inputs[BPM_INPUT].value, TROWASEQ_PATTERN_MIN_V, TROWASEQ_PATTERN_MAX_V, 
					TROWA_SEQ_BPM_KNOB_MIN, TROWA_SEQ_BPM_KNOB_MAX);
			}
			else
			{
				// Otherwise read our knob
				input = params[BPM_PARAM].value; // -2 to 6
			}
			clockTime = powf(2.0, input); // -2 to 6
			float dt = clockTime / engineGetSampleRate(); // Real dt
			int n = (index + 1) % swingResetSteps;
			// index 0 --> n = 1,
			// index 1 --> n = 2,
			// index 2 --> n = 3,
			// index 3 --> n = 0
			bool inSwing = swingAdjustment != 0 &&  n > 0;	// index 0, 1, 2 and 4, 5, 7
			realPhase += dt; // Real Time no matter what
			if (realPhase >= 1.0) 
			{
				realPhase -= 1.0;
				if ( swingAdjustment == 0 || swingRealSteps++ >= swingResetSteps ) // n == 0 We should be index 3, 7, 11, 15
				{
					nextStep = true; // Next step should be index 0, 4, 8, 12, 16
					swingAdjustedPhase = 0; // reset swing
					swingRealSteps = 0;					
				}								
			}			
			if (!nextStep && inSwing) // If not index 3, 7, 11, or 15
			{
				swingAdjustedPhase += dt * (1 + swingAdjustment * n / (swingResetSteps - 1));				
				if (swingAdjustedPhase >= 1.0)
				{
					swingAdjustedPhase -= 1.0;
					nextStep = true;
				}
			}
			
			if (nextStep)
			{
				currentBPM = roundf(clockTime * 30.0);				
			}
		}
	} // end if running

	// Reset
	if (resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].value)) 
	{
		realPhase = 0.0;
		swingAdjustedPhase = 0; // Reset swing		
		index = 999;
		nextStep = true;
		lights[RESET_LIGHT].value = 1.0;
	}
	
	// Swing Adjustment
	swingAdjustment = params[SWING_ADJ_PARAM].value;

	
	// Current Playing Pattern
	// If we get an input, then use that:
	if (inputs[SELECTED_PATTERN_PLAY_INPUT].active)
	{
		currentPatternPlayingIx = VoltsToPattern(inputs[SELECTED_PATTERN_PLAY_INPUT].value) - 1;
		//currentPatternPlayingIx = clampi(roundf(inputs[SELECTED_PATTERN_PLAY_INPUT].value / 6.0 * TROWASEQ_NUM_PATTERNS), 1, TROWASEQ_NUM_PATTERNS) - 1;
	}
	else
	{
		// Otherwise read our knob parameter and use that
		currentPatternPlayingIx = clampi(roundf(params[SELECTED_PATTERN_PLAY_PARAM].value), 0, TROWASEQ_NUM_PATTERNS - 1);
	}
	if (currentPatternPlayingIx < 0)
		currentPatternPlayingIx = 0;
	else if(currentPatternPlayingIx > TROWASEQ_NUM_PATTERNS - 1)
		currentPatternPlayingIx = TROWASEQ_NUM_PATTERNS - 1;
	
	// Current Edit Pattern
	int lastEditPatternIx = currentPatternEditingIx;
	// From User Knob:
	currentPatternEditingIx = clampi(roundf(params[SELECTED_PATTERN_EDIT_PARAM].value), 0, TROWASEQ_NUM_PATTERNS - 1);
	if (currentPatternEditingIx < 0)
		currentPatternEditingIx = 0;
	else if(currentPatternEditingIx > TROWASEQ_NUM_PATTERNS - 1)
		currentPatternEditingIx = TROWASEQ_NUM_PATTERNS - 1;

	// Gate inputs (which gate we are displaying & editing)
	int lastGateIx = currentTriggerEditingIx;
	currentTriggerEditingIx = clampi(roundf(params[SELECTED_GATE_PARAM].value), 0, TROWA_SEQ_NUM_TRIGS - 1);
	if (currentTriggerEditingIx < 0)
		currentTriggerEditingIx = 0;
	else if (currentTriggerEditingIx > TROWA_SEQ_NUM_TRIGS - 1)
		currentTriggerEditingIx = TROWA_SEQ_NUM_TRIGS - 1;
	
	bool pasteCompleted = false;
	if (pasteTrigger.process(params[PASTE_PARAM].value))
	{
		pasteCompleted = paste(); // Paste whatever we have if we have anything		
	}
	else
	{
		// Check Copy
		if (copyPatternTrigger.process(params[COPY_PATTERN_PARAM].value))
		{
			if (copySourcePatternIx > -1 && copySourceGateIx == TROWA_SEQ_COPY_GATEIX_ALL)
			{
				// Clear clipboard 
				clearClipboard();
			}
			else
			{
				copy(currentPatternEditingIx, TROWA_SEQ_COPY_GATEIX_ALL);
				lights[PASTE_LIGHT].value = 1;	// Activate paste light to show there is something on the clipboard
				pasteLight->setColor(COLOR_WHITE);
				lights[COPY_PATTERN_LIGHT].value = 1; // Light up Pattern Copy as Active clipboard
				lights[COPY_GATE_LIGHT].value = 0;	  // Inactivate Gate Copy light				
			}
		}
		if (copyGateTrigger.process(params[COPY_GATE_PARAM].value))
		{
			if (copySourcePatternIx > -1 && copySourceGateIx > -1)
			{
				// Clear clipboard 
				clearClipboard();
			}
			else 
			{
				copy(currentPatternEditingIx, currentTriggerEditingIx);
				lights[PASTE_LIGHT].value = 1;	// Activate paste light to show there is something on the clipboard
				pasteLight->setColor(voiceColors[currentTriggerEditingIx]);
				lights[COPY_GATE_LIGHT].value = 1;		// Light up Gate Copy Light as Active clipboard
				copyGateLight->setColor(voiceColors[currentTriggerEditingIx]); // Match the color with our Trigger color
				lights[COPY_PATTERN_LIGHT].value = 0; // Inactivate Pattern Copy Light				
			}
		} // end if copyGateTrigger()
	}
	
	
	
	int r = 0;
	int c = 0;

	// Current output value mode	
	selectedOutputValueMode = static_cast<ValueMode>(clampi(roundf(params[SELECTED_OUTPUT_VALUE_MODE_PARAM].value), 0, TROWA_SEQ_NUM_MODES - 1));
	*valueModeChanged = (lastOutputValueMode != selectedOutputValueMode);	
	lastOutputValueMode = selectedOutputValueMode;
	
	if (inputs[STEPS_INPUT].active)
	{
		// Use the input if something is connected.
		currentNumberSteps = clampi(roundf(rescalef(inputs[STEPS_INPUT].value, TROWASEQ_STEPS_MIN_V, TROWASEQ_STEPS_MAX_V, 1, maxSteps)), 1, maxSteps);
	}
	else
	{
		// Otherwise read our knob
		currentNumberSteps = clampi(roundf(params[STEPS_PARAM].value), 1, maxSteps);		
	}
	if (nextStep)	
	{
		// Advance step
		index += 1;
		if (index >= currentNumberSteps) {
			index = 0; // Reset (artifical limit)
		}
		// Show which step we are on:
		r = index / this->numCols;// TROWA_SEQ_STEP_NUM_COLS;
		c = index % this->numCols; //TROWA_SEQ_STEP_NUM_COLS;
		stepLights[r][c] = 1.0f;
		//lights[PAD_LIGHTS + index].value = stepLights[r][c];	
		gatePulse.trigger(1e-3);		
	}
	// Reset light
	lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / lightLambda / engineGetSampleRate();
	*pulse = gatePulse.process(1.0 / engineGetSampleRate());
	
	// See if we should reload our matrix
	*reloadMatrix = currentTriggerEditingIx != lastGateIx || currentPatternEditingIx != lastEditPatternIx || pasteCompleted;
		
	
	return;

}