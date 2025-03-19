#include "ResourceManager.h"

#include <iostream>

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::Destroy()
{
	for (auto& pair : data)
	{
		if (pair.first)
		{
			//std::cout << "Deleting : " << pair.second.second << std::endl;
			pair.second(pair.first);
		}
	}
}

void ResourceManager::deleteResource(const std::string& name)
{
	if (searchMap.count(name))
	{
		return;//ERROR
	}
	//Delete from memory
	data[searchMap[name]].second(data[searchMap[name]].first);
	//data[searchMap[name]].second.first(data[searchMap[name]].first);
	//Delete from vector
	data.erase(data.begin() + searchMap[name]);
	//Delete from map
	searchMap.erase(name);
}
