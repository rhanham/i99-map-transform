#pragma once
#include <deque>
#include <glm/glm.hpp>
#include <boost/filesystem.hpp>
#include <rapidxml/rapidxml.hpp>


//NAMESPACES
namespace fs = boost::filesystem;
namespace rx = rapidxml;

namespace i99
{	
	//TYPEDEFS
	typedef std::function<glm::vec3(glm::vec3)> MapItemTransformFunc;	
	typedef std::deque<MapItemTransformFunc> transformQueue;
	
	//FUNCTIONS
	int 	   readMapFile(fs::path const& path, rx::xml_document<>& doc);	
	int       transformMap(rapidxml::xml_document<>& doc, transformQueue const& funcs);
	glm::vec3 transformObject(glm::vec3 o, transformQueue const& funcs);
	
	glm::vec3 findMapCenter(rapidxml::xml_document<>& doc, int* p_err);
	
	//Functions to generate transform functions to be passed into the transform queue
	MapItemTransformFunc gen_makeObjectRelative(rapidxml::xml_document<>& doc, int* err);
	MapItemTransformFunc gen_makeObjectRelative(glm::vec3 center);
	
}
