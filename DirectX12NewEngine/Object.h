#pragma once
#include <unordered_map>
#include "Components.h"
#include "CommonHeaders.h"
#include <typeindex>
#include "ConstantBuffers.h"

class Object {
public:
	Object(Graphics* gfx);
	Object();
	virtual ~Object();

	void setPosition(const DirectX::XMFLOAT3& position);
	void setRotation(const DirectX::XMFLOAT3& rotation);
	void setScale(const DirectX::XMFLOAT3& scale);

	void move(const DirectX::XMFLOAT3& direction);
	void rotate(const DirectX::XMFLOAT3& angles);
	void scale(const DirectX::XMFLOAT3& scale);
	void scale(const float& scale);

	DirectX::XMFLOAT3& getPosition();
	DirectX::XMFLOAT3& getRotation();
	DirectX::XMFLOAT3& getScale();

	void setConstantBuffer(Graphics* gfx);//move this to objectHandlerMayber?
	DirectX::XMMATRIX CreateTransformationMatrix();
	
	template<typename T>
	void addComponent(const T* componentObject)
	{
#ifdef _DEBUG
		static_assert(std::is_base_of_v<Component, T>, "must be derived from Component");
		if constexpr (!std::is_base_of<Component, T>::value)
		{
			std::cout << typeid(T).name() << " does not derive from Component" << std::endl;
			return;
		}
#endif
		
		if (components.find(typeid(T)) == components.end())
		{
			components.insert(
				std::pair<std::type_index, std::vector<Component*>>
					(
						std::type_index(typeid(T)), 
						std::vector<Component*>()
					)
			);
		}
		components[typeid(T)].push_back((Component*)componentObject);
	}


	template<typename T>
	T* getComponent(uint32_t index = 0) {
		auto it = components.find(std::type_index(typeid(T)));
		if (it != components.end() && index < it->second.size())
		{
			return (T*)it->second[index];
		}
		return nullptr;
	}
protected:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scal;

	ConstantBuffer transformBuffer;
	std::unordered_map<std::type_index, std::vector<Component*>> components;
};