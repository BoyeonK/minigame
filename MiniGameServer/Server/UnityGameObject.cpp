#include "pch.h"
#include "UnityGameObject.h"

S2C_Protocol::XYZ UnityGameObject::SerializePosition() const {
    S2C_Protocol::XYZ serialized;
    serialized.set_x(_position.x);
    serialized.set_y(_position.y);
    serialized.set_z(_position.z);
    return serialized;
}

S2C_Protocol::UnityGameObject UnityGameObject::SerializeObject() const {
    S2C_Protocol::UnityGameObject serialized;
    serialized.set_objectid(_objectId);
    serialized.set_objecttype(static_cast<int>(_objectType));
    S2C_Protocol::XYZ* pos_ptr = serialized.mutable_position();
    pos_ptr->set_x(_position.x);
    pos_ptr->set_y(_position.y);
    pos_ptr->set_z(_position.z);
    return serialized;
}

void UnityGameObject::SerializeObject(S2C_Protocol::UnityGameObject* dest) const {
    if (dest == nullptr)
        return;

    dest->set_objectid(_objectId);
    dest->set_objecttype(static_cast<int>(_objectType));
    S2C_Protocol::XYZ* pos_ptr = dest->mutable_position();
    pos_ptr->set_x(_position.x);
    pos_ptr->set_y(_position.y);
    pos_ptr->set_z(_position.z);
}

void UnityGameObject::SetPosition(float x, float y, float z) {
    _position.SetPosition(x, y, z);
}

XYZ UnityGameObject::GetPosition() const {
    return _position;
}
