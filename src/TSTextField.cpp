#include "TSTextField.hpp"
#include <widget/Widget.hpp> //#include "widgets.hpp"
#include "ui.hpp"
// for gVg
#include "window.hpp"
// for key codes
#include <GLFW/glfw3.h>
using namespace rack;

#include "trowaSoftComponents.hpp"

TSTextField::TSTextField(TextType textType) : TextField() {
	multiline = false; // Default to no multiline.
	setTextType(textType);
	font = APP->window->loadFont(asset::plugin(pluginInstance, TROWA_MONOSPACE_FONT));
	fontSize = 14.0f;
	backgroundColor = FORMS_DEFAULT_BG_COLOR;
	color = FORMS_DEFAULT_TEXT_COLOR;
	textOffset = Vec(0, 0);
	borderWidth = 1;
	borderColor = FORMS_DEFAULT_BORDER_COLOR;
	//caretColor = COLOR_TS_RED;// nvgRGBAf(1.0f - color.r, 1.0f - color.g, 1.0f - color.b, 0.70);
	caretColor = nvgRGBAf((color.r + backgroundColor.r) / 2.0, (color.g + backgroundColor.g) / 2.0, (color.b + backgroundColor.b) / 2.0, 0.70);
	return;
}
TSTextField::TSTextField(TextType textType, int maxLength) : TSTextField(textType) {
	this->maxLength = maxLength;
	return;
}


// Taken from Rack's LEDTextField
int TSTextField::getTextPosition(Vec mousePos) {
	bndSetFont(font->handle);
	int textPos = bndIconLabelTextPosition(APP->window->vg, textOffset.x, textOffset.y,
		box.size.x - 2 * textOffset.x, box.size.y - 2 * textOffset.y,
		-1, fontSize, displayStr.c_str(), mousePos.x, mousePos.y);
	bndSetFont(APP->window->uiFont->handle);	
	return textPos;

}

void TSTextField::setVisible(bool isVisible){
	this->visible = isVisible;
	return;
}

// Draw if visible.
void TSTextField::draw(const DrawArgs &args) 
{
	if (this->visible)
	{
		// Draw taken from Rack's LEDTextField and modified for scrolling (my quick & dirty ghetto text scrolling---ONLY truly effective for calculating the width with MONOSPACE font
		// since I don't want to do a bunch of calcs... [lazy]).
		nvgScissor(args.vg, 0, 0, box.size.x, box.size.y);

		// Background
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 5.0);
		nvgFillColor(args.vg, backgroundColor);
		nvgFill(args.vg);

		// Border
		if (borderWidth > 0) {
			nvgStrokeWidth(args.vg, borderWidth);
			nvgStrokeColor(args.vg, borderColor);
			nvgStroke(args.vg);
		}

		// Text
		if (font->handle >= 0) {
			bndSetFont(font->handle);
			
			
			BNDwidgetState state;
			if (this == APP->event->selectedWidget)
				state = BND_ACTIVE;
			else if (this == APP->event->hoveredWidget)
				state = BND_HOVER;
			else
				state = BND_DEFAULT;			

			//NVGcolor highlightColor = color;
			//highlightColor.a = 0.5;
			int begin = std::min(cursor, selection);
			int end = (state == BND_ACTIVE) ? std::max(cursor, selection) : -1;

			// Calculate overflow and the displayed text (based on bounding box)
			// Currently the scrolling should work for any font, **BUT** the width calculation is only really good for monospace.
			float txtBounds[4] = { 0,0,0,0 };
			nvgTextAlign(args.vg, NVG_ALIGN_LEFT);
			nvgFontSize(args.vg, fontSize);
			nvgFontFaceId(args.vg, font->handle);
			int maxTextWidth = box.size.x - textOffset.x * 2 - fontSize / 2.0; // There should be a caret
			float estLetterSize = nvgTextBounds(args.vg, 0, 0, "X", NULL, txtBounds); // Estimate size of a letter (accurate for monospace)
			float nextX = nvgTextBounds(args.vg, 0, 0, text.c_str(), NULL, txtBounds); // Calculate full string size

			displayStr = text;
			if (nextX > maxTextWidth) {
				int nChars = maxTextWidth / estLetterSize - 1;
				if (nChars < 1)
					nChars = 1;

				if (state == BND_ACTIVE) {
					int lastIx = (cursor > nChars) ? cursor : nChars;
					int startIx = clamp(lastIx - nChars, 0, lastIx);
					displayStr = text.substr(startIx, nChars);
					begin -= startIx;
					if (end > -1)
						end -= startIx;
				}
				else {
					displayStr = text.substr(0, nChars);
				}
			}


			// The caret color actually isn't the cursor color (that is hard-coded as nvgRGBf(0.337,0.502,0.761))
			// 

			//void bndIconLabelCaret(NVGcontext *ctx, float x, float y, float w, float h,
			//	int iconid, NVGcolor color, float fontsize, const char *label,
			//	NVGcolor caretcolor, int cbegin, int cend
			bndIconLabelCaret(args.vg, /*x*/ textOffset.x, /*y*/ textOffset.y,
				/*w*/ box.size.x - 2 * textOffset.x, /*h*/ box.size.y - 2 * textOffset.y,
				/*iconid*/ -1, /*textColor*/ color, /*fontsize*/ fontSize, 
				/*label*/ displayStr.c_str(), 
				/*caretcolor*/ caretColor, /*cbegin*/ begin, /*cend*/ end);

			bndSetFont(APP->window->uiFont->handle);
		}

		nvgResetScissor(args.vg);
	}
} // end draw()

