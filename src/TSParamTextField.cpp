#include "TSParamTextField.hpp"
#include <string>

//-----------------------------------------------------------------------------------------------
// TSParamTextField()
// @textType: (IN) Text type for this field (for validation). Should be a value type.
// @maxLength: (IN) Max length for this field.
// @paramCtl: (IN) The widget for control.
// @formatStr: (IN) The format string.
//-----------------------------------------------------------------------------------------------
TSParamTextField::TSParamTextField(TextType textType, int maxLength, ParamWidget* paramCtl, const char* formatStr) : TSTextField(textType, maxLength)
{
	this->control = paramCtl;
	FramebufferWidget* fbw = dynamic_cast<FramebufferWidget*>(paramCtl);
	if (fbw != NULL)
	{
		isDirty = &(fbw->dirty);
		isBufferedCtrl = true;
	}
	this->formatString = formatStr;
	return;
}

//-----------------------------------------------------------------------------------------------
// saveValue()
// Validate input and save value (valid values only).
//-----------------------------------------------------------------------------------------------
void TSParamTextField::saveValue()
{
	isEditing = 2; // Wait 2 cycles before setting knob -> text again in step()
	char buffer[50] = { 0 };
	if (control != NULL && control->getParamQuantity() != NULL)
	{
		ParamQuantity* cParamQty = control->getParamQuantity();
		float controlVal = cParamQty->getValue();
		if (isValid()) {
			// Set the value on the control:
			float val = (text.length() > 0) ? std::stof(text.c_str()) : 0.0f;
			if (text2KnobVal != NULL)
				controlVal = text2KnobVal(val);
			else
				controlVal = val;
			if (controlVal < cParamQty->getMinValue())
			{
				val = (knob2TextVal == NULL) ? cParamQty->getMinValue() : knob2TextVal(cParamQty->getMinValue());
				controlVal = cParamQty->getMinValue();
			}
			else if (controlVal > cParamQty->getMaxValue())
			{
				val = (knob2TextVal == NULL) ? cParamQty->getMaxValue() : knob2TextVal(cParamQty->getMaxValue());
				controlVal = cParamQty->getMaxValue();
			}
			cParamQty->setValue(controlVal);
			if (isBufferedCtrl && isDirty != NULL)
			{
				*isDirty = true; // Set dirty flag to redraw
			}
		}
		lastControlVal = controlVal;
		if (knob2TextVal != NULL)
			sprintf(buffer, formatString, knob2TextVal(controlVal));
		else
			sprintf(buffer, formatString, controlVal);
		text = buffer;
	}
	return;
}

//-----------------------------------------------------------------------------------------------
// onAction()
// Save value if valid.
//-----------------------------------------------------------------------------------------------
void TSParamTextField::onAction(const event::Action &e)
{
	//DEBUG("onAction() - visible = %d!", visible);
	if (visible)
	{
		saveValue();
		//e.consumed = true;
		e.consume(this);
	}
	return;
}
//-----------------------------------------------------------------------------------------------
// onDefocus()
// Validate input, set control value to match, format the text field number.
//-----------------------------------------------------------------------------------------------
void TSParamTextField::onDeselect(const event::Deselect &e)
{
//DEBUG("TSParamTextField::onDeselect(%d)", id);
	saveValue();
	if (autoHideMode == AutoHideMode::AutoHideOnDefocus) {
		visible = false;
	}
	isEditing = 2; // Wait one cycle before setting knob -> text again in step()
	//e.consumed = true;
	e.consume(this);
	return; 
} // end onDefocus()
//-----------------------------------------------------------------------------------------------
// step()
// Set value to match the control.
//-----------------------------------------------------------------------------------------------
void TSParamTextField::step()
{
	if (control != NULL && !isEditing && control->getParamQuantity() != NULL)
	{
		float val = control->getParamQuantity()->getValue();
		if (val != lastControlVal)
		{
			char buffer[50] = { 0 };

			if (knob2TextVal != NULL)
				sprintf(buffer, formatString, knob2TextVal(val));
			else
				sprintf(buffer, formatString, val);
			text = buffer;

			lastControlVal = val;
		}
	}
	else if (isEditing < 3 && isEditing > 0)
		isEditing--;
	return;
}
//-----------------------------------------------------------------------------------------------
// setText()
// @val : (IN) Float value to set the text.
// Uses the format string.
//-----------------------------------------------------------------------------------------------
void TSParamTextField::setText(float val)
{
	char buffer[50] = { 0 };
	float controlVal = val;
	if (control != NULL && control->getParamQuantity() != NULL)
	{
		ParamQuantity* cParamQty = control->getParamQuantity();		
		if (text2KnobVal != NULL)
			controlVal = text2KnobVal(val);
		else
			controlVal = val;
		if (controlVal < cParamQty->getMinValue())
			val = (knob2TextVal == NULL) ? cParamQty->getMinValue() : knob2TextVal(cParamQty->getMinValue());
		else if (controlVal > cParamQty->getMaxValue())
			val = (knob2TextVal == NULL) ? cParamQty->getMaxValue() : knob2TextVal(cParamQty->getMaxValue());
	}
	// Format the text
	sprintf(buffer, formatString, val);
	text = buffer;
	return;
}
