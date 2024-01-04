// Assignment 1d - Transparency and Mirror Reflections
// Work by Jacob Secunda
#ifndef VECTOR3D_H
#define VECTOR3D_H

#include "Point.hpp"
#include "TypeDefinitions.hpp"

struct Vector3D {
	float dx;
	float dy;
	float dz;

public:
	Vector3D() = default;

	Vector3D(float dx, float dy, float dz)
			:
			dx(dx),
			dy(dy),
			dz(dz)
	{
	}

	// Description: Creates a vector from a 'start' and an 'end' point.
	Vector3D(const Point3D &start, const Point3D &end) {
		dx = end.x - start.x;
		dy = end.y - start.y;
		dz = end.z - start.z;
	}

	// Description: Calculates the length of this vector.
	[[nodiscard]] inline float
	Length() const {
		return sqrt((dx * dx) + (dy * dy) + (dz * dz));
	}

	// Description: Converts this vector into a unit vector.
	// 	- This is a vector with length 1, though with the same direction.
	void
	NormalizeSelf() {
		float vectorLength = Length();
		// We can't divide by zero!!!
		if (std::fpclassify(vectorLength) == FP_ZERO) {
			// This isn't ideal...but let's not crash at least!
			return;
		}

		dx /= vectorLength;
		dy /= vectorLength;
		dz /= vectorLength;
	}

	// Description: Calculates the unit vector of this vector and returns it.
	// 	- This is a vector with length 1, though with the same direction.
	[[nodiscard]] Vector3D
	Normalize() const {
		Vector3D norm = *this;
		norm.NormalizeSelf();
		return norm;
	}

	// Description: Obtain angle between directions of this vector and 'v'.
	// 	- Note: Vectors are Orthogonal when Dot Product == 0.
	[[nodiscard]] float
	DotProduct(const Vector3D &v) const {
		return (dx * v.dx) + (dy * v.dy) + (dz * v.dz);
	}

	// Description: Finds the vector normal/perpendicular to this Vector 3D and 'v'.
	[[nodiscard]] Vector3D
	CrossProduct(const Vector3D &v) const {
		Vector3D normal{};

		normal.dx = (this->dy * v.dz) - (this->dz * v.dy);
		normal.dy = -1.f * ((this->dx * v.dz) - (this->dz * v.dx));
		normal.dz = (this->dx * v.dy) - (this->dy * v.dx);

		return normal;
	}

	// Description: Add vectors together to create a vector connecting this vector and 'vector' together.
	constexpr Vector3D &
	operator+=(const Vector3D &vector) {
		dx += vector.dx;
		dy += vector.dy;
		dz += vector.dz;

		return *this;
	}

	// Description: Lengthen/shorten this vector by 'scalar' amount.
	constexpr Vector3D &
	operator*=(const float &scalar) {
		dx *= scalar;
		dy *= scalar;
		dz *= scalar;

		return *this;
	}

	// Description: Shorten/lengthen this vector by 'scalar' amount.
	constexpr Vector3D &
	operator/=(const float &scalar) {
		dx /= scalar;
		dy /= scalar;
		dz /= scalar;

		return *this;
	}

	auto operator<=>(const Vector3D &other) const = default;
};

MAKE_SHARED_NAME(Vector3D);

static Vector3D
operator*(const Vector3D &vector, const float &scalar) {
	Vector3D scaledVector = vector;
	scaledVector *= scalar;
	return scaledVector;
}

static Vector3D
operator+(const Vector3D &vectorA, const Vector3D &vectorB) {
	Vector3D vectorC{};

	vectorC.dx = vectorA.dx + vectorB.dx;
	vectorC.dy = vectorA.dy + vectorB.dy;
	vectorC.dz = vectorA.dz + vectorB.dz;

	return vectorC;

}

static Vector3D
operator-(const Vector3D &vectorA, const Vector3D &vectorB) {
	Vector3D vectorC = vectorA;
	vectorC.dx -= vectorB.dx;
	vectorC.dy -= vectorB.dy;
	vectorC.dz -= vectorB.dz;
	return vectorC;
}

static std::ostream &
operator<<(std::ostream &out, const Vector3D &vector) {
	out << "(dx: " << std::to_string(vector.dx) << ", dy: " << std::to_string(vector.dy) << ", dz: "
		<< std::to_string(vector.dz) << ")";
	return out;
}

// Point Operator Integration
// Description: Obtains the point at the end of 'vector' starting at 'point'.

static Point3D
operator+(const Point3D &origin, const Vector3D &direction) {
	Point3D point{};
	point.x = origin.x + direction.dx;
	point.y = origin.y + direction.dy;
	point.z = origin.z + direction.dz;

	return point;
}

#endif // VECTOR3D_H
