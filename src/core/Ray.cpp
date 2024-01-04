// Assignment 1d - Transparency and Mirror Reflections
// Work by Jacob Secunda

#include "Object.hpp"
#include "Ray.hpp"

Ray::Ray()
	:
	origin(),
	direction()
{
}

Ray::Ray(const Ray& other)
	:
	origin(other.origin),
	direction(other.direction)
{
}

void
Ray::SetDirectionFromIntersection(const Point3D& viewWindowIntersection)
{
	Vector3D rayVector(origin, viewWindowIntersection);
	direction = rayVector.Normalize();
}

[[nodiscard]] Ray
Ray::Invert() const
{
	Ray invertedRay;

	invertedRay.origin.x = this->origin.x + this->direction.dx;
	invertedRay.origin.y = this->origin.y + this->direction.dy;
	invertedRay.origin.z = this->origin.z + this->direction.dz;
	invertedRay.direction *= -1.f;

	return invertedRay;
}

[[nodiscard]] std::optional<float>
Ray::IntersectionTime(const Point3D& pointToIntersect) const
{
	// Intersect Point = origin + t*direction
	float directionToTest;
	float originToTest;
	float intersectToTest;
	if (std::fpclassify(direction.dx) != FP_ZERO) {
		directionToTest = direction.dx;
		originToTest = origin.x;
		intersectToTest = pointToIntersect.x;
	} else if (std::fpclassify(direction.dy) != FP_ZERO) {
		directionToTest = direction.dy;
		originToTest = origin.y;
		intersectToTest = pointToIntersect.y;
	} else if (std::fpclassify(direction.dz) != FP_ZERO) {
		directionToTest = direction.dz;
		originToTest = origin.z;
		intersectToTest = pointToIntersect.z;
	} else {
		return {};
	}
	float time = (intersectToTest - originToTest) / directionToTest;

	return time;
}

#if 0
[[nodiscard]] std::vector<Ray::IntersectionInfo>
Ray::IntersectWithObjects(const std::vector<SharedObject>& objects) const
{
	std::vector<Ray::IntersectionInfo> intersections;

	for (const auto& object : objects)
	{
		float intersectionTime;
		std::optional<Point3D> maybeIntersection = object->IntersectWith(this, &intersectionTime);
		if (!maybeIntersection)
			continue;

		intersections.emplace_back(intersectionTime, *maybeIntersection, object);
	}

	const auto sortFunction = [](const IntersectionInfo& a, const IntersectionInfo& b) -> bool {
		return std::get<0>(a) < std::get<0>(b);
	};
	std::sort(intersections.begin(), intersections.end(), sortFunction);

	return intersections;
}
#endif
