#include "transform.hpp"
#include "errors.hpp"
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <rapidxml/rapidxml_print.hpp>

//NAMESPACES
namespace fs = boost::filesystem;
namespace rx = rapidxml;

//FUNCTIONS
namespace i99 {
	bool isPositionableItem(rx::xml_node<> *p_itemNode, int* err);
	
	int readMapItemPosAttr(rx::xml_attribute<> *p_attribute, float& f);
	
	int transformMapItem(rx::xml_node<> *p_itemNode, transformQueue const& funcs); 
	glm::vec3 transformPoint(glm::vec3 o, transformQueue const& funcs);
}

glm::vec3 i99::transformPoint(glm::vec3 o, transformQueue const& funcs)
{
	auto it = std::begin(funcs);
	for(; it != std::end(funcs); it++)	{
		o = (*it)(o);
	}

	return o;
}

int i99::readMapItemPosAttr(rx::xml_attribute<> *p_attribute, float& f) {			
	int r = 0;
		
	if(p_attribute)
	{
		try { 
			f = boost::lexical_cast<float>(p_attribute->value());
		} catch(boost::bad_lexical_cast& ex) {
			r = MAP_ITEM_READ_ERROR;
		}
		
		return r;
	}
	else
		return MISSING_POS_ATTRIBUTE;
}

bool i99::isPositionableItem(rx::xml_node<> *p_itemNode, int* err)
{
	return strcmp(p_itemNode->name(), "object") == 0
			|| strcmp(p_itemNode->name(), "pickup") == 0 
			|| strcmp(p_itemNode->name(), "spawnpoint") == 0  
			|| strcmp(p_itemNode->name(), "checkpoint") == 0;
}

int i99::readMapFile(fs::path const& path, rx::xml_document<>& doc)
{	
	if( !fs::exists(path) )
		return FILE_DOES_NOT_EXIST;
	else if(!fs::is_regular_file(path) && !fs::is_symlink(path) )
		return PATH_IS_NOT_A_FILE;

	fs::ifstream fs(path);
	if(!fs)	
		return FAILED_TO_OPEN_MAP;		
	

	//Read the contents of the file into a std::vector.
	std::vector<char> mapData;
	fs >> std::noskipws;
	std::copy(
		(std::istream_iterator<char>(fs)),
		std::istream_iterator<char>(),
		std::back_inserter(mapData)
	);
	mapData.push_back('\0');	

	if(!fs && !fs.eof() )		
		return MAP_READ_ERROR;	

	//Parse the document.	
	try  {	
		doc.clear();
		char* docStr = doc.allocate_string(mapData.data(), mapData.size());	
		doc.parse<0>(docStr);		
	} 

	catch(rx::parse_error const& ex)
	{
		std::cout << "Parse error: " << ex.what() << " : " << ex.where<char>() << std::endl;
		return MAP_PARSE_ERROR;		
	}

	return NO_ERROR;
}



glm::vec3 i99::findMapCenter(rapidxml::xml_document<>& doc, int* p_err)
{
	int r = NO_ERROR;
	glm::vec3 c;

	//Get the root map node.
	rx::xml_node<> *p_mapNode = doc.first_node("map");
	if(p_mapNode == nullptr)
		r = MISSING_MAP_NODE;
		
	//Bounds
	float xMin = 0.0f, xMax = 0.0f, 
		   yMin = 0.0f, yMax = 0.0f,
		   zMin = 0.0f, zMax = 0.0f;

	
	rx::xml_node<> *p_itemNode = p_mapNode->first_node();
	
	//Find the minimum bounds.
	if(p_itemNode != nullptr) {
		do { 
		
			if( isPositionableItem(p_itemNode, &r) ) {
				glm::vec3 v;
				rx::xml_attribute<> *p_attr = nullptr;
				if(r == NO_ERROR) p_attr = p_itemNode->first_attribute("posX"); r = readMapItemPosAttr(p_attr, v.x);
				if(r == NO_ERROR) p_attr = p_itemNode->first_attribute("posY"); r = readMapItemPosAttr(p_attr, v.y);
				if(r == NO_ERROR) p_attr = p_itemNode->first_attribute("posZ"); r = readMapItemPosAttr(p_attr, v.z);
			
				if(r == NO_ERROR) {
					//Set the bounds to the first item
					xMin = v.x; xMax = v.x;
					yMin = yMax = v.y;
					zMin = zMax = v.z;				
				}
			}
			
		} while( !isPositionableItem(p_itemNode, &r) && (p_itemNode = p_itemNode->next_sibling()) != nullptr);
	}
	
	
	//Iterate through each node to find the bounds.
	for(; r == NO_ERROR && p_itemNode != nullptr; p_itemNode = p_itemNode->next_sibling() )
	{
		//Check if each item is beyond the current bounds.		
		if(isPositionableItem(p_itemNode, &r) )
		{
			glm::vec3 v;
			rx::xml_attribute<> *p_attr = nullptr;
			if(r == NO_ERROR) p_attr = p_itemNode->first_attribute("posX"); r = readMapItemPosAttr(p_attr, v.x);
			if(r == NO_ERROR) p_attr = p_itemNode->first_attribute("posY"); r = readMapItemPosAttr(p_attr, v.y);
			if(r == NO_ERROR) p_attr = p_itemNode->first_attribute("posZ"); r = readMapItemPosAttr(p_attr, v.z);
			
			if(r == NO_ERROR) {
				//Check and store the bounds
				xMin = std::min(xMin, v.x);		xMax = std::max(xMax, v.x);				
				yMin = std::min(yMin, v.y);		yMax = std::max(yMax, v.y);				
				zMin = std::min(zMin, v.z);		zMax = std::max(zMax, v.z);
			}
		}
		  
	}
	
	//Calculate the center.
	c.x = (xMax + xMin) / 2.0f;
	c.y = (yMax + yMin) / 2.0f;
	c.z = (zMax + zMin) / 2.0f;
	
	std::cout << "Bounds: " << std::endl;
	std::cout << xMin << " " << xMax << std::endl;
	std::cout << yMin << " " << yMax << std::endl;
	std::cout << zMin << " " << zMax << std::endl;
	std::cout << "Center: " << c.x << " " << c.y << " " << c.z << std::endl;

	if(p_err)
	{
		*p_err = r;
		if(r != NO_ERROR) return glm::vec3();
	}
	
	return c;
}

