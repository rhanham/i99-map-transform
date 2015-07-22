#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <functional>
#include <boost/any.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>
#include <glm/glm.hpp>

#include "errors.hpp"
#include "options.hpp"
#include "transform.hpp"

//NAMESPACES
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace rx = rapidxml;

//TYPEDEFS
namespace i99 {	
	typedef std::vector<fs::path> filepathVector;
}

//GLOBALS
namespace i99 {		
	static std::deque<MapItemTransformFunc> g_transforms;
	static filepathVector     g_inputs;
	static filepathVector     g_outputs;
}

//FUNCTIONS
namespace i99 { 
	int handleCommands(po::variables_map const& vm, po::options_description const& desc);
	
	int transformMaps(po::variables_map const& vm,
			filepathVector const& inputs,
			filepathVector const& outputs,
			std::deque<MapItemTransformFunc> const& transforms);
			
	glm::vec3 stdVectorToVec (std::vector<float> const& v);
}

int main(int argc, char **argv)
{
	
	//Declare a group of hidden general command line options
	po::options_description cmdline_hidden;
	cmdline_hidden.add_options()
		(i99::opt_inputFiles.name, po::value< i99::filepathVector >(), i99::opt_inputFiles.desc )		
		
	;	

	//Declare a group of visible general command line options
	po::options_description cmdline_visible("General");
	cmdline_visible.add_options()
		(i99::opt_help.name, i99::opt_help.desc)
		(i99::opt_outputFiles.name, po::value< i99::filepathVector >(), i99::opt_outputFiles.desc )		
	;

	//Declare a group of transform commands
	po::options_description transform("Transforms");
	transform.add_options()
		(i99::opt_translate.name, 
			po::value< std::vector<float> >()->multitoken()->notifier(
				[](boost::any const& a) -> void
				{
					//Get the vector.
					std::vector<float> v = boost::any_cast<std::vector<float> >(a);
					glm::vec3 vec = i99::stdVectorToVec(v);	
					
					auto f = [=](glm::vec3 o) { return o + vec;};
					i99::g_transforms.push_back(f);
				}),
			i99::opt_translate.desc)
		(i99::opt_makeRelative.name, po::value<bool>()->default_value(false), i99::opt_makeRelative.desc)
	;
	
	po::options_description visible("Allowed options");
	visible.add(cmdline_visible).add(transform);

	po::options_description cmdline_options;
	cmdline_options.add(cmdline_visible).add(transform).add(cmdline_hidden);
	
	//Set up the positional options
	po::positional_options_description p;
	p.add(i99::opt_inputFiles.name, -1);
	

	//Process the commands
	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).
			options(cmdline_options).positional(p).run(), vm);
		po::notify(vm);
	} catch(...)
	{
		std::cout << visible << std::endl;
		return 0;
	}

	//Handle the commands
	int result = i99::handleCommands(vm, visible);
	
	//Transform the map
	result = i99::transformMaps(vm, i99::g_inputs, i99::g_outputs, i99::g_transforms);		
	
	if(result != i99::NO_ERROR)
	{
		std::cerr << "********************************************\n";
		std::cerr << "Error " << result <<": " << i99::errorNumberToString(result) << std::endl;
		std::cerr << "********************************************\n";
	}

	return result;
}

glm::vec3 i99::stdVectorToVec(std::vector<float> const& v) {
		glm::vec3 vec;
		vec.x = ( v.size() >= 1 ) ? v[0] : 0.0f;
		vec.y = ( v.size() >= 2 ) ? v[1] : 0.0f;
		vec.z = ( v.size() >= 3 ) ? v[2] : 0.0f;
	
		return vec;
};



int i99::handleCommands(po::variables_map const& vm, po::options_description const& desc)
{
	int result = -1;

	//Create a lambda for returning if the command should execute.
	auto cont = [&]() { return result == -1 || result == NO_ERROR; };

	if(vm.count(i99::opt_help.name)) { 
		std::cout << desc << std::endl;
		return NO_ERROR;
	}
	
	if(cont() && vm.count(i99::opt_inputFiles.name) ) {
		filepathVector const& v = vm[i99::opt_inputFiles.name].as< filepathVector >();		
		std::for_each( std::begin(v), std::end(v), [](fs::path const& s) { 
			i99::g_inputs.push_back(s);
		});
		

		result = NO_ERROR;
	} else {
		result =  NO_INPUT_FILES;
	}

	if(cont() && vm.count(i99::opt_outputFiles.name) ) {
		filepathVector const& v = vm[i99::opt_outputFiles.name].as< filepathVector >();		
		std::for_each( std::begin(v), std::end(v), [](fs::path const& s) { 
			i99::g_outputs.push_back(s);
		});
		
		result = NO_ERROR;
	}
	
	if(cont() && vm.count(i99::opt_makeRelative.name) ) {		
		result = NO_ERROR;
	}

	if(cont() && vm.count(i99::opt_translate.name) ) {
		result = NO_ERROR;
	}

	if(result == -1)
		result = INVALID_COMMAND;

	return result;
} 

int i99::transformMaps(po::variables_map const& vm,
		filepathVector const& inputs,
		filepathVector const& outputs,
		std::deque<MapItemTransformFunc> const& transforms)
{
	
	int result = NO_ERROR;
	if(outputs.size() != inputs.size())
		result = MISMATCHED_INPUT_OUTPUT_COUNT;
		
	if(result == NO_ERROR)
	{
		//For each map: Read the input file, apply the transforms and write to the output file.
		auto outIt = std::begin(outputs);
		std::for_each( std::begin(inputs), std::end(inputs), [&](fs::path const& mapPath) {
			
			if(result != NO_ERROR)
				return;
				
			//Read the map contents
			rx::xml_document<> doc;
			int r = readMapFile(mapPath, doc);
			
			//Setup the transform pipeline.
			std::deque<MapItemTransformFunc> pre_transforms;
			std::deque<MapItemTransformFunc> post_transforms;
	
			//Get the pre transforms
			if(r == NO_ERROR)
			{
				if( vm.count(opt_makeRelative.name) && vm[opt_makeRelative.name].as<bool>()) { 
					pre_transforms.push_back( gen_makeObjectRelative(doc, &r) );
				}
			}
			
			auto pushBackFuncQueue = [](std::deque<MapItemTransformFunc>& q, std::deque<MapItemTransformFunc>const & i)
			{
				std::for_each( i.begin(), i.end(), [&](MapItemTransformFunc const& f) {
					q.push_back(f);
				});
			};		
	
			//Merge the queues.
			std::deque<MapItemTransformFunc> transform_queue;
			pushBackFuncQueue(transform_queue, pre_transforms);
			pushBackFuncQueue(transform_queue, transforms);
			pushBackFuncQueue(transform_queue, post_transforms);

			//Transform the contents of the map
			if(r == NO_ERROR)
				r = transformMap(doc, transform_queue);

			//Output the map contents
			if(r == NO_ERROR && outIt == outputs.end() )
				r = MISMATCHED_INPUT_OUTPUT_COUNT;
			
			//Create the file.							
			if(r == NO_ERROR)
			{
				fs::ofstream output(*outIt);
				if(!output)
					r = FAILED_TO_SAVE_MAP;
					
				//Write out the contents.			
				output << doc;				
			}			
			
			
			//Store any errors.
			if(r != NO_ERROR)
				result = r;
				
			++outIt;
			
		});
		
		
	}
	
	return result;
}
