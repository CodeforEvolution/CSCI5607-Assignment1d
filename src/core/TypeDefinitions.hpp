// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda
#ifndef ASSIGNMENT1B_LIGHTANDSHADOW_TYPEDEFINITIONS_HPP
#define ASSIGNMENT1B_LIGHTANDSHADOW_TYPEDEFINITIONS_HPP

#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>
#include <optional>
#include <memory>

/** Helpers **/

#define MAKE_SHARED_NAME(TYPE) \
    typedef std::shared_ptr<TYPE> Shared##TYPE


/** Point 2D **/

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


/** Point 3D **/

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


/** VertNormTextIndex */

// Contains an index to a vertex and possibly an index to a vertex normal.
// Vertex index, along with possibly a vertex normal index, and possibly a texture index...few!
//typedef std::tuple<std::size_t, std::optional<std::size_t>, std::optional<std::size_t>> VertNormTextIndex;
struct VertNormTextIndex {
	std::size_t vertexIndex;
	std::optional<std::size_t> vertexNormalIndex;
	std::optional<std::size_t> textureCoordIndex;

public:
	VertNormTextIndex()
		:
		vertexIndex(0),
		vertexNormalIndex(),
		textureCoordIndex()
	{
	}
};


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

static std::ostream&
operator<<(std::ostream& out, const Point3D& point)
{
	out << "(x: " << std::to_string(point.x) << ", y: " << std::to_string(point.y) << ", z: " << std::to_string(point.z) << ")";
	return out;
}


/** Vector3D **/

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
	Vector3D(const Point3D& start, const Point3D& end)
	{
		dx = end.x - start.x;
		dy = end.y - start.y;
		dz = end.z - start.z;
	}

	// Description: Calculates the length of this vector.
	[[nodiscard]] float
	Length() const
	{
		float innerSqrt = powf(dx, 2) + powf(dy, 2) + powf(dz, 2);
		return sqrtf(innerSqrt);
	}

	// Description: Converts this vector into a unit vector.
	// 	- This is a vector with length 1, though with the same direction.
	void
	NormalizeSelf()
	{
		float vectorLength = Length();
		// We can't divide by zero!!!
		if (std::fpclassify(vectorLength) == FP_ZERO) {
			std::cerr << "Attempted to divide by zero! Exiting..." << std::endl;
			exit(EXIT_FAILURE);
		}

		dx /= vectorLength;
		dy /= vectorLength;
		dz /= vectorLength;
	}

	// Description: Calculates the unit vector of this vector and returns it.
	// 	- This is a vector with length 1, though with the same direction.
	[[nodiscard]] Vector3D
	Normalize() const
	{
		Vector3D norm = *this;
		norm.NormalizeSelf();
		return norm;
	}

	// Description: Obtain angle between directions of this vector and 'v'.
	// 	- Note: Vectors are Orthogonal when Dot Product == 0.
	[[nodiscard]] float
	DotProduct(const Vector3D& v) const
	{
		return (dx * v.dx) + (dy * v.dy) + (dz * v.dz);
	}
    
    // Description: Finds the vector normal/perpendicular to this Vector 3D and 'v'.
    [[nodiscard]] Vector3D
    CrossProduct(const Vector3D& v) const
    {
        Vector3D normal{};

        normal.dx = (this->dy * v.dz) - (this->dz * v.dy);
        normal.dy = -1.f * ((this->dx * v.dz) - (this->dz * v.dx));
        normal.dz = (this->dx * v.dy) - (this->dy * v.dx);

        return normal;
    }

	// Description: Add vectors together to create a vector connecting this vector and 'vector' together.
	constexpr Vector3D&
	operator+=(const Vector3D& vector)
	{
		dx += vector.dx;
		dy += vector.dy;
		dz += vector.dz;

		return *this;
	}

	// Description: Lengthen/shorten this vector by 'scalar' amount.
	constexpr Vector3D&
	operator*=(const float& scalar)
	{
		dx *= scalar;
		dy *= scalar;
		dz *= scalar;

		return *this;
	}

	// Description: Shorten/lengthen this vector by 'scalar' amount.
	constexpr Vector3D&
	operator/=(const float& scalar)
	{
		dx /= scalar;
		dy /= scalar;
		dz /= scalar;

		return *this;
	}

	auto operator<=>(const Vector3D& other) const = default;
};

MAKE_SHARED_NAME(Vector3D);

static Vector3D
operator*(const Vector3D& vector, const float& scalar)
{
    Vector3D scaledVector = vector;
    scaledVector *= scalar;
    return scaledVector;
}

static Vector3D
operator-(const Vector3D& vectorA, const Vector3D& vectorB)
{
	Vector3D vectorC = vectorA;
	vectorC.dx -= vectorB.dx;
	vectorC.dy -= vectorB.dy;
	vectorC.dz -= vectorB.dz;
	return vectorC;
}

static std::ostream&
operator<<(std::ostream& out, const Vector3D& vector)
{
	out << "(dx: " << std::to_string(vector.dx) << ", dy: " << std::to_string(vector.dy) << ", dz: " << std::to_string(vector.dz) << ")";
	return out;
}


/** Ray **/

struct Ray {
	Point3D origin;
	Vector3D direction;

public:
	Ray()
		:
		origin(),
		direction()
	{
	}

	void SetDirectionVector(const Point3D& viewWindowIntersection)
	{
		Vector3D rayVector(origin, viewWindowIntersection);
		direction = rayVector.Normalize();
	}

