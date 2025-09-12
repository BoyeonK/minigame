#pragma once
#include "XYZ.h"
#include "GameType.h"

class UnityGameObject {

protected:
	int32_t _objectId;
	GameObjectType _objectType;
	XYZ _Position;
	//XYZ _Rotation;
};

