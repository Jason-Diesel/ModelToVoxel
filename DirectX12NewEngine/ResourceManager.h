#pragma once
#include <vector>
#include <functional>
#include <unordered_map>
#include <string>

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();
	void Destroy();

	template<typename T>
	void addResource(T* resource)
	{
		data.emplace_back(
			resource,
			std::function<void(void*)>([](void* ptr) { delete static_cast<T*>(ptr); })
		);
		//static int nameIndex = 0;
		//data.emplace_back(
		//	resource,
		//	std::pair(
		//		std::function<void(void*)>([](void* ptr) { delete static_cast<T*>(ptr); }),
		//		std::to_string(nameIndex)
		//		)
		//);
	}
	//template<typename T>
	//void addResourceName(T* resource, const std::string name)
	//{
	//	data.emplace_back(
	//		resource,
	//		std::pair(
	//			std::function<void(void*)>([](void* ptr) { delete static_cast<T*>(ptr); }),
	//			name
	//		)
	//	);
	//}
	template<typename T>
	void addResource(T* resource, std::string name)
	{
		addResource(resource);
		//addResourceName(resource, name);
		searchMap.insert(std::pair<std::string, uint32_t>(name, data.size() - 1));
	}
	
	template<typename T>
	T* getResource(const std::string& name)
	{
		if (searchMap.count(name) == 0)
		{
			return nullptr;
		}
		return (T*)data[searchMap[name]].first;
	}
	void deleteResource(const std::string& name);

private:
	//std::vector<std::pair<void*, std::pair<std::function<void(void*)>, std::string>>> data;//pointer, delete function//ONLY USE IN WHEN DEBUGGING SHIT
	std::vector<std::pair<void*, std::function<void(void*)>>> data;//pointer, delete function
	std::unordered_map<std::string, uint32_t> searchMap;//name, position in data
};