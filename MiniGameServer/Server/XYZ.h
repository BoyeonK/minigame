#pragma once
struct XYZ {
	XYZ(): x(0), y(0), z(0) {};
	XYZ(float X, float Y, float Z) : x(X), y(Y), z(Z) {};
	void SetPosition(float X, float Y, float Z);

	float x = 0;
	float y = 0;
	float z = 0;
};
