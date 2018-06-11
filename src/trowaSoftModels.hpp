#ifndef TROWASOFTMODELS_HPP
#define TROWASOFTMODELS_HPP

#include "rack.hpp"
using namespace rack;
#include "tags.hpp"
#include "plugin.hpp"

#include "TSSequencerModuleBase.hpp"
#include "Module_trigSeq.hpp"

/////// For trigSeq ////////
//#define N64_NUM_STEPS	64
//#define N64_NUM_ROWS	 8
//#define N64_NUM_COLS	 (N64_NUM_STEPS/N64_NUM_ROWS)

//struct trigSeq64Model : Model {
//	trigSeq64Model(std::string manufacturer, std::string slug, std::string name, Tags... tags)
//	{
//		this->manufacturer = manufacturer;
//		this->slug = slug;
//		this->name = name;
//		this->tags = { tags... };
//		return;
//	}
//
//	Module *createModule() override {
//		trigSeq *module = new trigSeq(N64_NUM_STEPS, N64_NUM_ROWS, N64_NUM_COLS);
//		return module;
//	}
//	ModuleWidget *createModuleWidget() override {
//		trigSeq *module = new trigSeq(N64_NUM_STEPS, N64_NUM_ROWS, N64_NUM_COLS);
//		trigSeq64Widget *moduleWidget = new trigSeq64Widget(module);
//		moduleWidget->model = this;
//		return moduleWidget;
//	}
//	ModuleWidget *createModuleWidgetNull() override {
//		trigSeq64Widget *moduleWidget = new trigSeq64Widget(NULL);
//		moduleWidget->model = this;
//		return moduleWidget;
//	}
//};

///** Create Model subclass which constructs a specific Module and ModuleWidget subclass */
//template <typename TModule, typename TModuleWidget, typename... Tags>
//static Model *createTSSequencerModel(std::string manufacturer, std::string slug, std::string name,
//	/*in*/ int numSteps, /*in*/ int numRows, /*in*/ int numCols,
//	Tags... tags) {
//	struct TModel : Model {
//		Module *createModule() override {
//			trigSeq *module = new TModule(numSteps, numRows, numCols);
//			return module;
//		}
//		ModuleWidget *createModuleWidget() override {
//			trigSeq *module = new TModule(numSteps, numRows, numCols);
//			TModuleWidget *moduleWidget = new TModuleWidget(module);
//			moduleWidget->model = this;
//			return moduleWidget;
//		}
//		ModuleWidget *createModuleWidgetNull() override {
//			TModuleWidget *moduleWidget = new TModuleWidget(NULL);
//			moduleWidget->model = this;
//			return moduleWidget;
//		}
//	};
//	TModel *o = new TModel();
//	o->manufacturer = manufacturer;
//	o->slug = slug;
//	o->name = name;
//	o->tags = { tags... };
//	return o;
//}



//struct Model {
//	Plugin *plugin = NULL;
//	/** An identifier for the model, e.g. "VCO". Used for saving patches. The slug, manufacturerSlug pair must be unique. */
//	std::string slug;
//	/** Human readable name for your model, e.g. "Voltage Controlled Oscillator" */
//	std::string name;
//	/** The name of the manufacturer group of the module.
//	This might be different than the plugin slug. For example, if you create multiple plugins but want them to be branded similarly, you may use the same manufacturer name in multiple plugins.
//	You may even have multiple manufacturers in one plugin, although this would be unusual.
//	*/
//	std::string manufacturer;
//	/** List of tags representing the function(s) of the module (optional) */
//	std::list<ModelTag> tags;
//
//	virtual ~Model() {}
//	/** Creates a headless Module */
//	virtual Module *createModule() { return NULL; }
//	/** Creates a ModuleWidget with a Module attached */
//	virtual ModuleWidget *createModuleWidget() { return NULL; }
//	/** Creates a ModuleWidget with no Module, useful for previews */
//	virtual ModuleWidget *createModuleWidgetNull() { return NULL; }
//
//	/** Create Model subclass which constructs a specific Module and ModuleWidget subclass */
//	template <typename TModule, typename TModuleWidget, typename... Tags>
//	static Model *create(std::string manufacturer, std::string slug, std::string name, Tags... tags) {
//		struct TModel : Model {
//			Module *createModule() override {
//				TModule *module = new TModule();
//				return module;
//			}
//			ModuleWidget *createModuleWidget() override {
//				TModule *module = new TModule();
//				TModuleWidget *moduleWidget = new TModuleWidget(module);
//				moduleWidget->model = this;
//				return moduleWidget;
//			}
//			ModuleWidget *createModuleWidgetNull() override {
//				TModuleWidget *moduleWidget = new TModuleWidget(NULL);
//				moduleWidget->model = this;
//				return moduleWidget;
//			}
//		};
//		TModel *o = new TModel();
//		o->manufacturer = manufacturer;
//		o->slug = slug;
//		o->name = name;
//		o->tags = { tags... };
//		return o;
//	}
//};


#endif // !TROWASOFTMODELS_HPP


