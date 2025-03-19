#pragma once
#include "Object.h"

class ObjectManager {
public:
	ObjectManager();
	~ObjectManager();
	void init(Graphics* gfx);
	uint32_t createAnObject(
		const DirectX::XMFLOAT3& position = DirectX::XMFLOAT3(0,0,0),
		const DirectX::XMFLOAT3& rotation = DirectX::XMFLOAT3(0, 0, 0),
		const DirectX::XMFLOAT3& scale = DirectX::XMFLOAT3(1, 1, 1)
		);
	uint32_t createAnObject(
		const std::string& nameID,
		const DirectX::XMFLOAT3& position = DirectX::XMFLOAT3(0, 0, 0),
		const DirectX::XMFLOAT3& rotation = DirectX::XMFLOAT3(0, 0, 0),
		const DirectX::XMFLOAT3& scale = DirectX::XMFLOAT3(1, 1, 1)
	);
	Object* getObject(uint32_t index);
	Object* getObject(const std::string& id);
	std::vector<Object*>& getAllObjects();
private:
	Graphics* gfx;
	std::vector<Object*> objectsWithId;
	std::unordered_map<std::string, Object*> objectsWithName;
};