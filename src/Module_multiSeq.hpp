#ifndef MODULE_SEQ_HPP
#define MODULE_SEQ_HPP

#include <rack.hpp>
using namespace rack;

#include "TSSequencerModuleBase.hpp"
#include "TSSequencerWidgetBase.hpp"
#include "TSParamQuantity.hpp"
#include "Module_voltSeq.hpp"
#include "trowaSoftComponents.hpp"

// multiSeq model.
extern Model *modelMultiSeq64;

#define SEQ_NUM_VALUE_MODES			 6 // This one should use all 6 modes (TRIG, RTRG, GATE, VOLT, NOTE, PATT).
#define SEQ_NUM_STEPS				64
#define	SEQ_NUM_ROWS				 8
#define	SEQ_NUM_COLS				 8

#define  MULTISEQ_STEP_KNOB_MIN		voltSeq_STEP_KNOB_MIN
#define  MULTISEQ_STEP_KNOB_MAX		voltSeq_STEP_KNOB_MAX

// Simple Pattern Value mode (basically 0-63, displayed 1 to 64).
// Doesn't try to use 'voltages'.
extern ValueSequencerMode* SimplePatternValueMode;

#define DEBUG_TSSWITCH_KNOB		0

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Sequencer with voltSeq and trigSeq combined.
// Also should implement:
// * 'Song Mode' -- configure which patterns to play so you don't need
//   another voltSeq to arrange the pattern.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct multiSeq : TSSequencerModuleBase
{
	// Array of values of what we last sent over OSC (for comparison).
	float* oscLastSentVals = NULL;
	
	// What to show on our grid. Normally we are showing the steps but could be the pattern order.
	enum MatrixShown : uint8_t
	{
		// Default. Show the step matrix for the channel being edited.
		ShowEditStepMatrix,
		// Show the pattern matrix for song mode being edited.		
		ShowEditPatternMatrix
	};
	// The current mode for what we show on our matrix.
	MatrixShown currentMatrixShowMode = MatrixShown::ShowEditStepMatrix;
	
	// Value modes this supports.
	ValueSequencerMode* ValueModes[SEQ_NUM_VALUE_MODES] = 
	{ 
		// TRIG (simple bool)
		new ValueSequencerMode(/*displayName*/ "TRIG", /*unit*/ "",
			/*minDisplayValue*/ 0, /*maxDisplayValue*/ 1, 
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX, 
			/*outVoltageMin*/ 0, /*outVoltageMax*/ 10, 
			/*whole numbers*/ true, 
			/*zeroPointAngle*/ TROWA_ANGLE_STRAIGHT_UP_RADIANS,
			/*display format String */ "%1.0f",
			/*roundDisplay*/ 1, /*roundOutput*/ 0,
			/*zeroValue*/ 0.0f,  //voltSeq_STEP_KNOB_MIN
			/*outputIsBoolean*/ true),	
		// RTRG (simple bool)
		new ValueSequencerMode(/*displayName*/ "RTRG", /*unit*/ "",
			/*minDisplayValue*/ 0, /*maxDisplayValue*/ 1, 
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX, 
			/*outVoltageMin*/ 0, /*outVoltageMax*/ 10, 
			/*whole numbers*/ true, 
			/*zeroPointAngle*/ TROWA_ANGLE_STRAIGHT_UP_RADIANS,
			/*display format String */ "%1.0f",
			/*roundDisplay*/ 1, /*roundOutput*/ 0,
			/*zeroValue*/ 0.0f,  //voltSeq_STEP_KNOB_MIN
			/*outputIsBoolean*/ true),				
		// GATE (simple bool)
		new ValueSequencerMode(/*displayName*/ "GATE", /*unit*/ "",
			/*minDisplayValue*/ 0, /*maxDisplayValue*/ 1, 
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX, 
			/*outVoltageMin*/ 0, /*outVoltageMax*/ 10, 
			/*whole numbers*/ true, 
			/*zeroPointAngle*/ TROWA_ANGLE_STRAIGHT_UP_RADIANS,
			/*display format String */ "%1.0f",
			/*roundDisplay*/ 1, /*roundOutput*/ 0,
			/*zeroValue*/ 0.0f,  //voltSeq_STEP_KNOB_MIN
			/*outputIsBoolean*/ true),			
	
		// Voltage Mode 
		new ValueSequencerMode(/*displayName*/ "VOLT", /*unit*/ "V",
			/*minDisplayValue*/ -10, /*maxDisplayValue*/ 10, 
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX, 
			/*outVoltageMin*/ -10, /*outVoltageMax*/ 10, 
			/*whole numbers*/ false, 
			/*zeroPointAngle*/ TROWA_ANGLE_STRAIGHT_UP_RADIANS,
			/*display format String */ "%04.2f",
			/*roundDisplay*/ 0, /*roundOutput*/ 0,
			/*zeroValue*/ (voltSeq_STEP_KNOB_MAX+voltSeq_STEP_KNOB_MIN)/2.0),
		
		// Note mode (1 octave per V; 10 octaves)
		new NoteValueSequencerMode(/*displayName*/ "NOTE",			
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX),
			
		// Sequence Mode (1-64 for the patterns)			
		new ValueSequencerMode(/*displayName*/ "PATT",  /*unit*/ "pattern",
			/*minDisplayValue*/ 1, /*maxDisplayValue*/ TROWA_SEQ_NUM_PATTERNS, 
			/*inVoltageMin*/ voltSeq_STEP_KNOB_MIN, /*inVoltageMax*/ voltSeq_STEP_KNOB_MAX, 
			/*outVoltageMin*/ TROWA_SEQ_PATTERN_MIN_V, /*outVoltageMax*/ TROWA_SEQ_PATTERN_MAX_V, 
			/*whole numbers*/ false, 
			/*zeroPointAngle*/ 0.67*NVG_PI, 
			/*display format String */ "%02.0f",
			/*roundDisplay*/ 0, /*roundOutput*/ 0,
			/*zeroValue*/ voltSeq_STEP_KNOB_MIN,
			/*outputIsBoolean*/ false,
			/*displayIsInt*/ true)			
	};	
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// multiSeq()
	// Create a multiSeq with the given number of steps layed out in the number of rows
	// and columns specified.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	multiSeq(int numSteps, int numRows, int numCols);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// multiSeq()
	// Create a multiSeq with the default # steps and layout.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-			
	multiSeq() : multiSeq(SEQ_NUM_STEPS, SEQ_NUM_ROWS, SEQ_NUM_COLS)
	{
		return;
	}	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// ~multiSeq()
	// Destroy.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-				
	~multiSeq()
	{
		if (oscLastSentVals != NULL)
			delete [] oscLastSentVals;
	}	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Set a single the step value
	// (i.e. this command probably comes from an external source).
	// >> Should set the control knob value too if applicable. <<
	// @step : (IN) The step number to edit (0 to maxSteps).
	// @val : (IN) The step value.
	// @channel : (IN) The channel to edit (0 to TROWA_SEQ_NUM_CHNLS - 1).
	// @pattern: (IN) The pattern to edit (0 to TROWA_SEQ_NUM_PATTERNS - 1).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void setStepValue(int step, float val, int channel, int pattern) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// process()
	// [Previously step(void)]
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void process(const ProcessArgs &args) override;
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getValueSeqChannelModes()
	// Gets the array of ValueSequencerModes if any.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	ValueSequencerMode** getValueSeqChannelModes() override
	{
		return ValueModes;
	}

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// randomize()
	// Only randomize the current gate/trigger steps by default OR the 
	// the pattern sequences if that is shown.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	virtual void onRandomize(const RandomizeEvent& e) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getRandomValue()
	// Get a random value for a step in this sequencer.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	float getRandomValue(int channelIx) override;
	// Configure the value mode parameters on the steps.
	void configValueModeParam();

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// getRandomValue()
	// Get a random value for a step in this sequencer.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	float getRandomValue() override {
		return voltSeq_STEP_KNOB_MIN + random::uniform()*(voltSeq_STEP_KNOB_MAX - voltSeq_STEP_KNOB_MIN);
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// onShownStepChange()
	// If we changed a step that is shown on the matrix, then do something.
	// For voltSeq to adjust the knobs so we dont' read the old knob values again.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void onShownStepChange(int step, float val) override 
	{
		this->params[CHANNEL_PARAM + step].setValue(val);
		// int r = step / numRows;
		// int c = step % numRows;
		//knobStepMatrix[r][c]->setKnobValue(val);
		return;
	}
	// Get the toggle step value
	float getToggleStepValue(int step, float val, int channel, int pattern) override;
	// Calculate a representation of all channels for this step
	float getPlayingStepValue(int step, int pattern) override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Shift all steps (+/-) some number of volts.
	// @patternIx : (IN) The index into our pattern matrix (0-15). Or TROWA_INDEX_UNDEFINED for all patterns.
	// @channelIx : (IN) The index of the channel (gate/trigger/voice) if any (0-15, or TROWA_SEQ_COPY_CHANNELIX_ALL/TROWA_INDEX_UNDEFINED for all).
	// @volts: (IN) The number of volts to add.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void shiftValues(/*in*/ int patternIx, /*in*/ int channelIx, /*in*/ float volts);
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// dataFromJson(void)
	// Read in our junk from json.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	virtual void dataFromJson(json_t *rootJ) override
	{
		this->allowPatternSequencing = true; // make sure this is set
		this->TSSequencerModuleBase::dataFromJson(rootJ);
		return;
	}
};




