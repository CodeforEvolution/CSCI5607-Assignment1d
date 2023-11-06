// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda
#ifndef ASSIGNMENT1B_LIGHTANDSHADOW_OBJECT_HPP
#define ASSIGNMENT1B_LIGHTANDSHADOW_OBJECT_HPP

#include <filesystem>

#include "TypeDefinitions.hpp"
#include "Texture.hpp"
#include "TextureCache.hpp"

/** Object */
struct Object {
public:
	MaterialProps material;
    std::filesystem::path texturePath;

	enum ObjectType {
		OBJ_SPHERE = 0,
		OBJ_CYLINDER,
        OBJ_TRIANGLE
	};

public:
    // Description: Don't construct this! Only construct subclasses like Sphere and Cylinder.
	Object() = delete;

    // Description: Default destructor, copy constructor, and move constructor.
	virtual ~Object() = default;
	Object(const Object& object) = default;
	Object(Object&& object) = default;

	// Description: Returns the type of this Object.
	[[nodiscard]] ObjectType Type() const { return type; }

	// Description: Prints out information about this Object to the stream 'out'.
	virtual void Print(std::ostream& out) const = 0;

	// Description: Returns the point of intersection if there is an intersection between 'ray' and this Object.
    // Otherwise, nothing is returned.
	[[gnu::always_inline]] [[nodiscard]] virtual std::optional<Point3D> IntersectWith(const Ray& ray, float* intersectionTime) const = 0;

	// Description: Calculates the vector N that originates at 'surfacePoint' and is perpendicular to this Object's surface.
	[[nodiscard]] virtual std::optional<Vector3D> SurfaceNormal(const std::optional<Point3D>& surfacePoint) const = 0;

    // Description: Retrieves the intrinsic color at the point 'surfacePoint' on the object's surface/
	// This can
    [[nodiscard]] virtual ColorRGB GetIntrinsicColorAtSurfacePoint(const Point3D& surfacePoint) const = 0;

    // Description: Checks if the ID of this Object is equivalent to the ID of Object 'other'.
	bool operator==(const Object& other) const
    {
        return this->id == other.id;
    }

protected:
	ObjectType type;
    uint32_t id;

    // Description: Constructs an Object of 'type' with its material properties all initialized to 0, and its ID set to
    // a unique id.
	explicit
	Object(ObjectType type)
			:
            material(),
			type(type),
            id(nextID++)
	{
	}

private:
    static uint32_t nextID;
};

MAKE_SHARED_NAME(Object);

// Description: Prints out information about 'object' to the output stream 'out'.
static std::ostream&
operator<<(std::ostream& out, const Object& object)
{
	object.Print(out);
	return out;
}


/** Sphere */

struct Sphere : Object {
	Point3D center;
	float radius;

public:
    // Description: Constructs a sphere with the center at (0.0, 0.0) and a radius of 0.0;
	Sphere()
			:
			Object(OBJ_SPHERE),
			center(),
			radius(0.0f)
	{
	}

    // Description: Prints out information about this Sphere to the output stream 'out'.
	void Print(std::ostream& out) const override
	{
		out << "Sphere:\n";
		out << "\tCenter: " << center << '\n';
		out << "\tRadius: " << radius << '\n';
		out << "\tMaterial Properties: " << material << '\n';
		if (!texturePath.empty())
			out << "\tTexture Path: " << texturePath << '\n';
	}

