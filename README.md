# MegaDrumMidiEditor
Complementary app made to extend the functionality of the MegaDrum drumming module with an easy to use UI that lets you apply custom logic in order to remap, gate and <br />
edit MIDI notes in various ways. With this app, you can create plethora of complex effects such as rim shots, flams, cymbal chokes and so on. The user can monitor midi notes coming in from the MegaDrum module, group them via their family and name tags and apply filtering rules, <br />
which can retrigger other notes and rules, allowing for complex behaviour. All the notes outputted by the app are sent over a virtual midi device which can be <br />
picked up inside of DAWs or other apps for further processing <br />
# Overview:
Going to "Add->New Midi Note" will crete a new midi note each comes with a monitor view (left side) and it's rules (right side).<br />
In order to setup the note, click on the cogwheel icon in the monitor view of the note and enter it's MegaDrum family and name number.<br />
<br />
![megadrum_midi_vid_01](https://github.com/StRaToX123/MegaDrumMidiEditor/assets/26925590/8201633a-ddc9-44c2-a8d4-87fa20263288)
<br />
<br />
# Creating custom logic:
Any number of rules can be added, by clicking on the plus sign in the monitor view.<br />
Available rules:<br />
    - WaitForNoteRule (the note being processed will fire of if another note is detected within a time frame)<br />
    - WaitForTimeRule (delay the note being processed by a certain amount of time)<br />
    - RemapRule (linear remap for the note's velocity)<br />
    - ActivateTabRule (the app supportes multiple collections of notes and rules called tabs. This rule will active another tab when fired of)<br />
    - CreateMessageRule (create a )<br />
    - VelocityRule (velocity gating logic, with a lower and an upper threshold)<br />
    - DropAllRule (deny this note from firing of)<br />
    - DropNoteRule (deny another note from firing of in a certain time frame, via it's family and note number)<br />
<br />
![megadrum_midi_vid_02](https://github.com/StRaToX123/MegaDrumMidiEditor/assets/26925590/3259eb4c-3d78-42f5-9005-63c7ef0538f6)

