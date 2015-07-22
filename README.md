i99 Map Transform
================

Introduction
-----------

This is a program for generating a transformation pipeline and using it
to transform the entire contents of a map file. Operations include
translating the map and making the map objects relative to it's center
instead of the world's center.

Usage
-----
i99-map-transform [input files] [transforms] [output files]

input files - A list of files you wish to transform.

transforms - A series of transforms each map object will go through. 
	     The order is important, depending on the transforms.
	     
output files - A list of files to output the transformed object into. The number of
	       output files must match the number of input files.
	       
Compilation
----------
To compile this program you will need the following tools:
	* A good C++0x/11 compliant compiler (e.g. g++ 4.6.3)
	* Premake4 (4.4-beta4 or higher). Get it from here: http://industriousone.com/premake/download
	
Compiling:
	1. Run premake4 inside the root of the directory (for example "premake4 gmake" ).
	2. The project files will be generated inside the build directory. Navigate and build as normal.
	3. The program will be built to the bin directory.
		       
Installation
-----------
Linux:
	Place the executable into a directory where you can execute it from the terminal. The directories 
	/usr/bin and /usr/local/bin are standard places for executables to be stored.	
	 You may need to copy any shared libraries inside the projects lib folder to a directory where they
	can be loaded.
	
Windows:
	Place the executable inside any directory stored within the PATH variable. Or add the directory where you
	wish to store the program to the end of the PATH variable. 

Bugs and features
---------------
Report bugs and suggest features at: https://github.com/techsage0x/i99-map-transform/issues

