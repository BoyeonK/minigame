#pragma once
#include "XYZ.h"
#include "GameType.h"

class UnityGameObject {
public:
	S2C_Protocol::XYZ SerializePosition() const;
	S2C_Protocol::UnityGameObject SerializeObject() const;
	void SerializeObject(S2C_Protocol::UnityGameObject* dest) const;
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

