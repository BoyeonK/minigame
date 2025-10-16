#pragma once
#include "XYZ.h"
#include "GameType.h"

class UnityGameObject {
public:
	void SetObjectId(int32_t id);
	int32_t GetObjectId() const;

	int32_t GetObjectTypeInteger() const;

	S2C_Protocol::XYZ SerializePosition() const;
	void SerializePosition(S2C_Protocol::XYZ* pXYZ) const;

	S2C_Protocol::UnityGameObject SerializeObject() const;
	void SerializeObject(S2C_Protocol::UnityGameObject* pObj) const;

	void SetPosition(float x, float y, float z);
	XYZ GetPosition() const;
	virtual void Update() = 0;

protected:
	int32_t _objectId;
	GameObjectType _objectType;
	XYZ _position;
	//XYZ _Rotation;
	bool _isPool = false;
};

