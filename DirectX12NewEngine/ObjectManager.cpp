#include "ObjectManager.h"

ObjectManager::ObjectManager()
{
}

ObjectManager::~ObjectManager()
{
    for (int i = 0; i < objectsWithId.size(); i++)
    {
        delete objectsWithId[i];
    }
    for (auto& it : objectsWithName) {
        delete it.second;
    }
}

void ObjectManager::init(Graphics* gfx)
{
    this->gfx = gfx;
}

uint32_t ObjectManager::createAnObject(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& rotation,
    const DirectX::XMFLOAT3& scale
)
{
    objectsWithId.push_back(new Object(gfx));
    objectsWithId.back()->setPosition(position);
    objectsWithId.back()->setRotation(rotation);
    objectsWithId.back()->setScale(scale);
    return objectsWithId.size() - 1;
}

uint32_t ObjectManager::createAnObject(const std::string& nameID, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& scale)
{
    Object* theObject = new Object(gfx);
    objectsWithId.push_back(theObject);
    objectsWithName.insert(std::pair<std::string, Object*>(nameID, theObject));
    theObject->setPosition(position);
    theObject->setRotation(rotation);
    theObject->setScale(scale);
    return objectsWithId.size() - 1;
}
std::vector<Object*>& ObjectManager::getAllObjects() 
{
    return objectsWithId;
}

Object* ObjectManager::getObject(uint32_t index)
{
    return objectsWithId[index];
}

Object* ObjectManager::getObject(const std::string& id)
{
    return objectsWithName[id];
}
