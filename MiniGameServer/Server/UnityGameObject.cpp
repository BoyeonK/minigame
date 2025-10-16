#include "pch.h"
#include "UnityGameObject.h"

void UnityGameObject::SetObjectId(int32_t id) {
    _objectId = id;
}

int32_t UnityGameObject::GetObjectId() const {
    return _objectId;
}

int32_t UnityGameObject::GetObjectTypeInteger() const {
    int32_t ans = (int)_objectType;
    return ans;
}

S2C_Protocol::XYZ UnityGameObject::SerializePosition() const {
    return _position.Serialize();
}

void UnityGameObject::SerializePosition(S2C_Protocol::XYZ* pXYZ) const {
    _position.Serialize(pXYZ);
}

S2C_Protocol::UnityGameObject UnityGameObject::SerializeObject() const {
    S2C_Protocol::UnityGameObject serialized;
    serialized.set_objectid(_objectId);
    serialized.set_objecttype(static_cast<int>(_objectType));
    _position.Serialize(serialized.mutable_position());
    return serialized;
}

void UnityGameObject::SerializeObject(S2C_Protocol::UnityGameObject* pObj) const {
    if (pObj == nullptr)
        return;

    pObj->set_objectid(_objectId);
    pObj->set_objecttype(static_cast<int>(_objectType));
    _position.Serialize(pObj->mutable_position());
}

void UnityGameObject::SetPosition(float x, float y, float z) {
    _position.SetPosition(x, y, z);
}

XYZ UnityGameObject::GetPosition() const {
    return _position;
}
