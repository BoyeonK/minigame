#pragma once
struct XYZ {
	XYZ(): x(0), y(0), z(0) {};
	XYZ(float X, float Y, float Z) : x(X), y(Y), z(Z) {};

	void SetPosition(float X, float Y, float Z);
    float Magnitude() const;
    void Normalize();
	XYZ Normalized() const;

    XYZ operator*(float scalar) const {
        return XYZ(x * scalar, y * scalar, z * scalar);
    }
    XYZ operator*(int scalar) const {
        return XYZ(x * scalar, y * scalar, z * scalar);
    }
    XYZ& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this; 
    }

	float x = 0;
	float y = 0;
	float z = 0;
};

inline XYZ operator*(float scalar, const XYZ& vec) {
	return vec * scalar;
}

inline XYZ operator*(int scalar, const XYZ& vec) {
	return vec * scalar;
}