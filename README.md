# trowaSoft-VCV
<div>
<img src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2fdemo_screenshot.jpg" />
</div>

trowaSoft Modules plugin for [VCV Rack](https://github.com/VCVRack/Rack) v0.5.0.

For more information about these modules, please visit:
http://www.geekasaurusrex.net/page/trowaSoft-Sequencer-Modules-for-VCV-Rack.aspx.

For more information about Rack, please visit:
https://vcvrack.com/.

## Sequencers
Currently there are three (3) sequencer modules.

### trigSeq & trigSeq64
<div>
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2ftrigSeq_screenshot.jpg" />
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2ftrigSeq64_screenshot.jpg" />
</div>


These are basic boolean on/off pad step sequencers (0V or 10V), based off the [Fundamentals SEQ3 sequencer](https://github.com/VCVRack/Fundamental).
+ **trigSeq** is 16-step; **trigSeq64** is 64-step.
+ Now 64 patterns. ~~16 patterns.~~
+ 16 channels (outputs).
+ Output modes: **TRIG** (trigger), **RTRG** (retrigger), **GATE** (0 or 10V).
+ Inputs: Pattern, BPM, (step) Length, Clock, Reset.
+ Copy & Paste of channel or entire pattern.

### voltSeq
<div>
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2fvoltSeq_screenshot.jpg" />
</div>


Variable voltage output step sequencer (-10V to +10V), based off the [Fundamentals SEQ3 sequencer](https://github.com/VCVRack/Fundamental).
+ **voltSeq** is 16-step.
+ Now 64 patterns. ~~16 patterns.~~
+ 16 channels (outputs).
+ Output modes:
    + **VOLT** - Voltage (-10V to +10V): Output whatever voltage you want.
    + **NOTE** - Midi Note (-5V to +5V): Output notes (12 notes per 1 V; 10 octaves).    
	+ **PATT** - Pattern (-10V to +10V): To control the currently playing Pattern (or Length) on another **trigSeq** or **voltSeq**. (Now 1 to 64 in range).
  
+ Inputs: Pattern, BPM, (step) Length, Clock, Reset.
+ Copy & Paste of channel or entire pattern.

## Other
### multiScope
<div>
<img width="700" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f12%2fmultiScope_screenshot.jpg" />
</div>

**multiScope** is a visual effects scope (lissajous mode) that allows three (3) waveforms to be drawn on the same screen/canvas.
+ Input Ports (per waveform; so x3):
    + **X** - X-value (horizontal component).
    + **Y** - Y-value (vertical component).
    + **C** - Color/hue (0V to +5V).
    + **A** - Alpha channel (0V to +5V).
    + **BLANK** - Blank ON or OFF. By default, Blank is off. ON is any input <=0 (really < 0.1V), otherwise it will be OFF.  
   You can use a **trigSeq** (synchronized with a **voltSeq**) to control / hide lines that you do not wish to be shown.  
    + **R** - Rotation (-10V to +10V). Will either be a rotational rate or if the **ABS** button is on, it will be the absolute angular position.
    + **T** - Time.
  
+ User Controls (per waveform; so x3):
    + **X** - Offset (OFF) & Scale (SCL) knobs.
    + **Y** - Offset (OFF) & Scale (SCL) knobs.
    + **LNK** - (Toggle) Link the X-scale and Y-scale knobs together so they will change together (have the same value).
    + **C** - Color knob. If a input is active on the Color port, this is ignored.
    + **A** - Alpha channel knob. If a input is active on the Alpha port, this is ignored.
    + _Rotation Controls_:
        + **R** - Rotation knob. If a input is active on the Rotation port, this is ignored.
        + **ABS** - (Toggle) Turning ABS on will make the rotation inputs control the absolute angular position instead of a rate.		
    + **T** - Time adjustment knob. Will be used along with the Time input port.
    + **X*Y** - (Toggle) Toggle lissajous mode on / off (default is on).
	+ **INFO** - (Toggle) Toggle wave information on / off (default is on). Located on the right-hand-side (RHS) bar.

