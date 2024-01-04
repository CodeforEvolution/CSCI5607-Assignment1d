// Assignment 1d - Transparency and Mirror Reflections
// Work by Jacob Secunda
#ifndef RAY_H
#define RAY_H

#include <map>
#include <tuple>
#include <vector>

#include "Point.hpp"
#include "TypeDefinitions.hpp"
#include "Vector3D.hpp"

class Ray {
public:
	Point3D origin;
	Vector3D direction;

public:
	Ray();
	Ray(const Ray& other);

	virtual ~Ray() = default;

	void SetDirectionFromIntersection(const Point3D& viewWindowIntersection);

	[[nodiscard]] Ray Invert() const;

	[[nodiscard]] std::optional<float> IntersectionTime(const Point3D& pointToIntersect) const;

#if 0
	// Returns a vector containing every object in 'objects' that intersected with this ray.
	// Each object is in a tuple containing the time and point of intersection.
	typedef std::tuple<float, Point3D, SharedObject> IntersectionInfo;
	[[nodiscard]] std::vector<IntersectionInfo> IntersectWithObjects(const std::vector<SharedObject>& objects) const;
#endif
};

#endif // RAY_H
