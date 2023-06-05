# switchboard
Render a visual representation of the load on a supercomputer

![sample](frontier8192x15.png?raw=true "Example of 8192-node job on Frontier")

## Build and run
You should be able to build and run the code with:

	make
	./switchboard.bin -n nodelist8192 -o image.png

Generate the nodelist file with a command like

	squeue > nodelist
	squeue -t running > nodelist

## To do
* look for one of a number of keywords: "frontier", "crusher", "file" (to start a new image), etc.
* allow continuity of colors based on job ID
* generalize the box-sizing code to any number of levels in a hierarchy (not just 2)
* read the nodelist from stdin if "-" is given
* generalize the method by which machines are added - a user header file?

## Thanks
[Lodepng](https://github.com/lvandeve/lodepng), [CLI11](https://github.com/CLIUtils/CLI11), [Oak Ridge National Laboratory](https://www.olcf.ornl.gov/)
