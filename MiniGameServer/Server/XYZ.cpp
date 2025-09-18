#include "pch.h"
#include "XYZ.h"

void XYZ::SetPosition(float X, float Y, float Z) {
	x = X;
	y = Y;
	z = Z;
}

float XYZ::Magnitude() const {
	return sqrt(x * x + y * y + z * z);
}

S2C_Protocol::XYZ XYZ::Serialize() const {
    S2C_Protocol::XYZ vector3;
    vector3.set_x(x);
    vector3.set_y(y);
    vector3.set_z(z);
    return vector3;
}

void XYZ::Serialize(S2C_Protocol::XYZ* pXYZ) const {
    if (pXYZ == nullptr)
        return;

    pXYZ->set_x(x);
    pXYZ->set_y(y);
    pXYZ->set_z(z);
}

void XYZ::Normalize() {
    float mag = Magnitude();
    if (mag > 0) { // 0으로 나누는 것을 방지
        x /= mag;
        y /= mag;
        z /= mag;
    }
}

XYZ XYZ::Normalized() const {
    float mag = Magnitude();
    if (mag > 0) 
        return XYZ(x / mag, y / mag, z / mag);

    return XYZ(0, 0, 0);
}
