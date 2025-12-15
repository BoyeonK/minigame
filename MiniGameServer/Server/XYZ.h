#pragma once
struct XYZ {
	XYZ(): x(0), y(0), z(0) {};
	XYZ(float X, float Y, float Z) : x(X), y(Y), z(Z) {};

	void SetPosition(float X, float Y, float Z);
    float Magnitude() const;
    S2C_Protocol::XYZ Serialize() const;
    void Serialize(S2C_Protocol::XYZ* pXYZ) const;
    void DeserializeFrom(const S2C_Protocol::XYZ& XYZ);
    void Normalize();
	XYZ Normalized() const;

	XYZ operator+(const XYZ& other) const {
		return XYZ(x + other.x, y + other.y, z + other.z);
	}

	XYZ operator-(const XYZ& other) const {
		return XYZ(x - other.x, y - other.y, z - other.z);
	}

	XYZ& operator+=(const XYZ& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	XYZ& operator-=(const XYZ& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	XYZ operator-() const {
		return XYZ(-x, -y, -z);
	}

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

	float x;
	float y;
	float z;
};

inline XYZ operator*(float scalar, const XYZ& vec) {
	return vec * scalar;
}

inline XYZ operator*(int scalar, const XYZ& vec) {
	return vec * scalar;
}