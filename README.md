# switchboard
Render a visual representation of the load on a supercomputer

![sample](frontier8192x15.png?raw=true "Example of 8192-node job on Frontier")

## Build and run
You should be able to build and run the code with:

	make
	./switchboard nodelist8192

## To do
* allow continuity of colors based on job ID
* generalize the box-sizing code to any number of levels in a hierarchy
* read the nodelist from stdin if "-" is given
* read command line args to set some of these figures, specify an output filename

## Thanks
Lodepng, OLCF