    // Description: Refer to the Object struct.
	[[nodiscard]] std::optional<Point3D>
	IntersectWith(const Ray& ray, float* intersectionTime) const override
	{
		float b = 2.f * (ray.direction.dx * (ray.origin.x - center.x) + ray.direction.dy * (ray.origin.y - this->center.y) + ray.direction.dz * (ray.origin.z - this->center.z));
		float c = powf(ray.origin.x - this->center.x, 2.f) + powf(ray.origin.y - this->center.y, 2.f) + powf(ray.origin.z - this->center.z, 2.f) - powf(this->radius, 2.f);

		float discriminant = powf(b, 2.f) - (4.f * c);

		// Check if discriminant is negative; This implies no intersection!
		if (std::signbit(discriminant) && std::fpclassify(discriminant) != FP_ZERO) {
			return {};
		}

		float t1 = ((-1.f * b) + sqrtf(discriminant)) / 2.f;
		float t2 = ((-1.f * b) - sqrtf(discriminant)) / 2.f;

		// Check for smallest positive t, which indicates the intersection we care about!
		Vector3D tempVector = ray.direction;
		if (!std::signbit(t1) && std::isless(t1, t2)) {
			// t1 is the viewable point!
			tempVector *= t1;
			if (intersectionTime != nullptr)
				*intersectionTime = t1;
		} else if (!std::signbit(t2)) {
			// t2 is the viewable point!
			tempVector *= t2;
			if (intersectionTime != nullptr)
				*intersectionTime = t2;
		} else {
            return {};
        }

		return ray.origin + tempVector;
	}

    // Description: Refer to the Object struct.
	[[nodiscard]] std::optional<Vector3D>
	SurfaceNormal(const std::optional<Point3D>& surfacePoint) const override
	{
        if (!surfacePoint)
            return {};

		Vector3D vectorN(center, *surfacePoint);
		vectorN /= radius;

		return vectorN;
	}

    // Description: Refer to the Object struct.
    [[nodiscard]] ColorRGB GetIntrinsicColorAtSurfacePoint(const Point3D& surfacePoint) const override
    {
        if (texturePath.empty())
            return material.intrinsicColor;

        Vector3D N = *SurfaceNormal(surfacePoint);
        float phi = std::acos(N.dz);
        float theta = std::atan2(N.dy, N.dx);

        float u = 0.f;
        if (theta >= 0.f)
            u = theta * 0.5f * std::numbers::inv_pi_v<float>;
        else
            u = (theta + (2.f * std::numbers::pi_v<float>)) * 0.5f * std::numbers::inv_pi_v<float>;

        float v = phi * std::numbers::inv_pi_v<float>;

        SharedTexture objectTexture;
        if (!TextureCache::Instance().GetTexture(texturePath, objectTexture))
            return {};

        ColorRGB pixelOut;
        if (!objectTexture->GetPixelWithTextureCoordinate(u, v, pixelOut))
            return {};

        return pixelOut;
    }

    // Description: Checks if the Sphere 'other' is equivalent to this Sphere.
	bool operator==(const Sphere& other) const = default;
};


/** Cylinder */

struct Cylinder : Object {
	Point3D center;
	float radius;
	Vector3D direction;
	float length;

public:
    // Description: Constructs a Cylinder with all properties initialized to 0.
	Cylinder()
			:
			Object(OBJ_CYLINDER),
			center(),
			radius(0.0f),
			direction(),
			length(0.0f)
	{
	}

    // Description: Prints information about this Cylinder to the output stream 'out'.
	void
	Print(std::ostream& out) const override
	{
		out << "Cylinder:\n";
		out << "\tCenter: " << center << '\n';
		out << "\tRadius: " << radius << '\n';
		out << "\tDirection: " << direction << '\n';
		out << "\tLength: " << length << '\n';
	}

    // Description: Refer to the Object struct.
	[[nodiscard]] std::optional<Point3D>
	IntersectWith(const Ray& ray, float* intersectionTime) const override
	{
		// Unimplemented
		return {};
	}

    // Description: Refer to the Object struct.
	[[nodiscard]] std::optional<Vector3D>
	SurfaceNormal(const std::optional<Point3D>& surfacePoint) const override
	{
		// Unimplemented
		return {};
	}

    // Description: Refer to the Object struct.
    [[nodiscard]] ColorRGB
    GetIntrinsicColorAtSurfacePoint(const Point3D& surfacePoint) const override
    {
        // Unimplemented
        return {};
    }


    // Description: Checks if the Cylinder 'other' is equivalent to this Cylinder.
	bool operator==(const Cylinder& other) const = default;
};


/** Triangle */

