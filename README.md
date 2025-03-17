# trowaSoft-VCV
![trowaSoft Modules for VCV Rack](https://github.com/j4s0n-c/trowaSoft-VCV/blob/v2/screenshots/all_modules.png "trowaSoft Modules for VCV Rack")

trowaSoft Modules plugin for [VCV Rack](https://github.com/VCVRack/Rack) v0.5.x, v0.6.x, v1.x, v2.x. 
The current pack includes [trigSeq & trigSeq64](#trigseq--trigseq64), [voltSeq](#voltseq), [polyGen](#polygen), [multiSeq](#multiseq), [multiWave & muiltiWaveMini](#multiwave--multiwavemini), [multiScope](#multiscope), [cvOSCcv](#cvosccv), and [cvOSC & OSCcv](#cvosccv-expansion-modules).

For more information about Rack, please visit:
[https://vcvrack.com/](https://vcvrack.com/).

If you like the modules and wish to donate, you may do so [here](https://paypal.me/j4s0n). Any donation is much appreciated!

## Binaries/Builds
Any builds that are currently available are at [Github Releases page](https://github.com/j4s0n-c/trowaSoft-VCV/releases).
Recent builds for [trowaSoft modules](https://library.vcvrack.com/?query=&brand=trowaSoft) should also be available in the [VCV plugin manager](https://library.vcvrack.com/trowaSoft).

**VCV Rack v2.x.x**:   
**2025-03-16**: The latest version is [v2.0.9](https://github.com/j4s0n-c/trowaSoft-VCV/releases/tag/v2.0.9).
([Change Log](https://github.com/j4s0n-c/trowaSoft-VCV/blob/v2/CHANGELOG.md)).

No more versions for older Rack versions will be developed, but they are still available here:
+ **VCV Rack v1.x.x**:   
2020-09-13: The latest version is [v1.0.3](https://github.com/j4s0n-c/trowaSoft-VCV/releases/tag/v1.0.3).
+ **VCV Rack v0.6.x**:   
2018-09-20: The latest version is [v0.6.4a](https://github.com/j4s0n-c/trowaSoft-VCV/releases/tag/v0.6.4a).
+ **VCV Rack v0.5.x**:   
2018-02-17: The last version is [v0.5.5.2](https://github.com/j4s0n-c/trowaSoft-VCV/releases/tag/v0.5.5.2). 

To build for your platform, please visit the [VCV rack documentation](https://vcvrack.com/manual/Building#Building-Rack-plugins).

## Modules

+ Sequencers
    + **[trigSeq & trigSeq64](#trigseq--trigseq64)** - Simple On/Off step sequencers.
	+ **[voltSeq](#voltseq)** - Variable voltage step sequencer.
	+ **[multiSeq](#multiseq)** - trigSeq and voltSeq smooshed into one module.
+ Oscillators & Drawing Tools
    + **[polyGen](#polygen)** - Oscillator that generates x- and y-coordinates of simple polygons for drawing on a scope.
    + **[multiWave & muiltiWaveMini](#multiwave--multiwavemini)** - Module with three (3) oscillator clocks.
    + **[multiScope](#multiscope)** - Scope that allows three (3) waveforms to be drawn on the same canvas.
+ Open Sound Control CV Interface
	+ **[cvOSCcv](#cvosccv)** - Simple module for sending CV to OSC and receiving OSC to CVs.
	+ **[cvOSC/16/32 & OSCcv/16/32](#cvosccv-expansion-modules)** - Expansion modules for cvOSCcv.

## Sequencers
Currently there are four (4) sequencer modules.

### trigSeq & trigSeq64
![trigSeq and trigSeq64](https://github.com/j4s0n-c/trowaSoft-VCV/blob/v2/screenshots/trigSeq_trigSeq64.png?raw=true "trigSeq and trigSeq64 step sequencers")
  
These are basic boolean on/off pad step sequencers (0V or 10V), based off the [Fundamentals SEQ3 sequencer](https://github.com/VCVRack/Fundamental).

+ **trigSeq** is 16-step; **trigSeq64** is 64-step.
+ 64 patterns.
+ 16 channels (outputs).
+ Output modes: **TRIG** (trigger), **RTRG** (retrigger), **GATE** (continuous) (0 or 10V).   
  (as of v1.0.2) Each channel may have its own separate output mode.
+ Inputs: Pattern, BPM, (step) Length, Clock, Reset.
+ Copy & Paste of channel or entire pattern.
+ Copy & Paste of row/column/single step (as of v2.0.7). Use the context menu on a step to access copy/paste. 
  - **CTRL-C** / **CMD-C** will copy the single hovered step (to copy a row or column you must use the context menu).
  - **CTRL-V** / **CMD-V** will paste the row/column/single step from the clipboard.
+ Open Sound Control (OSC) interface (as of v.0.5.5.1). [(more info)](https://github.com/j4s0n-c/trowaSoft-VCV/wiki/Open-Sound-Control-(OSC)-Interface)
+ Advanced Randomization options (as of v.0.5.5.2) for all patterns, current edit pattern, or only the displayed channel. Chose from 'normal random' or 'structured' random patterns.
+ Hold mouse down and set multiple pads by dragging.

### voltSeq
![voltSeq variable voltage sequencer](https://github.com/j4s0n-c/trowaSoft-VCV/blob/v2/screenshots/voltSeq.png?raw=true "voltSeq variable voltage sequencer")

**voltSeq** is a variable voltage output step sequencer (-10V to +10V), based off the [Fundamentals SEQ3 sequencer](https://github.com/VCVRack/Fundamental).

+ **voltSeq** is 16-step.
+ 64 patterns.
+ 16 channels (outputs).
+ Output modes: (as of v1.0.2) Each channel may have its own separate output mode.
    + **VOLT** - Voltage (-10V to +10V): Output whatever voltage you want.
    + **NOTE** - Midi Note (-5V to +5V): Output notes (12 notes per 1 V; 10 octaves).
    + **PATT** - Pattern (-10V to +10V): To control the currently playing Pattern (or Length) on another **trigSeq**, **voltSeq**, or **multiSeq**. (1 to 64 in range).  
+ Inputs: Pattern, BPM, (step) Length, Clock, Reset.
+ Copy & Paste of channel or entire pattern.
+ Copy & Paste of row/column/single step (as of v2.0.7). Use the context menu on a step to access copy/paste. 
  - **CTRL-C** / **CMD-C** will copy the single hovered step (to copy a row or column you must use the context menu).
  - **CTRL-V** / **CMD-V** will paste the row/column/single step from the clipboard.
+ Open Sound Control (OSC) interface (as of v.0.5.5.1). [(more info)](https://github.com/j4s0n-c/trowaSoft-VCV/wiki/Open-Sound-Control-(OSC)-Interface)
+ Advanced Randomization options (as of v.0.5.5.2) for all patterns, current edit pattern, or only the displayed channel. Chose from 'normal random' or 'structured' random patterns.
+ Shift Values (as of v0.5.5.2): +/- 1 Volt or 1 Octave or 1 Pattern for all patterns, current edit pattern, or only the displayed channel.

### multiSeq
![multiSeq](https://github.com/j4s0n-c/trowaSoft-VCV/blob/v2/screenshots/multiSeq.png?raw=true "multiSeq trigger and variable voltage sequencer")

**multiSeq** is a combination of trigSeq and voltSeq. It supports triggers or variable voltages. This module is new in v2.0.4.

+ **multiSeq** is 64-step.
+ 64 patterns.
+ 16 channels (outputs).
+ Output modes: Each channel may have its own separate output mode.
    + **TRIG** (trigger) (0 or 10V)
    + **RTRG** (retrigger) (0 or 10V)
    + **GATE** (continuous) (0 or 10V)
    + **VOLT** - Voltage (-10V to +10V): Output whatever voltage you want.
    + **NOTE** - Midi Note (-5V to +5V).
    + **PATT** - Pattern (-10V to +10V): To control the currently playing Pattern (or Length) on another **trigSeq**, **voltSeq**, or **multiSeq**. 
+ **"Song Mode" Internal Pattern Sequencer**: Setup automatic pattern changes so you don't need another sequencer to send CV into the PATT input. Up to 64 sequences.
+ Inputs: Pattern, BPM, (step) Length, Clock, Reset.
+ Copy & Paste of channel or entire pattern.
+ Copy & Paste of row/column/single step (as of v2.0.7). Use the context menu on a step to access copy/paste. 
  - **CTRL-C** / **CMD-C** will copy the single hovered step (to copy a row or column you must use the context menu).
  - **CTRL-V** / **CMD-V** will paste the row/column/single step from the clipboard.
+ Open Sound Control (OSC) interface (as of v.0.5.5.1). [(more info)](https://github.com/j4s0n-c/trowaSoft-VCV/wiki/Open-Sound-Control-(OSC)-Interface)
+ Advanced Randomization options (as of v.0.5.5.2) for all patterns, current edit pattern, or only the displayed channel. Chose from 'normal random' or 'structured' random patterns.
+ Shift Values (as of v0.5.5.2): +/- 1 Volt or 1 Octave or 1 Pattern for all patterns, current edit pattern, or only the displayed channel.

## Oscillators & Drawing

### polyGen
![polyGen](https://github.com/j4s0n-c/trowaSoft-VCV/blob/v2/screenshots/polyGen.png?raw=true "polyGen Shape Oscillator")
  
**polyGen** is a simple polygon generator/oscillator. It generates output CVs for x and y coordinates of simple shapes. This module is new in v2.0.7.

+ **CV Inputs & User Controls** - Most CV inputs have a corresponding user control knob. 
  **[+]** means the CV and Knob add together. **[-OR-]** means if the CV is present, the knob is ignored.
    + **Frequency** - **[+]** (-5V to +5V) Number of shapes to draw per second (Hz).
    + *(Main) Shape*
        + **# Vertices** - **[-OR-]** (0V to 10V) Number of outer vertices (N = 3 to 33; 1 V per 3 vertices). 'Inner' vertices will be mapped in between, but by default will be in-line with the outer vertices.
        + **Angle Offset** - **[+]** (0V to 10V) Angle offset for shape / initial rotation. This applied before any adjustments in "*Radius / Amplitude*" or "*Rotation*" are applied.
    + *Star* - Adds a second set of inner vertices in between the main shape's vertices (i.e so you could draw a star).
        + **Inner Vertices Radius** - **[+]** (0V to 10V) Radius of inner vertices relative to the outer radius (main shape radius). Default is 1 (i.e. 100%; no star).
        + **Inner Vertices Angle** - **[+]** (0V to 10V) Angle offset of the inner vertices. Default is 0º from 180º/N (mid-angle of main shape/outer vertices).
    + *Radius / Amplitude* - Size applied to the main shape before "*Rotation*".
        + **R<sub>x</sub>** - **[-OR-]** (-10V to +10V) Radius in x direction (x-amplitude) (before rotation). Default is 5 V.
        + **R<sub>y</sub>** - **[-OR-]** (-10V to +10V) Radius in y direction (y-amplitude) (before rotation). Default is 5 V.
    + *Rotation* - Rotates the shape after "*Shape*", "*Star*" and "*Radius / Amplitude*" settings have been applied.
        + **Rotation** - **[+]** (-10V to +10V) Rotation. 1 V per full rotation (360º). If **SPIN** is on, this is the rate of rotation (º/s), else this is the absolute rotation (º).
        + **X<sub>c</sub>** - **[-OR-]** (-10V to +10V) Center of rotation (x). Default is 0 V.
        + **Y<sub>c</sub>** - **[-OR-]** (-10V to +10V) Center of rotation (y). Default is 0 V.
        + **SPIN** - **[Control Only]** Turn on if the rotation is relative (rotation is speed). Turn off if absolute rotation.
    + *Offset* - Final offset applied after all other transformations.
        + **X-Offset** - **[-OR-]** (-10V to +10V) Offset in the x direction. Default is 0 V.
        + **Y-Offset** - **[-OR-]** (-10V to +10V) Offset in the y direction. Default is 0 V.
    + **Sync** - **[CV only]** (0-10V) Trigger to reset the shape/oscillator to the start.

+ **Outputs**:
    + **X** - (-10V to +10V) x coordinate of the shape.
    + **Y** - (-10V to +10V) y coordinate of the shape.
    + **SYNC** - (0-10V) Triggers at start of new shape.

+ **Preview Screen**:
    + The previews show the ideal shapes (i.e. just the vertices with lines connecting them). In general, frequencies that are around / multiples of your frame rate produce the smoothest lines when charting.
    + The small red preview in the corner of the screen shows the shape *before* it is moved by "*Rotation*" and/or "*Offset*" parameters.
    + The white preview in the center of the screen shows the shape *after* it is moved by "*Rotation*" and/or "*Offset*" parameters.

### multiWave & multiWaveMini
![multiWave](https://github.com/j4s0n-c/trowaSoft-VCV/blob/v2/screenshots/multiWave.png?raw=true "multiWave digitial oscillator") ![multiWaveMini](https://github.com/j4s0n-c/trowaSoft-VCV/blob/v2/screenshots/multiWave_mini.png?raw=true "multiWaveMini oscillator")

**multiWave** is a digital oscillator module with three (3) oscillators/clocks, each with two (2) configurable wave channel outputs. 
This module has been made to complement [multiScope](#multiscope) and is new in v0.6.3.

**multiWaveMini** has been introduced in v2.0.7 and is the same as **multiWave**, but the display has been removed. 
Starting in Rack v1, the ability to see and set parameter (i.e knob) values directly was introduced, so being able to see the exact values and edit them in the screen isn't as important.

+ **Screen User Controls**: (for full-size **multiWave** only)
    + Click on a value to edit it directly (a text box should appear and allow you to type the value).
    + **Tab** or **Tab-Shift** will iterate through the editable text boxes. 
    + Valid for all displayed values except for WAVE and AUX (AUX is only an editable textbox for pulse width when SQR/rectangle wave is selected).

+ **CV Inputs & User Controls per Oscillator**:
    + **AMPL** - Amplitude (-10V to +10V).
    + **FREQ** - Frequency (1V/Oct) for the oscillator clock.  
    The Frequency knob rotates 360&deg;. Hold down the **Shift** key for coarser control or the **Control** key for finer control while dragging up/down.
    + **PHASE** - Phase Shift (-10V to +10V).
    + **OFFSET** - Offset (-10V to +10V).
    + **SYNC (Up)** Button - (new in v2.0.7) Directly synchronize with the oscillator above.   
     NOTE: This is only available on Oscillator 2 and Oscillator 3.
    + **SYNC** CV Input - (Right hand side) Reset/sync the oscillator (to phase 0). Currently this is CV only (no UI control).
    + CV Inputs & User Controls per Channel Output:
        + **WAV** - Waveform Type (-5V to +5V): SIN, TRI, SAW, SQR.
        + **AUX** - Aux (-5V to +5V). If the CV input is active then, the knob value is ignored.  
        Currently only SAW and SQR have functions: 
		    + SAW: Slope (pos |/| or neg |\\|). 0 or positive CV for positive slope.
			+ SQR: Pulse Width.
			+ SIN and TRI adjustments will be added later when/if we think of another parameter for these waveforms.  
        + **PHASE** - Phase Shift (-10V to +10V). Value is relative to the oscillator clock.
        + **MOD** - Amplitude modulation (-10V to +10V). Knob controls the mix between the raw signal and the modded signal.
        + **\*** - Button for modulation type (Digital or Ring). Currently this is UI only (no CV input).

+ **CV Outputs per Oscillator**:
    + **SYNC** - Triggers whenever the period restarts.
    + CV Outputs per Channel Output:
        + **X&lt;n&gt; or Y&lt;n&gt;** - RAW waveform without amplitude modulation (**MOD**).
        + **MOD** - The modulated waveform (based on the MOD knob and the incoming MOD signal input).

### multiScope
![multiScope](https://github.com/j4s0n-c/trowaSoft-VCV/blob/v2/screenshots/multiScope.png?raw=true "multiScope")

**multiScope** is a visual effects scope, with lissajous mode, that allows three (3) waveforms to be drawn on the same screen/canvas. (code based on [JW Modules FullScope](https://github.com/jeremywen) and [Fundamental Scope](https://github.com/VCVRack/Fundamental))

+ **CV Inputs per Channel**:
    + **X** - X-value (horizontal component).
    + **Y** - Y-value (vertical component).
    + **C** - Color/hue (0V to +5V).
    + **A** - Alpha channel (0V to +5V).
    + **BLANK** - Blank ON or OFF. By default, Blank is off. ON is any input <=0 (really < 0.1V), otherwise it will be OFF.  
    You can use a **trigSeq** (in **GATE** mode, synchronized with a **voltSeq**) to control / hide lines that you do not wish to be shown.
	+ **FC** - Fill Color hue (0V to +5V)
    + **FA** - Fill alpha channel (0V to +5V).
    + **R** - Rotation (-10V to +10V). Will either be a rotational rate or if the **ABS** button is on, it will be the absolute angular position.
    + **T** - Time.
    + **TH** - Line Thickness.
  
+ **User Controls per Channel**:
    + **X** - Offset (OFF) & Scale (SCL) knobs.
    + **Y** - Offset (OFF) & Scale (SCL) knobs.
    + **LNK** - (Toggle) Link the X-scale and Y-scale knobs together so they will change together (have the same value).
    + **C** - Color knob. If an input is active on the Color port, this is ignored. Highest setting will yield White now.
    + **A** - Alpha channel knob. If an input is active on the Alpha port, this is ignored.
    + **FC** - Fill Color knob. If an input is active on the Fill Color port, this is ignored. Highest setting yield give White now.
    + **Fill Color** - (Toggle) Fill on/off.
    + **FA** - Fill alpha channel knob. If an input is active on the Alpha port, this is ignored.
    + _Rotation Controls_:
        + **R** - Rotation knob. If an input is active on the Rotation port, this is ignored.
        + **ABS** - (Toggle) Turning ABS on will make the rotation inputs control the absolute angular position instead of a rate.		
    + **T** - Time adjustment knob. Will be used along with the Time input port.
    + **TH** - Line Thickness. If an input is active on the Thickness port, this is ignored.
	+ **EFFECT** - Effect knob.
    + **X*Y** - (Toggle) Toggle lissajous mode on / off (default is on).

+ **User Controls for entire module**:
    + **INFO** - (Toggle) Toggle input parameter information on / off (default is on). Located on the right-hand-side (RHS) bar.
	+ **BG COLOR** - (Toggle) Toggle on-screen Background Color picker on / off (default is on). Located on the right-hand-side (RHS) bar.
	+ **Background Color Picker** - Displayed on screen. Hue-Saturation-Light (HSL) sliders to pick the background color.

## Open Sound Control

### cvOSCcv
![cvOSCcv default screen and configuration screen.](https://github.com/j4s0n-c/trowaSoft-VCV/blob/v2/screenshots/cvOSCcv.png?raw=true "cvOSCcv default screen and configuration screen")
  
**cvOSCcv** is a simple, generic [Open Sound Control](http://opensoundcontrol.org/) (OSC) module for outputting Rack CVs to OSC and reading in simple OSC messages into Rack CVs. This module is new in version 0.6.0.

+ **CV Inputs** - CV => OSC (8 channels), each channel:
    + **TRG** - (mono) If active, then OSC messages will output the **VAL** CV input when triggered.
    + **VAL** - (poly as of v1.0.2) The value(s) that will output over OSC.  
    	+ If more than one polyphonic channel is set, it will be sent in OSC as an array, otherwise it will be sent as a scalar.   
    	+ If there is no trigger present, the module will output whenever **VAL** changes at least the **Change Threshold** (default 0.05) up to the **Send Frequency (Hz)** (default is 100 Hz).
+ **CV Outputs** - OSC => CV (8 channels), each channel:
    + **TRG** - (mono) (0-10V) Triggers whenever an OSC message is received.
	+ **VAL** - (poly as of v1.0.2) (Gate) Outputs the last OSC value(s) received.   
      Note that polyphonic cables are limited to 16 channels, so if an array larger than this is received, only the first 16 channels will be available.
+ **User Controls**:
	+ **CONFIG** - (Toggle) Button to toggle the configuration view. When an OSC connection is active, a blue light will appear on the button.
	+ **LEFT** / **RIGHT** - (as of v1.0.2) Buttons to navigate between the configuration of the module itself or any expansion modules ([cvOSC or OSCcv](#cvosccv-expansion-modules)) connected to it.
	+ **OSC IP Address** - The IP address of the OSC client/server.  Default is `127.0.0.1`.
	+ **Out Port** - Port for sending messages. 
	+ **In Port** - Port for receiving messages.   
      (as of v1.0.2) **cvOSCcv** modules may **share the same ports** to either speak to the same endpoint or talk to each other.
	+ **Namespace** - The OSC namespace. Default is empty (none).
	+ **Auto Con** - Automatically reconnect on load from save. The connection will be restored if the connection was active (in the save file) and this is checked.
    + Per Channel:  
        + **Address** - Endpoint address. Default is `/ch/{channel #}`.
		+ **ADV** - (as of v0.6.2) Advanced settings for simple value conversions. Specify simple OSC data types (float, int, bool) and the CV and OSC ranges.
    + **Send Frequency (Hz)** - (as of v2.0.5)(Context Menu) Choose the default send frequency (if no trigger is present) in the context menu. The default is 100 Hz.
    + **Change Threshold** - (as of v2.0.8)(Context Menu) Choose the default send change threshold (if no trigger is present) in the context menu. The default is 0.05.

	NOTE: To save Channel Address changes after a connection is active, simply hide the configuration screen again.

### cvOSCcv Expansion Modules
![Expansion modules for cvOSCcv: cvOSC and OSCcv.](https://github.com/j4s0n-c/trowaSoft-VCV/blob/v2/screenshots/cvOSCcv_expanders.png?raw=true "Expansion modules for cvOSCcv: cvOSC and OSCcv. 8-, 16- and 32-channel versions.")

**cvOSC** and **OSCcv** are expansion modules for **cvOSCcv**. They add extra inputs or outputs respectively. 
These modules are new in version 1.0.2. The 16- and 32-channel versions are new in v2.0.7.

+ **cvOSC** / **cvOSC16** / **cvOSC32**: CV => OSC. 
	+ Adds 8/16/32 more input channels. Each channel has a **TRG** (mono) and a **VAL** (poly) input.
	+ Module will only connect to a master if it is placed to the **LEFT** of the master cvOSCcv module. It must be touching it or another cvOSC.
    + **Change Threshold** - (as of v2.0.8)(Context Menu) Choose the default send change threshold (if no trigger is present) in the context menu. The default is "Match Master" (uses the threshold set in the master cvOSCcv module).
+ **OSCcv** / **OSCcv16** / **OSCcv32**: OSC => CV. 
	+ Adds 8/16/32 more output channels. Each channel has a **TRG** (mono) and a **VAL** (poly) output.
	+ Module will only connect to a master if it is placed to the **RIGHT** of the master cvOSCcv module. It must be touching it or another OSCcv.
+ Multiple expansion modules may be chained.
+ Expansion modules have no user controls. They must be configured in their master cvOSCcv module. Configurable items are:
    + Expansion Module Name (only used for display, not used in OSC)
	+ Channel Addresses
	+ Channel Conversions
	+ **RENUMBER** : (as of v2.0.7) Button to "renumber" the channel addresses. All advanced channel settings are retained, only the addresses are updated.
	+ **PAGE** : (as of v2.0.7) For expansion modules with more than eight (8) channels, change the channel page/column that you are editing.   
      If the expander has more than one (1) column, then the current column will be highlighted on the expander.
+ By default, channels start at #9. After connecting to a master [cvOSCcv](#cvosccv) module, initializing the expansion module will "re-number" its channels (i.e. `/ch/9` will become `/ch/17`).
  Or (as of v2.0.7) you may **RENUMBER** the channel names only in the master's configuration screen.

  NOTE: The expander's name is only for display purposes to help identify which expander you are configuring in the master module.

