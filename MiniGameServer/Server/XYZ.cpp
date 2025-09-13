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
