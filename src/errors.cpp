#include "errors.hpp"

std::string i99::errorNumberToString(int e)
{
	std::string s;
	switch(e)
	{
	case NO_ERROR:
		s = "No error";
		break;
	case INVALID_COMMAND:
		s = "Invalid command";
		break;
	case NO_INPUT_FILES:
		s = "No input files";
		break;	
	case NO_OUTPUT_FILES:
		s = "No output files";
		break;
	case FILE_DOES_NOT_EXIST:
		s = "File does not exist";
		break;
	case PATH_IS_NOT_A_FILE:
		s = "Path is not a file";
		break;
	case FAILED_TO_OPEN_MAP:
		s = "Failed to open map file";
		break;
	case MAP_READ_ERROR:
		s = "Failed to read map";
		break;
	case FAILED_TO_SAVE_MAP:
		s = "Failed to save map file";
		break;
	case MAP_PARSE_ERROR:
		s = "Map parse error";
		break;
	case MISSING_MAP_NODE:
		s = "Missing map node";
		break;
	case MISSING_POS_ATTRIBUTE:
		s = "Missing pos attribute";
		break;
	case MISMATCHED_INPUT_OUTPUT_COUNT:
		s = "Mismatch input and output file count";
		break;
	case MAP_ITEM_READ_ERROR:
		s = "Map item read error";
		break;
	case BAD_ALLOC:
		s = "Bad alloc. Failed to allocate memory";
		break;
	case NO_TRANSFORMS:
		s = "No transforms given.";
		break;
	default:
		s = "Unspecified error";
		break;
	}

	return s;
}
