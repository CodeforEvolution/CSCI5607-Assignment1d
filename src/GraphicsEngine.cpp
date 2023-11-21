// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda

#include "GraphicsEngine.hpp"

#include <cmath>
#include <iostream>

/* Ray Utilities */

// Description:
// Finds the color of the pixel corresponding to the viewing window point pointed to by 'ray', with 'ray'
// originating at the camera/eye position. 'scene' defines all the objects, lights, their properties, camera information,
// and to ensure the returned pixel color accounts for shadows, material properties, light colors, and more.
ColorRGB
GraphicsEngine::TraceWithRay(const Ray &ray, const SceneDefinition &scene, float previousRefractionIndex, uint32_t depth)
{
	float closestIntersectionTime = std::numeric_limits<float>::max();
	int64_t closestObjectIndex = -1;
	Point3D closestIntersectPoint{};
	for (std::size_t index = 0; index < scene.objectList.size(); index++) {
		SharedObject currentObject = scene.objectList[index];

		// Do the intersection test with the ray and the current object!
		float intersectionTime = 0.f;
		std::optional<Point3D> currentIntersect = currentObject->IntersectWith(ray, &intersectionTime);
		if (!currentIntersect.has_value())
			continue;

		if (std::isless(intersectionTime, closestIntersectionTime)) {
			closestIntersectionTime = intersectionTime;
			closestObjectIndex = static_cast<int64_t>(index);
			closestIntersectPoint = currentIntersect.value();
		}
	}

	// The ray didn't hit any objects, oh well...
	if (closestObjectIndex == -1)
		return scene.backgroundColor;

	return ShadeWithRay(ray, closestIntersectPoint, scene, scene.objectList[closestObjectIndex], previousRefractionIndex, depth);
}


// Description: Calculates the amount of shadow at 'startPoint' caused by 'objects' blocking light rays sourced from 'lightToCheck'
// on their way to 'targetObject'.
// Returns: 0.0 if 'startPoint' is in shadow, and 1.0 otherwise.
float
GraphicsEngine::CalculateShadow(const Point3D& startPoint, const SharedLight& lightToCheck, const std::vector<SharedObject>& objects, const SharedObject& objectHit)
{
	const Ray shadowRay = lightToCheck->GenerateShadowRay(startPoint, *objectHit->SurfaceNormal(startPoint));
    const std::optional<Vector3D> vectorL = lightToCheck->CalculateL(startPoint);

	float shadowAmount = 1.f;
	for (const auto& object : objects)
	{
        std::optional<Vector3D> vectorN = object->SurfaceNormal(startPoint);

		if (*object == *objectHit
            && vectorN.has_value() && vectorL.has_value()
            && std::isless(vectorN->DotProduct(*vectorL), 0.0f))
		{
			shadowAmount *= (1.f - object->material.opacity);
			continue;
		}

		if (lightToCheck->IntersectionBeforeLight(shadowRay, object)) {
			shadowAmount *= (1.f - object->material.opacity);
			continue;
		}
	}

	return shadowAmount;
}