struct Triangle : Object {
    Point3D vertexA;
    Point3D vertexB;
    Point3D vertexC;

    std::optional<Vector3D> vertexNormalA;
    std::optional<Vector3D> vertexNormalB;
    std::optional<Vector3D> vertexNormalC;

    std::optional<TextureCoordinate> textureCoordinateA;
    std::optional<TextureCoordinate> textureCoordinateB;
    std::optional<TextureCoordinate> textureCoordinateC;

public:
    // Description: Constructs a sphere with the center at (0.0, 0.0) and a radius of 0.0;
    Triangle()
            :
            Object(OBJ_TRIANGLE),
            vertexA(),
            vertexB(),
            vertexC(),
            vertexNormalA(),
            vertexNormalB(),
            vertexNormalC(),
            textureCoordinateA(),
            textureCoordinateB(),
            textureCoordinateC()
    {
    }

    // Description: Prints out information about this Sphere to the output stream 'out'.
    void Print(std::ostream& out) const override
    {
        out << "Triangle:\n";
        out << "\tMaterial: " << material << '\n';
        out << "\tVertex A: " << vertexA << '\n';
        out << "\tVertex B: " << vertexB << '\n';
        out << "\tVertex C: " << vertexC << '\n';
        if (SmoothShaded())
        {
            out << "\tVertex Normal A: " << *vertexNormalA << '\n';
            out << "\tVertex Normal B: " << *vertexNormalB << '\n';
            out << "\tVertex Normal C: " << *vertexNormalC << '\n';
        }
		if (Textured())
		{
			out << "\tTexture Path: " << texturePath << '\n';
			out << "\tTexture Coordinate A: " << *textureCoordinateA << "\n";
			out << "\tTexture Coordinate B: " << *textureCoordinateB << "\n";
			out << "\tTexture Coordinate C: " << *textureCoordinateC << "\n";
		}
    }

    // Description: Refer to the Object struct.
    [[nodiscard]] std::optional<Point3D>
    IntersectWith(const Ray& ray, float* intersectionTime) const override
    {
        auto e1 = Vector3D(vertexA, vertexB);
        auto e2 = Vector3D(vertexA, vertexC);
        auto n = e1.CrossProduct(e2);

        // Solve for D in Plane Equation: Ax + By + Cz + D = 0
        // A = nx, B = ny, C = nz, x = p0x, y = p0y, z = p0z
        float d = (n.dx * vertexA.x) + (n.dy * vertexA.y) + (n.dz * vertexA.z);
        d *= -1.f;

        // Solve for t (intersection)
        // Plane equation: Ax + By + Cz + D = 0
        // x = x0 + t×xd
        // y = y0 + t×yd
        // z = z0 + t×zd
        // t = –(A×x0 + B×y0 + C×z0 + D)/(A×xd + B×yd + C×zd)
        float tDenominator = (n.dx * ray.direction.dx) + (n.dy * ray.direction.dy) + (n.dz * ray.direction.dz);
        if (std::fpclassify(tDenominator) == FP_ZERO) {
            // The ray is parallel to the plane!
            return {};
        }

        float tNumerator = (n.dx * ray.origin.x) + (n.dy * ray.origin.y) + (n.dz * ray.origin.z) + d;
        float t = tNumerator / tDenominator;
        t *= -1.f;
        if (std::islessequal(t, 0.0f)) {
            // Intersection behind ray origin!
            return {};
        }

        Point3D intersectionPoint{};
        intersectionPoint.x = ray.origin.x + (ray.direction.dx * t);
        intersectionPoint.y = ray.origin.y + (ray.direction.dy * t);
        intersectionPoint.z = ray.origin.z + (ray.direction.dz * t);

        if (!PointInTriangle(intersectionPoint))
            return {};

		if (intersectionTime != nullptr)
			*intersectionTime = t;

        return intersectionPoint;
    }

    // Description: Refer to the Object struct.
    [[nodiscard]] std::optional<Vector3D>
    SurfaceNormal(const std::optional<Point3D>& surfacePoint) const override
    {
        return SmoothShaded() ? SmoothShadeSurfaceNormal(*surfacePoint) : FlatShadeSurfaceNormal();
    }

