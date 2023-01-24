# trowaSoft VCV Rack Modules Change Log

NOTE: The change log is only started v2.0.7. Any previous changes were generally noted in the release notes (refer to the [Github Releases page](https://github.com/j4s0n-c/trowaSoft-VCV/releases)).

## v2.0.7 
- Release [v2.0.7](https://github.com/j4s0n-c/trowaSoft-VCV/releases/tag/v2.0.7)
- **cvOSCcv**
	- Fixed a bug where simple message's (TSOSCCVSimpleMessage) rxVals buffer size was larger than the max number of polyphonic channels (extra vals would never be used anyway right now).
	- Stopped processing OSCcv output expander's OSC messages and allow expander to handle the messages itself.  
	- Added button in config of expanders to renumber the channels from the config screen.
	- Added paging control to the config of expanders for expanders with > 8 channels (multiple columns).
	- Overall, improved performance (measured with the VCV Performance Meters).
	- In ADV settings, changed default channel conversion values to -5 V to + 5 V for the Rack voltage range and 0 to 1 for OSC range.
- **cvOSC** and **OSCcv** (expanders)
	- Added 16-channel and 32-channel versions
	- **OSCcv** will now process its own output messages (OSC -> CV).
	- Removed storing of channel history since expanders don't display history anyway.
- **Sequencers**
	- Added Copy & Paste of a row, column, or a single step.
	- (**voltSeq**, **multiSeq**) Added enumerated selections in the Context Menu for steps for NOTE and PATT modes. You can still type the value in the Context Menu or select the value from an option list.
	- (**voltSeq**, **multiSeq**) Adjusted how the octave was calculated for display in NOTE mode (sometimes it would give the wrong octave).
- **multiWave**
	- Added button to Oscillator 2 and Oscillator 3 to sync directly with the oscillator above it.
	- Added mini, screenless version of multiWave (**mulitWaveMini**).
	- Panel facelift.
- **multiScope**
	- Small panel facelift.
	- Added enumerated selections for EFFECT/Blend Mode knob in Context Menu. 
	- Added double-click of resize handle will return the screen to the default size.
- **polyGen** - Added polyGen to plugin.
- Misc.
	- Adjusted spacing on checkbox text.
	- Changed knobs to Rack's generic RoundBlackKnob.
	- New port graphics.
	- Other misc. changes.
