//
// Created by jake on 11/15/23.
//

#if 0
void testViewWindow()
{
	Size imageSize = Size(100, 100);
	ViewingWindow window{};
	window.upperLeft = {-5, 5, 0};
	window.upperRight = {5, 5, 0};
	window.lowerLeft = {-5,-5,0};
	window.lowerRight = {5,-5,0};

	Point2D<uint32_t> point(33, 66);

	Point3D mappedPoint = window.MapImagePixelToPoint(imageSize, point);
	std::cout << "Mapped Point: " << mappedPoint << std::endl;
}

void testRaySphereIntersect()
{
	Ray ray{};
	ray.origin = {0,0,0};
	ray.direction = {0,0,-1};

	Sphere sphere{};
	sphere.center = {0,0,-10};
	sphere.radius = 4;
	Object object {};
	object.type = ObjectType::OBJ_SPHERE;
	object.properties.sphere = sphere;

	std::optional<Point3D> intersection = ray.TestIntersection(object);

	std::cout << "Intersection: " << intersection.value() << std::endl;
}

void testCoord()
{
	Point3D eyePos = {0,0,5};
	Vector3D upDir = {0,1,0};
	Vector3D viewDir = {0,0,-1};

	CoordSys coordSys(viewDir, upDir);
	std::cout << "u: " << coordSys.u << ", v: " << coordSys.v << std::endl;
}

void testIllumination()
{
	Point3D eyePos = {0,0,5};
	Vector3D rayDirection = {0,0,-1};
	Vector3D lightDirection = {0,-5,0};

	Sphere sphere{};
	sphere.center = {0,0,0};
	sphere.radius = 2;

	ColorRGB Od = {1.f,0.f,0.f};
	ColorRGB Os = {1.f,1.f,1.f};
	float ka = 0.1f;
	float kd = 0.5f;
	float ks = 0.2f;
	float n  = 2;

	Ray theRay{};
	theRay.origin = eyePos;
	theRay.direction = rayDirection;

	MaterialProps props{};
	props.intrinsicColor = Od;
	props.specularHighlightColor = Os;
	props.diffuseReflectionMagnitude = ka;
	props.matteMagnitude = kd;
	props.shinyMagnitude = ks;
	props.specularHighlightFocus = n;

	Object theObject{};
	theObject.type = ObjectType::OBJ_SPHERE;
	theObject.material = props;
	theObject.properties.sphere = sphere;

	std::vector<Object> objects;
	objects.push_back(theObject);

	std::optional<Point3D> intersection = theRay.TestIntersection(theObject);

	Light light{};
	light.color = {1.f,1.f,1.f};
	light.lightType = LightType::DIRECTIONAL_LIGHT;
	light.lightDirection = lightDirection;

	std::vector<Light> lights;
	lights.push_back(light);

	ColorRGB finalColor = GraphicsEngine::ShadeWithRay(theRay, theObject, intersection.value(), lights, objects);
	std::cout << "Shade Color: " << finalColor << std::endl;
}

void
test_triangle()
{
	Triangle triangle;
	triangle.vertexA = Point3D(1, 0, 0);
	triangle.vertexB = Point3D(0, 1, 0);
	triangle.vertexC = Point3D(0, 0, 1);

	Ray ray;
	ray.origin = Point3D(0, 0, 0);
	ray.direction = Vector3D(1.f / 3, 2.f / 3, 2.f / 3);
	ray.direction.NormalizeSelf();

	std::optional<Point3D> maybeIntersection = triangle.IntersectWith(ray, nullptr);
	std::cerr << "Intersection: " << *maybeIntersection << std::endl;
}
#endif

#include "core/Point.hpp"
#include "core/Ray.hpp"
#include "core/Vector3D.hpp"

void
testCalculateVectorR()
{
	/*
	 * Suppose that a ray that originates from the point (2, 3, 5)
	 * hits a surface at a point (3, –1, –3), where the surface
	 * normal is oriented in the direction (0, 1, 0). What is the
	 * direction of the reflected ray?
	 */

	Point3D rayOrigin(2, 3, 5);
	Point3D intersectionPoint(3, -1, -3);
	Vector3D rayDirection(rayOrigin, intersectionPoint);
	rayDirection.NormalizeSelf();

	Ray wildRay;
	wildRay.origin = rayOrigin;
	wildRay.direction = rayDirection;

	Vector3D surfaceNormal(0, 1, 0);

	// I - Faces outward, points from intersection point to incoming ray origin
	Vector3D vectorIPrime = Vector3D(wildRay.origin, intersectionPoint);
	std::cout << "Vector I Prime: " << vectorIPrime << std::endl;
	vectorIPrime.NormalizeSelf();
	std::cout << "Vector I Prime (Normalized): " << vectorIPrime << std::endl;

	Vector3D vectorI = vectorIPrime * -1.f;
	std::cout << "Vector I: " << vectorI << std::endl;

	// a - cos(theta_i) = N dot I
	float a = surfaceNormal.DotProduct(vectorI);
	std::cout << "a: " << a << std::endl;

	// R - Reflected Ray Direction
	Vector3D vectorR = (surfaceNormal * a * 2.f) - vectorI;
	std::cout << "Vector R: " << vectorR << std::endl;
}