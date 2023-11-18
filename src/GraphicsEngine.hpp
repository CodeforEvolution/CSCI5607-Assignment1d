// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda
#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <map>
#include <unordered_map>
#include <memory>

#include "core/TypeDefinitions.hpp"
#include "core/Light.hpp"
#include "core/Object.hpp"
#include "core/Vector3D.hpp"

/** The Scene Definition **/

struct SceneDefinition {
	// Globals
	Point3D eyePosition;
	Vector3D viewDirection;
	Vector3D upDirection;
	float fovVertical;
	Size imagePixelSize;
	ColorRGB backgroundColor;
	float backgroundRefractionIndex;

	float frustumHeight;

	// Objects
	std::vector<SharedObject> objectList;

	// Lights
	std::vector<SharedLight> lightList;

    // Vertexes
    std::vector<SharedPoint3D> vertexList;

    // Vertex Normals
    std::vector<SharedVector3D> vertexNormalList;

    // Texture Coordinates
    std::vector<SharedTextureCoordinate> textureCoordinateList;
};

static std::ostream&
operator<<(std::ostream& out, const SceneDefinition& definition)
{
	out << "Eye Position: " << definition.eyePosition << '\n';
	out << "View Direction: " << definition.viewDirection << '\n';
	out << "Up Direction: " << definition.upDirection << '\n';
	out << "Vertical Field Of View: " << definition.fovVertical << " degrees" << '\n';
	out << "Image Size in Pixels: " << definition.imagePixelSize << '\n';
	out << "Background Color: " << definition.backgroundColor << '\n';
	out << "Background Refraction Index: " << definition.backgroundRefractionIndex << '\n';
	out << "Objects: " << definition.objectList.size() << '\n';
	for (const auto& object : definition.objectList) {
		object->Print(out);
	}
	out << "Lights: " << definition.lightList.size() << '\n';
	for (const auto& light : definition.lightList) {
		light->Print(out);
	}

	return out;
}


// Visual Types

struct CoordSys {
	CoordSys() = default;

	CoordSys(const Vector3D& viewDir, const Vector3D& upDir)
		:
		u(),
		v(),
		w()
	{
		Vector3D normViewDir = viewDir.Normalize();
		Vector3D normUpDir = upDir.Normalize();

		// Vector u (Horizontal Image Direction): Orthogonal to view dir and up dir
		u = normViewDir.CrossProduct(normUpDir);
		u.NormalizeSelf();

		// Vector v (Vertical Image Direction):
		v = u.CrossProduct(normViewDir);
		v.NormalizeSelf();

		// Vector w
		w = normViewDir;
		w *= -1.f;
		w.NormalizeSelf();
	}

	Vector3D u;
	Vector3D v;
	Vector3D w;
};

struct ViewingWindow {
	ViewingWindow() = default;

	// Description: Creates a viewing window that is used to map a section of a 3d space onto a 2d plane.
	ViewingWindow(const CoordSys& coordSys, const Vector3D& viewDirection, const Point3D& eyePosition, const float& fovVertical, const Size& imagePixelSize)
		:
		upperLeft(),
		upperRight(),
		lowerLeft(),
		lowerRight()
	{
		Vector3D n = viewDirection.Normalize();

		const float distance = 1.f;
		n *= distance;

		Point3D viewWindowCenter = eyePosition + n;

		float fovVertRadians = fovVertical * (std::numbers::pi_v<float> / 180.f);
		float h = 2.f * distance * tanf(0.5f * fovVertRadians);

		float aspectRatio = (float)imagePixelSize.width / (float)imagePixelSize.height;

		float w = h * aspectRatio;

		Vector3D widthFactor = coordSys.u;
		widthFactor *= w / 2.f;

		Vector3D heightFactor = coordSys.v;
		heightFactor *= h / 2.f;

		widthFactor *= -1.f;
		upperLeft = viewWindowCenter + widthFactor;
		upperLeft = upperLeft + heightFactor;

		widthFactor *= -1.f;
		upperRight = viewWindowCenter + widthFactor;
		upperRight = upperRight + heightFactor;

		widthFactor *= -1.f;
		lowerLeft = viewWindowCenter + widthFactor;
		heightFactor *= -1.f;
		lowerLeft = lowerLeft + heightFactor;

		widthFactor *= -1.f;
		lowerRight = viewWindowCenter + widthFactor;
		lowerRight = viewWindowCenter + heightFactor;
	}

    // Description: Gets the increment by which each image pixel relative to the X axis
    // offsets into the ViewingWindow.
	[[nodiscard]] Vector3D
	GetHorizontalOffset(const uint32_t& imageWidth) const
	{
		Vector3D horizontalOffset(upperLeft, upperRight);
		horizontalOffset /= static_cast<float>(imageWidth - 1);

		return horizontalOffset;
	}

    // Description: Gets the increment by which each image pixel relative to the Y axis
    // offsets into the ViewingWindow.
	[[nodiscard]] Vector3D
	GetVerticalOffset(const uint32_t& imageHeight) const
	{
		Vector3D verticalOffset(upperLeft, lowerLeft);
		verticalOffset /= static_cast<float>(imageHeight - 1);

		return verticalOffset;
	}

    // Description: Maps 'imageCoord', a pixel coordinate in the final image of size 'imageSize', to
    // a point on the ViewingWindow.
    // Returns: The point on the ViewingWindow corresponding to the image pixel position.
	[[nodiscard]] Point3D
	MapImagePixelToPoint(const Size& imageSize, const Point2D<uint32_t>& imageCoord) const
	{
		Vector3D horizOffset = GetHorizontalOffset(imageSize.width);
		Vector3D vertOffset = GetVerticalOffset(imageSize.height);

		horizOffset *= static_cast<float>(imageCoord.x);
		vertOffset *= static_cast<float>(imageCoord.y);

		Point3D viewWindowPoint = upperLeft + horizOffset;
		viewWindowPoint = viewWindowPoint + vertOffset;

		return viewWindowPoint;
	}

	Point3D upperLeft;
	Point3D upperRight;
	Point3D lowerLeft;
	Point3D lowerRight;
};


class GraphicsEngine {
public:
	GraphicsEngine() = delete;

    // Check GraphicsEngine.cpp for information!
	static ColorRGB TraceWithRay(const Ray& ray, const SceneDefinition& scene, uint32_t depth = 0);
	static float CalculateShadow(const Point3D& startPoint, const SharedLight& lightToCheck, const std::vector<SharedObject>& objects, const SharedObject& objectHit);
	static ColorRGB ShadeWithRay(const Ray& ray, const Point3D& intersectionPoint, const SceneDefinition& scene, const SharedObject& objectHit, uint32_t depth = 0);
};

#endif // GRAPHICS_ENGINE_H