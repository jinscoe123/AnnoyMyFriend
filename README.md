# README


## Description

This is a simple program that overrides speaker volume on Mac OS X and executes a bash script for
a specified amount of time at a random minute each hour. The program is designed to be difficult
to stop \(i.e. by ignoring most signals which can be ignored, and by creating a separate process
for the sole purpose of restarting the main process if it is killed\).

**NOTE:** The main process also monitors the second process and restarts
it if it is killed or stopped.

## Configuration

The following \#define's can be modified as desired to achieve different program behavior.
Just open the 'annoy.cpp' file in a text editor, navigate to the following constant definitions,
and adjust their values to whatever.

* *ANNOY_COMMAND* - bash command to execute continuously for time duration	
\(e.g. `"say 'Blah blah blah blah blah'"`\)

* *DURATION* - duration of script execution \(in seconds\)	
\(e.g. 300 => execute script for 5 minutes\)

* *VOLUME* - volume to which speakers should be overridden \(0-7\)	
\(e.g. 0 => mute, 7 => max volume\)


## Compilation/Execution

`$ make`

`$ ./annoy &`


## Program Termination

`$ ./fastkill.sh`

**NOTE:** If the executable has been given a name other than its default, then
this name will have to be passed to the 'fastkill.sh' script.

i.e.

`$ ./fastkill.sh [program-name]`
