#include "Sweet.hpp"
#include "dsp/digital.hpp"

struct MyModule : Module {
	enum ParamIds {
        CLOCK_PARAM,
		CLOCK2_PARAM,
		RUN_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		CLOCK2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

    SchmittTrigger trigger;
	SchmittTrigger trigger2;
    SchmittTrigger runButtonTrigger;

	struct LFOGenerator {
		float phase = 0.0;
		float pw = 0.5;
		float freq = 1.0;
		void setFreq(float freq_to_set)
	  {
	    freq = freq_to_set;
	  }
		void step(float dt) {
			float deltaPhase = fminf(freq * dt, 0.5);
			phase += deltaPhase;
			if (phase >= 1.0)
				phase -= 1.0;
		}
		float sqr() {
			float sqr = phase < pw ? 1.0 : -1.0;
			return sqr;
		}
	};
	LFOGenerator clock;
	LFOGenerator clock2;


	MyModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

bool running = true;
void MyModule::step()
{

	if (params[RUN_SWITCH].value)
	{
		lights[BLINK_LIGHT].value = 0;
		outputs[CLOCK_OUTPUT].value = 0;
		outputs[CLOCK2_OUTPUT].value = 0;
		return;
   	}

    lights[BLINK_LIGHT].value = 1.0f;


	int tempo = std::round(params[CLOCK_PARAM].value);
    float frequency = tempo/60.0;
    clock.setFreq(frequency);
	clock.step(1.0 / engineGetSampleRate());
    bool pulse = trigger.process(clock.sqr());

	clock2.setFreq(frequency * params[CLOCK2_PARAM].value);
	clock2.step(1.0 / engineGetSampleRate());
	bool pulse2 = trigger2.process(clock2.sqr());

	outputs[CLOCK_OUTPUT].value = pulse ? 10.0 : 0.0;
    outputs[CLOCK2_OUTPUT].value = pulse2 ? 10.0 : 0.0;
}


struct MyModuleWidget : ModuleWidget {
	MyModuleWidget(MyModule *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/MyModule.svg")));

		addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(27, 80), module, MyModule::CLOCK_PARAM, 1, 1024, 140));

		addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(27, 200), module, MyModule::CLOCK2_PARAM, 0.0, 2.0, 1.0));

		addOutput(Port::create<PJ301MPort>(Vec(34, 130), Port::OUTPUT, module, MyModule::CLOCK_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(34, 250), Port::OUTPUT, module, MyModule::CLOCK2_OUTPUT));

		addParam(ParamWidget::create<CKSS>(Vec(38,305), module, MyModule::RUN_SWITCH, 0.0f, 1.0f, 1.0f));


		//addParam(ParamWidget::create<LEDBezel>(Vec(33.5, 330), module, MyModule::RUN_SWITCH , 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(40, 340), module, MyModule::BLINK_LIGHT));
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelMyModule = Model::create<MyModule, MyModuleWidget>("Sweet", "Sweet Clock", "Sweet Clock", CLOCK_TAG);