//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Alternates functionality between a switch and a knob.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSwitchKnob : TS_BaseKnob
{
	// This control's id/step index.
	int id = -1;
	// Group id (to match guys that should respond to mouse down drag).
	int groupId = -1;
	/** Return to original position when released */
	bool momentary = false;
	/** Hysteresis state for momentary switch */
	bool momentaryPressed = false;
	bool momentaryReleased = false;
	// If this control is for a normal step, not a pattern step.
	bool isStepValue = true;
	// Pointer to the sequencer module (already type cast).
	TSSequencerModuleBase* seqModule = NULL;
	
	bool allowRandomize = true;
	
	
	enum FunctionType : uint8_t
	{
		Switch,
		Knob
	};
	
	FunctionType currentFunctionType = FunctionType::Switch;
	
	void setFunctionType(FunctionType type)
	{
		currentFunctionType = type;
		return;
	}
	
	void setAsActiveStepEdit()
	{
		if (seqModule != NULL)
		{
			if (isStepValue)
			{
#if DEBUG_PATT_SEQ				
//				DEBUG("+> Set id %d as Active Edit Step (ParamId %d). Value = %4.2f.", id, paramId, paramQuantity->getValue());
#endif				
				seqModule->currentStepBeingEditedIx = id;
				seqModule->currentStepBeingEditedParamId = paramId;// paramQuantity->paramId;				
			}
			else
			{
#if DEBUG_PATT_SEQ
//				DEBUG("+> Set id %d as Active Edit PATTERN SEQ (ParamId %d). Value = %4.2f.", id, paramId, paramQuantity->getValue());			
#endif
				// The current pattern index being edited (index into patternData).
				seqModule->currentPatternDataBeingEditedIx = id;
				// The current pattern param id being edited.				
				seqModule->currentPatternDataBeingEditedParamId = paramId;// paramQuantity->paramId;							
			}
		}
		return;
	}
	void removeAsActiveStepEdit()
	{
		if (seqModule != NULL)
		{
			if (isStepValue)
			{
#if DEBUG_PATT_SEQ				
				DEBUG("-> Remove id %d as Active Edit Step.", id);
#endif				
				seqModule->currentStepBeingEditedIx = -1;
				seqModule->currentStepBeingEditedParamId = -1;				
			}
			else
			{
#if DEBUG_PATT_SEQ
				DEBUG("-> Remove id %d as Active Edit PATTERN SEQ.", id);			
#endif
				// The current pattern index being edited (index into patternData).
				seqModule->currentPatternDataBeingEditedIx = -1;
				// The current pattern param id being edited.				
				seqModule->currentPatternDataBeingEditedParamId = -1;				
			}
		}
		return;
	}	
	
	void step() override
	{
		if (currentFunctionType == FunctionType::Switch)
		{
			if (momentaryPressed) {
				momentaryPressed = false;
				// Wait another frame.
			}
			else if (momentaryReleased) {
				momentaryReleased = false;
				ParamQuantity* paramQuantity = getParamQuantity();
				if (paramQuantity) {
					// Set to minimum value
					paramQuantity->setMin();
				}
			}
			ParamWidget::step();			
		}
		else
		{
			TS_BaseKnob::step();
		}
		return;
	}

	// void randomize() override
	// {
		// if (allowRandomize && paramQuantity && !momentary) 
		// {
			// float value = paramQuantity->getMinValue() + std::floor(random::uniform() * (paramQuantity->getRange() + 1));
			// paramQuantity->setValue(value);
		// }
		// return;
	// }
	
	float getNewSwitchValue()
	{
		ParamQuantity* paramQuantity = getParamQuantity();
		if (paramQuantity)			
			return (paramQuantity->getValue() > paramQuantity->minValue) ? paramQuantity->minValue : paramQuantity->maxValue;
		return 0.0f;
	}

	void onDragMove(const event::DragMove& e) override 
	{
		if (e.button != GLFW_MOUSE_BUTTON_LEFT || !visible)
			return;	
		//DEBUG("onDragMove(%d) fired (Left Click = %d, Current Value is %3.1f).", id, leftClick, paramQuantity->getValue());		
		if (currentFunctionType == FunctionType::Switch)
			return;		
		TS_BaseKnob::onDragMove(e);
		return;
	}	
	
	// Allow mouse-down & drag to set buttons (i.e. on Sequencer grid where there are many buttons). 
	/** Called when a widget responds to `onMouseDown` for a left button press */
	void onDragStart(const event::DragStart &e) override 
	{
		if (e.button != GLFW_MOUSE_BUTTON_LEFT || !visible)
			return;		
#if DEBUG_PATT_SEQ		
	//	DEBUG("onDragSTART(%d) fired (Current Value is %3.1f). Selected Widget = %d.", id, paramQuantity->getValue(), APP->event->selectedWidget == this);		
#endif
		ParamQuantity* paramQuantity = getParamQuantity();
		if (paramQuantity)
		{
			if (currentFunctionType == FunctionType::Knob)
			{
				TS_BaseKnob::onDragStart(e);	
				setAsActiveStepEdit();
#if DEBUG_PATT_SEQ				
				DEBUG("onDragSTART(%d) - Knob set to %3.1f..", id, paramQuantity->getValue());
#endif				
			}
			else 
			{
				if (momentary)
				{
#if DEBUG_PATT_SEQ			
					DEBUG("onDragSTART(%d) - Momentary - Set Value to %3.1f.", id, paramQuantity->maxValue);
#endif					
					paramQuantity->setValue(paramQuantity->maxValue); // Trigger Value										
				}
				else
				{
					float newVal = getNewSwitchValue();
#if DEBUG_PATT_SEQ
					DEBUG("onDragSTART(%d) - Set Value to %3.1f. (Value was %3.f). Selected Widget = %d.", id, newVal, paramQuantity->getValue(), APP->event->selectedWidget == this);						
#endif
					paramQuantity->setValue(newVal); // Toggle Value
					setAsActiveStepEdit();						
				}					
			}
		}
		return;
	}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	void onDragEnter(const event::DragEnter &e) override 
	{	
		if (e.button != GLFW_MOUSE_BUTTON_LEFT || !visible)
			return;			
		TSSwitchKnob* origin = dynamic_cast<TSSwitchKnob*>(e.origin);					
		if (currentFunctionType == FunctionType::Knob)
		{
			if (origin && origin == this)
			{
				TS_BaseKnob::onDragEnter(e);
				setAsActiveStepEdit();
			}
		}
		else
		{
			ParamQuantity* paramQuantity = getParamQuantity();			
			// Set these no matter what because if you drag back onto your starting square, you want to toggle it again.
			if (origin && origin != this && origin->groupId == this->groupId && paramQuantity) 
			{
				float newVal = getNewSwitchValue();
				paramQuantity->setValue(newVal); // Toggle Value
				setAsActiveStepEdit();				
			}		
			
		}
		return;
	}
	void onDragLeave(const event::DragLeave &e) override 
	{
		if (e.button != GLFW_MOUSE_BUTTON_LEFT || !visible)
			return;				
		TSSwitchKnob* origin = dynamic_cast<TSSwitchKnob*>(e.origin);					
		if (currentFunctionType == FunctionType::Knob)
		{
			if (origin && origin == this)
			{
				TS_BaseKnob::onDragLeave(e);				
				//removeAsActiveStepEdit();
				//DEBUG("onDragLeave(%d) - Knob set to %3.1f..", id, this->paramQuantity->getValue());				
			}
		}
		else
		{
			ParamQuantity* paramQuantity = getParamQuantity();			
			if (origin && origin->groupId == this->groupId && paramQuantity) 
			{
				if (momentary)
				{
					//DEBUG("onDragLeave(%d) (momentary) - Set Value to %3.1f.", id, paramQuantity->minValue);
					paramQuantity->setValue(paramQuantity->minValue); // Turn Off				
				}
				else
				{
					//DEBUG("onDragLeave(%d) - Value is %3.1f. - DO NOTHING.", id, paramQuantity->getValue());
					//removeAsActiveStepEdit();					
				}
			}
		}
		return;
	}
	void onDragEnd(const event::DragEnd& e) override {
		if (e.button != GLFW_MOUSE_BUTTON_LEFT || !visible)
			return;
		//DEBUG("onDragEnd(%d) fired (Current Value is %3.1f). Selected Widget = %d.", id, paramQuantity->getValue(), APP->event->selectedWidget == this );		
		if (currentFunctionType == FunctionType::Knob)
		{
			TS_BaseKnob::onDragEnd(e);				
			removeAsActiveStepEdit();
			//DEBUG("onDragEnd(%d) - Knob set to %3.1f..", id, this->paramQuantity->getValue());				
		}
		else
		{			
			if (momentary) {
				momentaryReleased = true;
			}
			else {
				removeAsActiveStepEdit();
			}
		}
		return; 
	}	
	void onButton(const event::Button &e) override 
	{
		if (visible)
		{
			ParamWidget::onButton(e);
			//DEBUG("onButton(%d) fired (Current Value is %3.1f). Selected Widget = %d.", id, paramQuantity->getValue(), APP->event->selectedWidget == this);									
		}
		return;
	}	
};

