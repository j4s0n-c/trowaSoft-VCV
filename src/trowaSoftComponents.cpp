#include "trowaSoftComponents.hpp"
#include "TSSequencerModuleBase.hpp"

// Would be nice if Rack allowed ParamWidget::createContextMenu to be overrideable.

// ParamValueItem is not exposed in Rack v2, but we need it.
struct ParamValueItem : ui::MenuItem {
	ParamWidget* paramWidget;
	float value;

	void onAction(const ActionEvent& e) override {
		engine::ParamQuantity* pq = paramWidget->getParamQuantity();
		if (pq) {
			float oldValue = pq->getValue();
			pq->setValue(value);
			float newValue = pq->getValue();

			if (oldValue != newValue) {
				// Push ParamChange history action
				history::ParamChange* h = new history::ParamChange;
				h->name = "set parameter";
				h->moduleId = paramWidget->module->id;
				h->paramId = paramWidget->paramId;
				h->oldValue = oldValue;
				h->newValue = newValue;
				APP->history->push(h);
			}
		}
	}
};

// Append switch quantity stuff if it's in switch mode
void TS_Knob::appendContextMenu(ui::Menu* menu) {
	ParamQuantity* pQty = getParamQuantity();
	TS_SwitchQuantity* sQty = dynamic_cast<TS_SwitchQuantity*>(pQty);
	if (sQty && sQty->snapEnabled && sQty->labels.size() > 0) {
		float minValue = pQty->getMinValue();
		int index = (int)std::floor(pQty->getValue() - minValue);
		int numStates = sQty->labels.size();
		if (numStates > 0) {
			menu->addChild(new ui::MenuSeparator);
			menu->addChild(createMenuLabel(sQty->name));
		}
		for (int i = 0; i < numStates; i++) {
			std::string label = sQty->labels[i];
			// ParamValueItem is not exposed in Rack v2, 
			ParamValueItem* paramValueItem = createMenuItem<ParamValueItem>(label, CHECKMARK(i == index));
			paramValueItem->paramWidget = this;
			paramValueItem->value = minValue + i;
			menu->addChild(paramValueItem);
		}
	}

	return;
}


// If sequencer step, handle CTRL-C, CTRL-V
void TS_PadSwitch::onHoverKey(const HoverKeyEvent& e) {
	if (this->module == NULL)
		return;
	if (isSequencerStep)
	{
		TSSequencerModuleBase* seqModule = dynamic_cast<TSSequencerModuleBase*>(this->module);
		if (seqModule != NULL)
		{
			controlSeqHandleStepKeyboardInput(e, this, seqModule);
		}
	}
	if (!e.isConsumed())
	{
		// Base method
		this->Switch::onHoverKey(e);
	}
	return;
}
// If sequencer step, handle CTRL-C, CTRL-V
void TS_PadSvgSwitch::onHoverKey(const HoverKeyEvent& e) {
	if (this->module == NULL)
		return;
	if (isSequencerStep) 
	{
		TSSequencerModuleBase* seqModule = dynamic_cast<TSSequencerModuleBase*>(this->module);
		if (seqModule != NULL)
		{
			controlSeqHandleStepKeyboardInput(e, this, seqModule);
		}
	}
	if (!e.isConsumed())
	{
		// Base method
		this->SvgSwitch::onHoverKey(e);
	}
	return;
}


void TS_PadSwitch::appendContextMenu(ui::Menu* menu) {
	if (this->module == NULL)
		return;
	if (isSequencerStep)
	{
		TSSequencerModuleBase* seqModule = dynamic_cast<TSSequencerModuleBase*>(this->module);
		if (seqModule != NULL)
		{
			// For some reason this doesn't have intiialize where the other button does.
			controlAppendContextMenuCopyRowCol(menu, this, seqModule, false);
		}
	}
	return;
}

void TS_PadSvgSwitch::appendContextMenu(ui::Menu* menu) {
	if (this->module == NULL)
		return;
	if (isSequencerStep)
	{
		TSSequencerModuleBase* seqModule = (TSSequencerModuleBase*)(dynamic_cast<TSSequencerModuleBase*>(this->module));
		if (seqModule != NULL)
		{
			//controlAppendContextMenuSelect(menu, this, seqModule);
			controlAppendContextMenuCopyRowCol(menu, this, seqModule);
		}
	}
	return;
}