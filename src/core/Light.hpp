// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda
#ifndef LIGHT_H
#define LIGHT_H

#include "TypeDefinitions.hpp"
#include "Object.hpp"
#include "Ray.hpp"
#include "Vector3D.hpp"

/** Light **/

struct Light {
	ColorRGB color;

	enum LightType : unsigned int {
		DIRECTIONAL_LIGHT = 0,
		POINT_LIGHT = 1
	};

public:
	Light() = delete;

	virtual ~Light() = default;
	Light(const Light& object) = default;
	Light(Light&& object) = default;

	[[nodiscard]] virtual std::optional<Vector3D> CalculateL(std::optional<Point3D> surfacePos) const = 0;
	[[nodiscard]] virtual Ray GenerateShadowRay(const Point3D& startPoint, const Vector3D& surfaceNormal) const = 0;

	// Description: Returns true if 'object' is between the origin of 'shadowRay' and this light,
	// and false otherwise.
	[[nodiscard]] virtual bool IntersectionBeforeLight(const Ray& shadowRay, const SharedObject& object) const = 0;

	virtual void Print(std::ostream& out) const = 0;

	[[nodiscard]] LightType Type() const { return type; }

protected:
	LightType type;

	explicit
	Light(LightType type)
			:
			type(type),
			color()
	{
	}
};

MAKE_SHARED_NAME(Light);

static std::ostream&
operator<<(std::ostream& out, const Light& light)
{
	light.Print(out);
	return out;
}


struct DirectionalLight : Light {
	Vector3D direction;

public:
	DirectionalLight()
			:
			Light(DIRECTIONAL_LIGHT),
			direction()
	{
	}

	[[nodiscard]] std::optional<Vector3D>
	CalculateL(std::optional<Point3D> surfacePos) const override
	{
		Vector3D l = { };

		l = direction;
		l *= -1.f;
		l.NormalizeSelf();

		return l;
	}

	[[nodiscard]] Ray
	GenerateShadowRay(const Point3D& startPoint, const Vector3D& surfaceNormal) const override
	{
		static constexpr float kEpsilon = 0.001f;

		Ray shadowRay;
        // Push start point out a bit from normal!
		shadowRay.origin = startPoint + (surfaceNormal * kEpsilon);
		shadowRay.direction = direction.Normalize();
		shadowRay.direction *= -1.f;

		return shadowRay;
	}

	[[nodiscard]] bool
	IntersectionBeforeLight(const Ray& shadowRay, const SharedObject& object) const override
	{
		// Get point at which the ray intersects the object.
		float intersectionTime = 0.f;
		std::optional<Point3D> maybeIntersection = object->IntersectWith(shadowRay, &intersectionTime);
		if (!maybeIntersection.has_value())
			return false;

		return std::isgreater(intersectionTime, 0.0f);
	}

	void
	Print(std::ostream& out) const override
	{
		out << "Directional Light:\n";
		out << "\tDirection: " << direction << '\n';
		out << "\tColor: " << color << '\n';
	}
};

struct PointLight : Light {
	Point3D position;

public:
	PointLight()
			:
			Light(POINT_LIGHT),
			position()
	{
	}

	[[nodiscard]] std::optional<Vector3D>
	CalculateL(std::optional<Point3D> surfacePos) const override
	{
		Vector3D l = { };

		if (!surfacePos)
			return {};

		l = Vector3D(surfacePos.value(), position);
		l.NormalizeSelf();

		return l;
	}

	[[nodiscard]] Ray
	GenerateShadowRay(const Point3D& startPoint, const Vector3D& surfaceNormal) const override
	{
		Ray shadowRay;

        // Push start point out a bit from normal!
		shadowRay.origin = startPoint + (surfaceNormal * 0.001f);
		shadowRay.direction = Vector3D(shadowRay.origin, position);
		shadowRay.direction.NormalizeSelf();

		return shadowRay;
	}

	[[nodiscard]] bool
	IntersectionBeforeLight(const Ray& shadowRay, const SharedObject& object) const override
	{
		// Check whether the ray intersects the object and what time the intersection occurs at.
		float intersectionTime = 0.f;
		std::optional<Point3D> maybeIntersection = object->IntersectWith(shadowRay, &intersectionTime);
		if (!maybeIntersection.has_value())
			return false;

		return std::isgreater(intersectionTime, 0.0f);
	}

	void
	Print(std::ostream& out) const override
	{
		out << "Point Light:\n";
		out << "\tPosition: " << position << '\n';
		out << "\tColor: " << color << '\n';
	}
};

#endif // LIGHT_H
