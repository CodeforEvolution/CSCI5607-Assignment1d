// Assignment 1d - Transparency and Mirror Reflections
// Work by Jacob Secunda

#include "Object.hpp"

#include "../TextureCache.hpp"

uint32_t Object::sNextID = 0;

// Description: Refer to the Object struct.
[[nodiscard]] ColorRGB
Sphere::GetIntrinsicColorAtSurfacePoint(const Point3D& surfacePoint) const
{
	if (texturePath.empty())
		return material.intrinsicColor;

	Vector3D N = *SurfaceNormal(surfacePoint);
	float phi = std::acos(N.dz);
	float theta = std::atan2(N.dy, N.dx);

	float u;
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

// Description: Refer to the Object struct.`
[[nodiscard]] ColorRGB
Triangle::GetIntrinsicColorAtSurfacePoint(const Point3D& surfacePoint) const
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