int i99::transformMapItem(rx::xml_node<> *p_itemNode, transformQueue const& funcs) { 
		
	int r = NO_ERROR;
	glm::vec3 v;
		
	//Read and convert the attributes.
	rx::xml_attribute<> *p_attr = nullptr;
	
	if(r == NO_ERROR) p_attr = p_itemNode->first_attribute("posX"); r = readMapItemPosAttr(p_attr, v.x);
	if(r == NO_ERROR) p_attr = p_itemNode->first_attribute("posY"); r = readMapItemPosAttr(p_attr, v.y);
	if(r == NO_ERROR) p_attr = p_itemNode->first_attribute("posZ"); r = readMapItemPosAttr(p_attr, v.z);
	

	//Transform and store the point
	if(r == NO_ERROR)
		v = transformPoint(v, funcs);
		
		
	//Function to write the value to the document.
	auto writePosAttr = [](rx::xml_attribute<> *p_attribute, float f) -> int{
		if(p_attribute)
		{
			int r = NO_ERROR;
			try { 
				std::string s = boost::lexical_cast<std::string>(f);				
				
				rx::xml_document<> *p_doc = p_attribute->document();
				char* xmlStr = p_doc->allocate_string(s.c_str(), s.size());
				
				p_attribute->value(xmlStr, s.size());
			}

			catch(boost::bad_lexical_cast& ex) { 
				r = MAP_ITEM_READ_ERROR;
			}
			catch(std::bad_alloc) {
				r = BAD_ALLOC;
			}
			
			return r;
		}
		else
			return MISSING_POS_ATTRIBUTE;
				
	};		

	//Modify the item.
	if(r == NO_ERROR)p_attr = p_itemNode->first_attribute("posX"); r = writePosAttr(p_attr, v.x);
	if(r == NO_ERROR)p_attr = p_itemNode->first_attribute("posY"); r = writePosAttr(p_attr, v.y);
	if(r == NO_ERROR)p_attr = p_itemNode->first_attribute("posZ"); r = writePosAttr(p_attr, v.z);
	
	return r;		
}

int i99::transformMap(rapidxml::xml_document<>& doc, transformQueue const& funcs)
{
	int result = NO_ERROR;

	//Get the root map node.
	rx::xml_node<> *p_mapNode = doc.first_node("map");
	if(p_mapNode == nullptr)
		return MISSING_MAP_NODE;

	//Iterate through each node.
	rx::xml_node<> *p_itemNode = p_mapNode->first_node();
	for(; result == NO_ERROR && p_itemNode != nullptr; p_itemNode = p_itemNode->next_sibling() )
	{
		//Transform the map item.		
		if( isPositionableItem(p_itemNode, &result) ) 
			result = transformMapItem(p_itemNode, funcs);			
	}

	return result;
}


//Function generators

//Generates a function that takes a center and makes the map relative to that center.
namespace i99 { 
	MapItemTransformFunc gen_makeObjectRelative(glm::vec3 center)
	{
		return [=](glm::vec3 o)  {
			return o - center;
		};
	}

	MapItemTransformFunc gen_makeObjectRelative(rapidxml::xml_document<>& doc, int *p_err)
	{
		int err = 0;
		glm::vec3 c = findMapCenter(doc, &err);
		if(err != NO_ERROR) {
			if(p_err) *p_err = err;
			return MapItemTransformFunc();
		}
		
		if(p_err) *p_err = err;
		return i99::gen_makeObjectRelative(c);
	}
}
