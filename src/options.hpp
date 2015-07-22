#pragma once
#include <string>
namespace i99
{
	struct i99_option { 
		char const * const name; 
		char const * const desc;
	};

	//General
	static const i99_option opt_inputFiles = { "input-files", "List of files for the program to process. Each file will be processed in the order given." };
	static const i99_option opt_outputFiles = { "output-files", "List of files to be written to. Each output file will be written to in the order given." };
	static const i99_option opt_help       = { "help", "Lists the commands the user can use" };
	

	//Transforms
	static const i99_option opt_translate  = { "translate", "Performs a translation on every object inside a map" };
	static const i99_option opt_makeRelative = { "make-relative", "Finds the center of the map and makes all coordinates relative" };
}