    // Description: Refer to the Object struct.`
    [[nodiscard]] ColorRGB
    GetIntrinsicColorAtSurfacePoint(const Point3D& surfacePoint) const override
    {
        if (texturePath.empty() || !Textured())
			return material.intrinsicColor;

		float alpha = 0.f;
		float beta = 0.f;
		float gamma = 0.f;
		if (!CalculateBarycentricCoordinates(surfacePoint, alpha, beta, gamma))
			return {};

		float u = (alpha * textureCoordinateA->u) + (beta * textureCoordinateB->u) + (gamma * textureCoordinateC->u);
		float v = (alpha * textureCoordinateA->v) + (beta * textureCoordinateB->v) + (gamma * textureCoordinateC->v);

		SharedTexture objectTexture;
		if (!TextureCache::Instance().GetTexture(texturePath, objectTexture))
			return {};

		ColorRGB pixelOut;
		if (!objectTexture->GetPixelWithTextureCoordinate(u, v, pixelOut))
			return {};

		return pixelOut;
    }

    // Description: Checks if the Triangle 'other' is equivalent to this Triangle.
    bool operator==(const Triangle& other) const = default;

private:
    bool
    CalculateBarycentricCoordinates(const Point3D& surfacePoint, float& alpha, float& beta, float& gamma) const
    {
        // B * d11 + y * d12 = d1p
        // B * d12 + y * d11 = d2p
        Vector3D e1 = Vector3D(vertexA, vertexB);
        Vector3D e2 = Vector3D(vertexA, vertexC);
        Vector3D ep = Vector3D(vertexA, surfacePoint);

        float d11 = e1.DotProduct(e1);
        float d12 = e1.DotProduct(e2);
        float d22 = e2.DotProduct(e2);

        float determinant = (d11 * d22) - (d12 * d12);
        if (std::fpclassify(determinant) == FP_ZERO)
            return false;

        float d1p = e1.DotProduct(ep);
        float d2p = e2.DotProduct(ep);

        beta = ((d22 * d1p) - (d12 * d2p)) / determinant;
        gamma = ((d11 * d2p) - (d12 * d1p)) / determinant;
        alpha = 1.f - (beta + gamma);

        return true;
    }

    [[nodiscard]] bool
    PointInTriangle(const Point3D& pointToCheck) const
    {
        float alpha = 0.f;
        float beta = 0.f;
        float gamma = 0.f;
        if (!CalculateBarycentricCoordinates(pointToCheck, alpha, beta, gamma))
            return false;

        return (alpha >= 0.f && alpha <= 1.f && beta >= 0.f && beta <= 1.f && gamma >= 0.f && gamma <= 1.f);
    }

    [[nodiscard]] bool
    SmoothShaded() const
    {
        return vertexNormalA && vertexNormalB && vertexNormalC;
    }
    
    [[nodiscard]] Vector3D
    SmoothShadeSurfaceNormal(const Point3D& surfacePoint) const
    {
        float alpha = 0.f;
        float beta = 0.f;
        float gamma = 0.f;
        if (!CalculateBarycentricCoordinates(surfacePoint, alpha, beta, gamma))
            return {};

        Vector3D surfaceNormal = (*vertexNormalA * alpha) + (*vertexNormalB * beta) + (*vertexNormalC * gamma);
        surfaceNormal.NormalizeSelf();

        return surfaceNormal;
    }
    
    [[nodiscard]] Vector3D
    FlatShadeSurfaceNormal() const
    {
        auto e1 = Vector3D(vertexA, vertexB);
        auto e2 = Vector3D(vertexA, vertexC);

        return e1.CrossProduct(e2).Normalize();
    }

	[[nodiscard]] bool
	Textured() const
	{
		return textureCoordinateA && textureCoordinateB && textureCoordinateC;
	}
};



#endif //ASSIGNMENT1B_LIGHTANDSHADOW_OBJECT_HPP
