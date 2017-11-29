# trowaSoft-VCV
trowaSoft Modules for [VCV Rack](https://github.com/VCVRack/Rack) v0.5.0.
For more information visit:
http://www.geekasaurusrex.net/page/trowaSoft-Sequencer-Modules-for-VCV-Rack.aspx.

## Sequencers
Currently there are three (3) sequencer modules.

### trigSeq & trigSeq64
<div>
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f11%2ftrigSeq_Main.png" />
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f11%2ftrigSeq64_Main.png" />
</div>

<p>
These are basic boolean on/off pad step sequencers (0V or 10V), based off the <a href="https://github.com/VCVRack/Fundamental">Fundamentals SEQ3 sequencer</a>.
</p>
<ul>
  <li><strong style="color:blue;">trigSeq</strong> is 16-step; <strong style="color:blue;">trigSeq64</strong> is 64-step.</li>
  <li>16 patterns.</li>
  <li>16 channels (outputs).</li>
  <li>Output modes: <strong>TRIG</strong> (trigger), <strong>RTRI</strong> (retrigger), <strong>GATE</strong> (0 or 10V).</li>
  <li>Inputs: Pattern, BPM, (step) Length, Clock, Reset.</li>
  <li>Copy & Paste of channel or entire pattern.</li>
</ul>

### voltSeq
<div>
<img width="390" src="http://www.geekasaurusrex.net/image.axd?picture=2017%2f11%2fvoltSeq_Main.png" />
</div>

<p>
Variable voltage output step sequencer (-10V to +10V), based off the <a href="https://github.com/VCVRack/Fundamental">Fundamentals SEQ3 sequencer</a>.
</p>
<ul>
  <li><strong style="color:red;">voltSeq</strong> is 16-step.</li>
  <li>16 patterns.</li>
  <li>16 channels (outputs).</li>
  <li>Output modes:
    <ul>
      <li><strong>VOLT</strong> - Voltage (-10V to +10V): Output whatever voltage you want.</li>
      <li><strong>NOTE</strong> - Midi Note (-5V to +5V): Output notes (12 notes per 1 V; 10 octaves).</li>
      <li><strong>PATT</strong> - Pattern (-10 to +10V): To control the currently playing Pattern (or Length) on another <strong style="color:blue;">trigSeq</strong> or <strong style="color:blue">voltSeq</strong>.</li>
    </ul>
  </li>
  <li>Inputs: Pattern, BPM, (step) Length, Clock, Reset.</li>
  <li>Copy & Paste of channel or entire pattern.</li>
</ul>