struct TSContainerWidget : OpaqueWidget
{
	
	void setVisible(bool vis)
	{
		visible = vis;
		return;
	}
	
	void step() override 
	{
		if (!visible)
			return;
		OpaqueWidget::step();
		return;
	}
	void draw(const DrawArgs &args) override
	{
		if (visible)
		{
			OpaqueWidget::draw(args);
		}
		return;
	}
	
	void onHover(const event::Hover& e) override 
	{
		if (visible)
		{
			OpaqueWidget::onHover(e);
		}
	}
	void onButton(const event::Button& e) override 
	{
		if (visible)
		{
			OpaqueWidget::onButton(e);			
		}
	}
	void onHoverKey(const event::HoverKey& e) override 
	{
		if (visible)
		{
			OpaqueWidget::onHoverKey(e);
		}
	}
	void onHoverText(const event::HoverText& e) override 
	{
		if (visible)
		{
			OpaqueWidget::onHoverText(e);
		}
	}
	void onHoverScroll(const event::HoverScroll& e) override 
	{
		if (visible)
		{
			OpaqueWidget::onHoverScroll(e);
		}
	}
	void onDragHover(const event::DragHover& e) override 
	{
		if (visible)
		{
			OpaqueWidget::onDragHover(e);
		}
	}
	void onPathDrop(const event::PathDrop& e) override 
	{
		if (visible)
		{
			OpaqueWidget::onPathDrop(e);
		}
	}
};


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiSeqWidget
// Widget for the trowaSoft trig + volt seq in one.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct multiSeqWidget : TSSequencerWidgetBase 
{
	// Last value output mode
	short lastMode = -1;
	// Current value mode pointer.
	ValueSequencerMode* currValueModePtr = NULL;
	ValueSequencerMode* lastValueModePtr = NULL;
	
	// Container for all step objects.
	TSContainerWidget* stepContainer = NULL;
	// Container for all pattern step objects.
	TSContainerWidget* patternContainer = NULL;	
	
	// Pointers to step switch/knobs
	TSSwitchKnob*** padPtrs = NULL;	
	// Pointers to pattern sequencer step switch/knobs
	TSSwitchKnob*** patternSeqPtrs = NULL;
	// Pointers to pattern sequencer step switch/knob lights.
	TS_LightMeter*** patternSeqLightPtrs = NULL;
	
	bool lastShowPatternConfig = false;
	

	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// multiSeqWidget()
	// Widget for the trowaSoft trig + volt seq in one.
	// @seqModule : (IN) Pointer to the sequencer module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	multiSeqWidget(multiSeq* seqModule, int nSteps = SEQ_NUM_STEPS, int nRows = SEQ_NUM_ROWS, int nCols = SEQ_NUM_COLS);
	
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Destroy
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-		
	~multiSeqWidget()
	{
		if (padPtrs != NULL)
		{
			for (int r = 0; r < numRows; r++)
			{
				if (padPtrs[r])
				{
					delete[] padPtrs[r];
					padPtrs[r] = NULL;
				}
			}			
			delete[] padPtrs; 
			padPtrs = NULL;
		}
		if (patternSeqPtrs != NULL)
		{
			for (int r = 0; r < numRows; r++)
			{
				if (patternSeqPtrs[r])
				{
					delete[] patternSeqPtrs[r];
					patternSeqPtrs[r] = NULL;
				}
			}			
			delete[] patternSeqPtrs; 
			patternSeqPtrs = NULL;
		}
		if (patternSeqLightPtrs != NULL)
		{
			for (int r = 0; r < numRows; r++)
			{
				if (patternSeqLightPtrs[r])
				{
					delete[] patternSeqLightPtrs[r];
					patternSeqLightPtrs[r] = NULL;
				}
			}			
			delete[] patternSeqLightPtrs; 
			patternSeqLightPtrs = NULL;
		}		
		return;
	}		
	void step() override;
	void appendContextMenu(ui::Menu *menu) override;
};


#endif // !MODULE_SEQ_HPP