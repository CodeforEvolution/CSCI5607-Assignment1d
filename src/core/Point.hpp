// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda
#ifndef POINT_H
#define POINT_H

#include "TypeDefinitions.hpp"

/** Point 2D */

template<typename T>
struct Point2D {
	Point2D() = default;
	~Point2D() = default;

	Point2D(T x, T y)
	{
		this->x = x;
		this->y = y;
	}

	T x;
	T y;
};


/** Point 3D */

struct Point3D {
	float x;
	float y;
	float z;

public:
	Point3D() = default;
	~Point3D() = default;

	Point3D(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	constexpr Point3D&
	operator*=(const float& scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;

		return *this;
	}

	constexpr Point3D&
	operator+=(const Point3D& point)
	{
		x += point.x;
		y += point.y;
		z += point.z;

		return *this;
	}

	// This is a three-way comparison operator which handles <=>, ==, !=, <, >, <=, and >=.
	auto operator<=>(const Point3D& other) const = default;
};

static Point3D
operator+(const Point3D& point, float offset)
{
	Point3D translatedPoint = point;
	translatedPoint.x += offset;
	translatedPoint.y += offset;
	translatedPoint.z += offset;

	return translatedPoint;
}

MAKE_SHARED_NAME(Point3D);


static Point3D
operator*(const Point3D& point, const float& scalar)
{
	Point3D translatedPoint = point;
	translatedPoint *= scalar;

	return translatedPoint;
}

static Point3D
operator+(const Point3D& pointA, const Point3D& pointB)
{
	Point3D offsetPoint = pointA;
	offsetPoint += pointB;

	return offsetPoint;
}

static Point3D
operator-(const Point3D& pointA, const Point3D& pointB)
{
	Point3D point{};
	point.x = pointA.x - pointB.x;
	point.y = pointA.y - pointB.y;
	point.z = pointA.z - pointB.z;
	return point;
}

static std::ostream&
operator<<(std::ostream& out, const Point3D& point)
{
	out << "(x: " << std::to_string(point.x) << ", y: " << std::to_string(point.y) << ", z: " << std::to_string(point.z) << ")";
	return out;
}

#endif // POINT_H
