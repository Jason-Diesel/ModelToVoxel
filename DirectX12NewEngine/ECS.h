#pragma once
/*
#include "Components.h"
#include "unordered_map"
#include <vector>

class ECS {
public:
	template<typename T>
	void* addComponent(T component) {
		auto it = intTOIndex.find(typeid(T).hash_code());
		if (it == intTOIndex.end())
		{
			intTOIndex.insert(std::pair<uint32_t, uint32_t>(typeid(T).hash_code(), data.size()));
			data.resize(data.size() + 1);

			//std::pair<uint32_t, std::vector<void*>>
			data.back().first = 1;
			data.back().second.resize(1);
			data.back().second[0] = malloc(sizeof(T) * componentArraySize);

			return data.back().second[0];
		}
		else 
		{
			std::pair<uint32_t, std::vector<void*>> componentArrays = data[it.second];
			componentArrays.first++;
			int nrOfComponentsArrays = (componentArrays.first / componentArrays) + 1;
			if (nrOfComponentsArrays < componentArrays.second.size())
			{
				componentArrays.second.resize(componentArrays.second.size());
			}
			int componentIndex = componentArrays.first % componentArraySize;
			T* componentStack = data.back().second.back();
			memcpy(componentStack[componentIndex], component, sizeof(T));
			return componentStack[componentIndex];
		}
	}

	template<typename T>
	T* getComponent(void* addr) {
		return (T*)addr;
	}
	
private:
	const uint32_t componentArraySize = 40;
	std::unordered_map<uint32_t, uint32_t> intTOIndex;
	std::unordered_map<int, uint32_t> Extra;

	//What type of component
	//how many Components In array
	//component array
	//component data
	std::vector<std::pair<uint32_t, std::vector<void*>>> data;


	std::vector<void*> tempData;

};
*/