// Description: Calculates the pixel represented by a viewing window point that is pointed to by 'ray'.
// The calculation involved performing Blinn-Phong Illumination equation after calculating all necessary parameters.
// Parameters:
//  - ray: Originates from the eye/camera position and points to a point on the viewing window; Incoming ray
//  - objectHit: The object closest to the camera/eye position that intersected with 'ray'
//  - intersectionPoint: The point where 'ray' and 'objectHit' intersected.
//  - lights: All the lights in the scene. They will be tested to see if their light reaches 'intersectionPoint'.
//  - objects: All the objects in the scene. They will be tested to see if their presence blocks incoming light and cast a shadow.
ColorRGB
GraphicsEngine::ShadeWithRay(const Ray& ray, const Point3D& intersectionPoint, const SceneDefinition& scene, const SharedObject& objectHit, const float& previousRefractionIndex,  uint32_t depth)
{
	// Blinn-Phong Illumination Equation
	std::optional<Vector3D> maybeN = objectHit->SurfaceNormal(intersectionPoint);
	if (!maybeN) {
		std::cerr << "Error: Couldn't calculate surface normal vector N!" << std::endl;
		exit(EXIT_FAILURE);
	}
	Vector3D& vectorN = maybeN.value();
	vectorN.NormalizeSelf();

	// I - Faces outward, points from intersection point to incoming ray origin
	Vector3D vectorIPrime = Vector3D(ray.origin, intersectionPoint);
	vectorIPrime.NormalizeSelf();
	Vector3D vectorI = vectorIPrime * -1.f;

	// ηi - Refraction Index (Incoming Ray)
	const float& ηi = objectHit->material.refractionIndex;

	// ηt - Refraction Index (Transmitted Ray)
	const float& ηt = previousRefractionIndex;

	if (ηi > ηt)
		vectorN *= -1.f;

	// a - cos(theta_i) = I dot N
	float a = std::max(0.f, vectorI.DotProduct(vectorN));

	// α - Material Opacity
	const float& α = objectHit->material.opacity;

	// F_r - Fresnel Reflectance
	const float Fo = powf((ηi - 1.f) / (ηi + 1.f), 2.f);
	const float Fr = Fo + (1.f - Fo) * powf((1.f - a), 5.f);

	const MaterialProps& objMat = objectHit->material;

	using ColorRGBFloat = std::tuple<float, float, float>;
	const ColorRGBFloat Od = objectHit->GetIntrinsicColorAtSurfacePoint(intersectionPoint).ToFloat();
	const ColorRGBFloat Os = objMat.specularHighlightColor.ToFloat();

	// Sum up each light's contributions

	ColorRGBFloat lightSum = {0.f, 0.f, 0.f};
	for (const auto& light : scene.lightList) {
		// Is the light blocked?
		float shadow = CalculateShadow(intersectionPoint, light, scene.objectList, objectHit);

		Vector3D vectorL = light->CalculateL(intersectionPoint).value();

		Vector3D vectorV = ray.direction;
		vectorV *= -1.f;
		vectorV.NormalizeSelf();

		Vector3D vectorH = vectorL + vectorI;
		vectorH.NormalizeSelf();

		const ColorRGBFloat lightColor = light->color.ToFloat();

		const float nDotL = std::max(0.f, vectorN.DotProduct(vectorL));
		const float nDotH = std::max(0.f, vectorN.DotProduct(vectorH));

		// Red
		std::get<0>(lightSum) += shadow * std::get<0>(lightColor) * (objMat.matteMagnitude * std::get<0>(Od) * nDotL)
			+ (objMat.shinyMagnitude * std::get<0>(Os) * powf(nDotH, objMat.specularHighlightFocus));

		// Green
		std::get<1>(lightSum) += shadow * std::get<1>(lightColor) * (objMat.matteMagnitude * std::get<1>(Od) * nDotL)
			 + (objMat.shinyMagnitude * std::get<1>(Os) * powf(nDotH, objMat.specularHighlightFocus));

		// Blue
		std::get<2>(lightSum) += shadow * std::get<2>(lightColor) * (objMat.matteMagnitude * std::get<2>(Od) * nDotL)
			 + (objMat.shinyMagnitude * std::get<2>(Os) * powf(nDotH, objMat.specularHighlightFocus));
	}


	// Phong Illumination Equation Time!
	ColorRGBFloat illumination;

	// Red
	std::get<0>(illumination) = (objMat.diffuseReflectionMagnitude * std::get<0>(Od))
								+ std::get<0>(lightSum);

	// Green
	std::get<1>(illumination) = (objMat.diffuseReflectionMagnitude * std::get<1>(Od))
								+ std::get<1>(lightSum);


	// Blue
	std::get<2>(illumination) = (objMat.diffuseReflectionMagnitude * std::get<2>(Od))
								+ std::get<2>(lightSum);

	// Clamp up!
	std::clamp(std::get<0>(illumination), 0.0f, 1.0f);
	std::clamp(std::get<1>(illumination), 0.0f, 1.0f);
	std::clamp(std::get<2>(illumination), 0.0f, 1.0f);
	auto illuminationColor = ColorRGB(std::get<0>(illumination), std::get<1>(illumination), std::get<2>(illumination));

	// Refraction
	ColorRGB refractionColor(0.f, 0.f, 0.f);
	if (α < 1.f && depth > 0) {
		// T - Refraction transmitted ray
		Vector3D vectorT = (vectorN * -1.f) * sqrtf(1.f - (powf(ηi / ηt, 2.f) * (1.f - powf(a, 2.f)))) + ((vectorN * a) - vectorI) * (ηi / ηt);
		vectorT.NormalizeSelf();

		Ray refractedRay;
		refractedRay.origin = intersectionPoint + (vectorN * 0.001f);
		refractedRay.direction = vectorT;

		refractionColor = TraceWithRay(refractedRay, scene, ηt, depth - 1);

		ColorRGBFloat calcRefractColor = refractionColor.ToFloat();
		std::get<0>(calcRefractColor) *= (1.f - Fr) * (1.f - α); // Red
		std::get<1>(calcRefractColor) *= (1.f - Fr) * (1.f - α); // Green
		std::get<2>(calcRefractColor) *= (1.f - Fr) * (1.f - α); // Blue

		// Final Refraction Color
		refractionColor = ColorRGB(std::get<0>(calcRefractColor), std::get<1>(calcRefractColor), std::get<2>(calcRefractColor));
	}

	illuminationColor += refractionColor;

	// Reflection
	ColorRGB reflectionColor(0.f, 0.f, 0.f);
	if (depth > 0) {
		// R - Reflected Ray Direction
		Vector3D vectorR = (vectorN * a * 2.f) - vectorI;
		vectorR.NormalizeSelf();

		Ray reflectedRay;
		reflectedRay.origin = intersectionPoint + (vectorN * 0.001f);
		reflectedRay.direction = vectorR;

		reflectionColor = TraceWithRay(reflectedRay, scene, ηt, depth - 1);

		ColorRGBFloat calcReflectColor = reflectionColor.ToFloat();
		std::get<0>(calcReflectColor) *= Fr; // Red
		std::get<1>(calcReflectColor) *= Fr; // Green
		std::get<2>(calcReflectColor) *= Fr; // Blue

		// Final Reflection Color
		reflectionColor = ColorRGB(std::get<0>(calcReflectColor), std::get<1>(calcReflectColor), std::get<2>(calcReflectColor));
	}

	illuminationColor += reflectionColor;

	return illuminationColor;
}