	[[nodiscard]] Ray
	Invert() const
	{
		Ray invertedRay;

		invertedRay.origin.x = this->origin.x + this->direction.dx;
		invertedRay.origin.y = this->origin.y + this->direction.dy;
		invertedRay.origin.z = this->origin.z + this->direction.dz;
		invertedRay.direction = Vector3D(invertedRay.origin, this->origin);

		return invertedRay;
	}

	[[nodiscard]] std::optional<float>
	IntersectionTime(const Point3D& pointToIntersect) const
	{
		// Intersect Point = origin + t*direction
		float directionToTest = 0.0;
		float originToTest = 0.0f;
		float intersectToTest = 0.0f;
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
};


/** Size **/

struct Size {
	uint32_t width;
	uint32_t height;

public:
	Size()
		:
		width(0),
		height(0)
	{
	}

	Size(uint32_t width, uint32_t height)
	{
		this->width = width;
		this->height = height;
	}
};

static std::ostream&
operator<<(std::ostream& out, const Size& size)
{
	out << "(width: " << size.width << ", height: " << size.height << ")";
	return out;
}


/** ColorRGB **/

struct ColorRGB {
	uint8_t red;
	uint8_t green;
	uint8_t blue;

public:
	enum struct Component {
		RED,
		GREEN,
		BLUE
	};

	ColorRGB()
		:
		red(0),
		green(0),
		blue(0)
	{
	}

	// Description: Works on integer scale of 0 to 255
	ColorRGB(uint8_t red, uint8_t green, uint8_t blue)
	{
		this->red = red;
		this->green = green;
		this->blue = blue;
	}

	// Description: Works on decimal scale of 0.0 to 1.0
	ColorRGB(float red, float green, float blue)
	{
		this->red = static_cast<uint8_t>(std::clamp(roundf(red * 255.f), 0.f, 255.f));
		this->green = static_cast<uint8_t>(std::clamp(roundf(green * 255.f), 0.f, 255.f));
		this->blue = static_cast<uint8_t>(std::clamp(roundf(blue * 255.f), 0.f, 255.f));
	}

	[[nodiscard]] float
	ToFloat(Component colorComponent) const
	{
		switch (colorComponent)
		{
			case Component::RED:
				return kUInt8ToFloat * static_cast<float>(red);
			case Component::GREEN:
				return kUInt8ToFloat * static_cast<float>(green);
			case Component::BLUE:
				return kUInt8ToFloat * static_cast<float>(blue);
		}

		// It "shouldn't" be possible to get here!
		__builtin_unreachable();
	}

	constexpr ColorRGB&
	operator+=(const ColorRGB& other)
	{
		const auto clampColor = [](const uint32_t& value, const uint8_t& min, const uint8_t& max) -> uint8_t {
			if (value > max)
				return max;
			else if (value < min)
				return min;
			else
				return value;
		};

		red = clampColor(red + other.red, 0, 255);
		green = clampColor(green + other.green, 0, 255);
		blue = clampColor(blue + other.blue, 0, 255);

		return *this;
	}

	auto operator<=>(const ColorRGB& other) const = default;

private:
	static constexpr float kUInt8ToFloat = 1.f / 255.f;
};

static std::ostream&
operator<<(std::ostream& out, const ColorRGB& color)
{
	out << "(red: " << std::to_string(color.red) << ", green: " << std::to_string(color.green) << ", blue: " << std::to_string(color.blue) << ")";
	return out;
}


/** MaterialProps */

struct MaterialProps {
	ColorRGB intrinsicColor;			// Od
	ColorRGB specularHighlightColor;	// Os
	float diffuseReflectionMagnitude;	// ka
	float matteMagnitude;				// kd
	float shinyMagnitude;				// ks
	float specularHighlightFocus;		// n
	float opacity;						// α
	float refractionIndex;				// η
};

static std::ostream&
operator<<(std::ostream& out, const MaterialProps& materialProps)
{
	out << "Od: " << materialProps.intrinsicColor << ", "
		<< "Os: " << materialProps.specularHighlightColor << ", "
		<< "ka: " << materialProps.diffuseReflectionMagnitude << ", "
		<< "kd: " << materialProps.matteMagnitude << ", "
		<< "ks: " << materialProps.shinyMagnitude << ", "
		<< "n: " << materialProps.specularHighlightFocus;

	return out;
}


/** TextureCoordinate */

struct TextureCoordinate {
    float u;
    float v;

public:
	TextureCoordinate() = default;

	auto operator<=>(const TextureCoordinate& other) const = default;
};

MAKE_SHARED_NAME(TextureCoordinate);

static std::ostream&
operator<<(std::ostream& out, const TextureCoordinate& coordinate)
{
	out << "(u: " << coordinate.u << ", v: " << coordinate.v << ")";
	return out;
}


// Point & Vector Operators

static Point3D
operator-(const Point3D& pointA, const Point3D& pointB)
{
	Point3D point{};
	point.x = pointA.x - pointB.x;
	point.y = pointA.y - pointB.y;
	point.z = pointA.z - pointB.z;
	return point;
}

// Description: Obtains the point at the end of 'vector' starting at 'point'.
static Point3D
operator+(const Point3D& origin, const Vector3D& direction)
{
	Point3D point{};
	point.x = origin.x + direction.dx;
	point.y = origin.y + direction.dy;
	point.z = origin.z + direction.dz;

	return point;
}

static Vector3D
operator+(const Vector3D& vectorA, const Vector3D& vectorB)
{
	Vector3D vectorC{};

	vectorC.dx = vectorA.dx + vectorB.dx;
	vectorC.dy = vectorA.dy + vectorB.dy;
	vectorC.dz = vectorA.dz + vectorB.dz;

	return vectorC;

}

#endif //ASSIGNMENT1B_LIGHTANDSHADOW_TYPEDEFINITIONS_HPP