// Request focus on this field.
void TSTextField::requestFocus() {
	if (APP->event->selectedWidget) {
		event::Deselect evt;
		APP->event->selectedWidget->onDeselect(evt);
		APP->event->selectedWidget = NULL;
	}
	APP->event->selectedWidget = this;
	{
//DEBUG("TSTextField::requestFocus(%d) - Select this object, visible = %d", id, visible);					
		event::Select evt;
		onSelect(evt);
		cursor = 0;
		selection = text.length();
		visible = true;
//DEBUG("TSTextField::requestFocus(%d) - Select this object Finished. cursor = %d, selection = %d", id, cursor, selection);
	}
	return;
} // end requestFocus()

// Remove invalid chars from input.
std::string TSTextField::cleanseString(std::string newText)
{
	if (allowedTextType == TextType::Any)
	{
		return newText.substr(0, maxLength);
	}
	else
	{
		// Remove invalid chars
		std::stringstream cleansedStr;
		// Issue: https://github.com/j4s0n-c/trowaSoft-VCV/issues/5. Changed from string constant (emtpy string "") to string object empty string ("") to older Linux compilers. Thx to @Chaircrusher.
		std::regex_replace(std::ostream_iterator<char>(cleansedStr), newText.begin(), newText.end(), regexInvalidChar, std::string(""));
		return cleansedStr.str().substr(0, maxLength);
	}
} // end cleanseString()

// Remove invalid chars
/** Inserts text at the cursor, replacing the selection if necessary */
void TSTextField::insertText(std::string newText) {
	if (cursor != selection) {
		int begin = std::min(cursor, selection);
		this->text.erase(begin, std::abs(selection - cursor));
		cursor = selection = begin;
	}
	std::string cleansedStr = cleanseString(newText);
	this->text.insert(cursor, cleansedStr);
	cursor += cleansedStr.size();
	selection = cursor;
	onTextChange();
	return;
} // end insertText()

// On Key
//void TSTextField::onText(const event::Text &e) 
void TSTextField::onSelectText(const event::SelectText &e)
{
	if (enabled)
	{
		if (e.codepoint < 128) {
			std::string newText(1, (char)e.codepoint);
			//insertText(newText);
			if ((allowedTextType == TextType::Any || regex_match(newText, regexChar)) && text.length() < maxLength)
			{
				insertText(newText);
			}
		}
	}
	e.consume(this);
	return;
} // end onText()

void TSTextField::setText(std::string text) {
	this->text = cleanseString(text);
	selection = cursor = text.size();
	onTextChange();
}
// When the text changes.
void TSTextField::onTextChange() {
	text = cleanseString(text);
	cursor = clamp(cursor, 0, text.size());
	selection = clamp(selection, 0, text.size());
	//DEBUG("onTextChange() - New cursor: %d", cursor);
	return;
} // end onTextChanged()

