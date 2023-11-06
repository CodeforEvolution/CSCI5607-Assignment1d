// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda

#include "GraphicsEngine.hpp"

#include <cmath>
#include <iostream>

#include "core/TextureCache.hpp"

/* Ray Utilities */

// Description:
// Finds the color of the pixel corresponding to the viewing window point pointed to by 'ray', with 'ray'
// originating at the camera/eye position. 'scene' defines all the objects, lights, their properties, camera information,
// and to ensure the returned pixel color accounts for shadows, material properties, light colors, and more.
ColorRGB
GraphicsEngine::TraceWithRay(const Ray& ray, const SceneDefinition& scene)
{
	float closestDistance = std::numeric_limits<float>::max();
	int64_t closestObjectIndex = -1;
	Point3D closestIntersectPoint{};
	for (std::size_t index = 0; index < scene.objectList.size(); index++) {
		SharedObject currentObject = scene.objectList[index];

		// Do the intersection test with the ray and the current object!
		std::optional<Point3D> currentIntersect = currentObject->IntersectWith(ray, nullptr);

		if (!currentIntersect.has_value())
			continue;

		Vector3D currentVec(scene.eyePosition, currentIntersect.value());
		float currentDistance = currentVec.Length();

		if (std::isless(currentDistance, closestDistance)) {
			closestDistance = currentDistance;
			closestObjectIndex = static_cast<int64_t>(index);
			closestIntersectPoint = currentIntersect.value();
		}
	}

	// The ray didn't hit any objects, oh well...
	if (closestObjectIndex == -1)
		return scene.backgroundColor;

	return ShadeWithRay(ray, scene.objectList[closestObjectIndex], closestIntersectPoint, scene.lightList, scene.objectList);
}


// Description: Calculates the amount of shadow at 'startPoint' caused by 'objects' blocking light rays sourced from 'lightToCheck'
// on their way to 'targetObject'.
// Returns: 0.0 if 'startPoint' is in shadow, and 1.0 otherwise.
float
GraphicsEngine::CalculateShadow(const Point3D& startPoint, const SharedLight& lightToCheck, const SharedObject& targetObject, const std::vector<SharedObject>& objects)
{
	const Ray shadowRay = lightToCheck->GenerateShadowRay(startPoint, *targetObject->SurfaceNormal(startPoint));
    const std::optional<Vector3D> vectorL = lightToCheck->CalculateL(startPoint);
	for (const auto& object : objects)
	{
        std::optional<Vector3D> vectorN = object->SurfaceNormal(startPoint);

		if (*object == *targetObject
            && vectorN.has_value() && vectorL.has_value()
            && std::isless(vectorN->DotProduct(*vectorL), 0.0f))
			return 0.0f;

		if (lightToCheck->IntersectionBeforeLight(shadowRay, object)) {
			return 0.0f;
		}
	}

	return 1.f;
}

// Turns the image black and white when true. Areas of black are shadows cast on the object, while
// white shows light could reach the object unobstructed.
static constexpr bool shadowMode = false;

// Description: Calculates the pixel represented by a viewing window point that is pointed to by 'ray'.
// The calculation involved performing Blinn-Phong Illumination equation after calculating all necessary parameters.
// Parameters:
//  - ray: Originates from the eye/camera position and points to a point on the viewing window
//  - objectHit: The object closest to the camera/eye position that intersected with 'ray'
//  - intersectionPoint: The point where 'ray' and 'objectHit' intersected.
//  - lights: All the lights in the scene. They will be tested to see if their light reaches 'intersectionPoint'.
//  - objects: All the objects in the scene. They will be tested to see if their presence blocks incoming light and cast a shadow.
ColorRGB
GraphicsEngine::ShadeWithRay(const Ray& ray, const SharedObject& objectHit, const Point3D& intersectionPoint, const std::vector<SharedLight>& lights, const std::vector<SharedObject>& objects)
{
	// Blinn-Phong Illumination Equation
	std::optional<Vector3D> maybeN = objectHit->SurfaceNormal(intersectionPoint);
	if (!maybeN) {
		std::cerr << "Error: Couldn't calculate surface normal vector N!" << std::endl;
		exit(EXIT_FAILURE);
	}
	Vector3D& vectorN = maybeN.value();

	const MaterialProps& objMat = objectHit->material;
	const ColorRGB intrinsicColor = objectHit->GetIntrinsicColorAtSurfacePoint(intersectionPoint);
	auto phongIllumination = [&](const ColorRGB::Component colorComponent)
    {
        const float Od = intrinsicColor.ToFloat(colorComponent);
        const float Os = objMat.specularHighlightColor.ToFloat(colorComponent);

        float lightSum = 0.0f;
		for (const auto& light : lights)
		{
			// Is the light blocked?
			float shadow = CalculateShadow(intersectionPoint, light, objectHit, objects);

			Vector3D vectorL = light->CalculateL(intersectionPoint).value();

			Vector3D vectorV = ray.direction;
			vectorV *= -1.f;
			vectorV.NormalizeSelf();

			Vector3D vectorH = vectorL + vectorV;
			vectorH.NormalizeSelf();

			const float lightColor = light->color.ToFloat(colorComponent);
            const float currentLight = shadow * lightColor * ((objMat.matteMagnitude * Od * std::max(0.f, vectorN.DotProduct(vectorL)))
                    + (objMat.shinyMagnitude * Os * powf(std::max(0.f, vectorN.DotProduct(vectorH)), objMat.specularHighlightFocus)));

			if constexpr (!shadowMode)
				lightSum += currentLight;
			else
				lightSum += shadow;
		}

		float illumination = 0.0f;
		if constexpr (!shadowMode)
			illumination = (objMat.diffuseReflectionMagnitude * Od) + lightSum;
		else
			illumination = lightSum;

		std::clamp(illumination, 0.0f, 1.0f);

		return illumination;
	};

	float illuminationRed = phongIllumination(ColorRGB::Component::RED);
	float illuminationGreen = phongIllumination(ColorRGB::Component::GREEN);
	float illuminationBlue = phongIllumination(ColorRGB::Component::BLUE);

	return {illuminationRed, illuminationGreen, illuminationBlue};
}