// On key press.
//void TSTextField::onKey(const event::Key &e) 
void TSTextField::onSelectKey(const event::SelectKey &e)
{
	if (!visible)
	{
		// Do not capture the keys.
		//e.consumed = false;
		return;
	}
	if (!enabled)
	{
		//e.consumed = false; // We are ingoring this.
		return;
	}
	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		// We can throw invalid chars away in onText(), so we don't have to check here anymore.
		//// Flag if we need to validate/cleanse this character (only if printable and if we are doing validation).
		//bool checkKey = (this->allowedTextType != TextType::Any) && isPrintableKey(e.key);
		switch (e.key) {
			case GLFW_KEY_TAB:
				// If we have an event to fire, then do it
				if ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) //    windowIsShiftPressed())//(guiIsShiftPressed())
				{
					if (onShiftTabCallback != NULL)
					{			
						onShiftTabCallback(id);
					}
					else if (prevField != NULL)
					{
						TSTextField* fField = prevField;
						if (!fField->visible)
						{
							switch (tabNextHiddenAction)
							{
							case TabFieldHiddenAction::MoveToNextVisibleTabField:
								fField = fField->prevField;
								while (fField != NULL && !fField->visible && fField != this && !fField->canTabToThisEnabled)
									fField = fField->prevField;
								if (fField == this || (fField != NULL && !fField->visible))
									fField = NULL;
								break;
							case TabFieldHiddenAction::ShowHiddenTabToField:
								while (fField != NULL && fField != this && !fField->canTabToThisEnabled)
									fField = fField->prevField;
								if (fField == this || (fField != NULL && !fField->canTabToThisEnabled))
									fField = NULL;
								if (fField != NULL)
									fField->visible = true;
								break;
							case TabFieldHiddenAction::DoNothing:
							default:
								fField = NULL;
								break;
							}
						}
						if (fField != NULL)
						{
							fField->requestFocus();
						}
					} // end if previous field
				}
				else if (onTabCallback != NULL)
				{
					onTabCallback(id);
				}
				else if (nextField != NULL)
				{
					TSTextField* fField = nextField;
					if (!fField->visible)
					{
						switch (tabNextHiddenAction)
						{
						case TabFieldHiddenAction::MoveToNextVisibleTabField:
							fField = fField->nextField;
							while (fField != NULL && !fField->visible && fField != this && !fField->canTabToThisEnabled)
								fField = fField->nextField;
							if (fField == this || (fField != NULL && !fField->visible))
								fField = NULL;
							break;
						case TabFieldHiddenAction::ShowHiddenTabToField:
							while (fField != NULL && fField != this && !fField->canTabToThisEnabled)
								fField = fField->nextField;
							if (fField == this || (fField != NULL && !fField->canTabToThisEnabled))
								fField = NULL;
							if (fField != NULL)
								fField->visible = true;
							break;
						case TabFieldHiddenAction::DoNothing:
						default:
							fField = NULL;
							break;
						}
					}
					if (fField != NULL)
					{
						fField->requestFocus();
					}
				} // end if next field
				break;
			case GLFW_KEY_KP_ENTER:
			{
				if (multiline){
					insertText("\n");
				}
				else {
					// Key pad enter should also trigger event action
					event::Action evt;
					onAction(evt);
				}
			}
				break;
			default:
				// Call base method
				TextField::onSelectKey(e);
				break;
		}
		cursor = clamp(cursor, 0, text.size());
		selection = clamp(selection, 0, text.size());		
	}
	//e.consumed = true;
	e.consume(this);
	return;
} // end onKey()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// isPrintableKey()
// @keyCode : (IN) The key that is pressed.
// @returns: True if the key represents a printable character, false if not.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
bool isPrintableKey(int key)
{
	bool isPrintable = false;
	switch (key)
	{
	case GLFW_KEY_SPACE:
	case GLFW_KEY_APOSTROPHE:
	case GLFW_KEY_COMMA:
	case GLFW_KEY_MINUS:
	case GLFW_KEY_PERIOD:
	case GLFW_KEY_SLASH:
	case GLFW_KEY_0:
	case GLFW_KEY_1:
	case GLFW_KEY_2:
	case GLFW_KEY_3:
	case GLFW_KEY_4:
	case GLFW_KEY_5:
	case GLFW_KEY_6:
	case GLFW_KEY_7:
	case GLFW_KEY_8:
	case GLFW_KEY_9:
	case GLFW_KEY_SEMICOLON:
	case GLFW_KEY_EQUAL:
	case GLFW_KEY_A:
	case GLFW_KEY_B:
	case GLFW_KEY_C:
	case GLFW_KEY_D:
	case GLFW_KEY_E:
	case GLFW_KEY_F:
	case GLFW_KEY_G:
	case GLFW_KEY_H:
	case GLFW_KEY_I:
	case GLFW_KEY_J:
	case GLFW_KEY_K:
	case GLFW_KEY_L:
	case GLFW_KEY_M:
	case GLFW_KEY_N:
	case GLFW_KEY_O:
	case GLFW_KEY_P:
	case GLFW_KEY_Q:
	case GLFW_KEY_R:
	case GLFW_KEY_S:
	case GLFW_KEY_T:
	case GLFW_KEY_U:
	case GLFW_KEY_V:
	case GLFW_KEY_W:
	case GLFW_KEY_X:
	case GLFW_KEY_Y:
	case GLFW_KEY_Z:
	case GLFW_KEY_LEFT_BRACKET:
	case GLFW_KEY_BACKSLASH:
	case GLFW_KEY_RIGHT_BRACKET:
	case GLFW_KEY_GRAVE_ACCENT:
	case GLFW_KEY_WORLD_1:
	case GLFW_KEY_WORLD_2:
		isPrintable = true;
		break;
	}
	return isPrintable;
} // end isPrintableKey()
