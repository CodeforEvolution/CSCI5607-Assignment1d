// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda

#include <iostream>

#include "GraphicsEngine.hpp"
#include "InputFileParser.hpp"
#include "PpmWriter.hpp"
#include "core/Texture.hpp"
#include "core/TextureCache.hpp"
#include "core/TypeDefinitions.hpp"

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
#endif

void
test_triangle()
{
    Triangle triangle;
    triangle.vertexA = Point3D(1, 0, 0);
    triangle.vertexB = Point3D(0, 1, 0);
    triangle.vertexC = Point3D(0, 0, 1);

    Ray ray;
    ray.origin = Point3D(0, 0, 0);
    ray.direction = Vector3D(1.f/3, 2.f/3, 2.f/3);
    ray.direction.NormalizeSelf();

    std::optional<Point3D> maybeIntersection = triangle.IntersectWith(ray, nullptr);
    std::cerr << "Intersection: " << *maybeIntersection << std::endl;
}

int main(int argc, char* argv[])
{
	// Check arguments...
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <Path to input file>" << std::endl;
		return EXIT_FAILURE;
	}

	// (1) Start reading in the input file!
	std::cout << "=== Reading in Input File ===" << std::endl;

	auto inputFilePath = std::filesystem::path(argv[1]);
	if (!std::filesystem::exists(inputFilePath)) {
		std::cerr << "The provided filepath does not exist! Exiting..." << std::endl;
		return EXIT_FAILURE;
	}

	SceneDefinition scene;

	InputFileParser parser(inputFilePath);
	bool result = parser.Parse(scene);
	parser.Close();
	if (!result) {
		std::cerr << "Couldn't parse the input file! Did you format it correctly?" << std::endl;
		return EXIT_FAILURE;
	}

    // Let's set our current working directory to where the scene definition file was found.
    std::error_code error;
    std::filesystem::current_path(inputFilePath.remove_filename(), error);
    if (error) {
        std::cerr << "(Error) Failed to switch current working directory to directory containing the scene definition file." << std::endl;
        return EXIT_FAILURE;
    }

	// Print out Scene Definition
	std::cout << "Scene Definition Read:" << std::endl;
	std::cout << scene << std::endl;

    // Pre-Load Texture Files
    std::cout << "=== Pre-Loading Texture Files ===" << std::endl;
    for (const auto& object : scene.objectList) {
        if (object->texturePath.empty())
            continue;

        if (TextureCache::Instance().HasTexture(object->texturePath))
            continue;

        if (!TextureCache::Instance().LoadTexture(object->texturePath)) {
            std::cerr << "(Error) Failed to start loading texture from file: " << object->texturePath << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "\tLoaded Texture: " << object->texturePath << std::endl;
    }

    // Wait on textures to finish loading...
//    TextureCache::Instance().WaitForTextureLoad();
    std::cout << "\tFinished loading textures!" << std::endl;

	// (2) Pixel Array Time!
	std::cout << "=== Creating Pixel Array ===" << std::endl;
	const uint64_t pixelBufferSize = scene.imagePixelSize.width * scene.imagePixelSize.height;

	std::vector<ColorRGB> pixelBuffer;
	pixelBuffer.resize(pixelBufferSize);

	// (3) Define Viewing Window
	std::cout << "=== Defining View Window ===" << std::endl;
	CoordSys coordinateSystem(scene.viewDirection, scene.upDirection);
	ViewingWindow window(coordinateSystem, scene.viewDirection, scene.eyePosition, scene.fovVertical, scene.imagePixelSize);

	// (4) Ray Casting Time!
	std::cout << "=== Casting The Rays ===" << std::endl;
	Ray wildRay{};
	wildRay.origin = scene.eyePosition;

	std::size_t currentPixelIndex = 0;
	for (uint32_t currentY = 0; currentY < scene.imagePixelSize.height; currentY++) {
		for (uint32_t currentX = 0; currentX < scene.imagePixelSize.width; currentX++) {
			// Map the current pixel of the image to a point on the view window.
			Point2D<uint32_t> currentPoint(currentX, currentY);
			Point3D viewWindowPoint = window.MapImagePixelToPoint(scene.imagePixelSize, currentPoint);
			// Point the ray towards the view window.
			wildRay.SetDirectionVector(viewWindowPoint);

			// Determine objects intersected by ray &
			// which intersected object is closest to the camera!
			pixelBuffer[currentPixelIndex] = GraphicsEngine::TraceWithRay(wildRay, scene);
			currentPixelIndex++;
		}
	}

	// Write out PPM File!
	std::cout << "=== Writing Out PPM File ===" << std::endl;
	PPMWriter writer;

	if (!ppm_writer_open(argv[1], &writer))
		return EXIT_FAILURE;

	ppm_writer_set_image_size(&writer, scene.imagePixelSize);

	// Write out the pixels in ASCII PPM format
	if (!ppm_writer_write(&writer, pixelBuffer)) {
		std::cerr << "Failed to write out pixels to the PPM file." << std::endl;
		return EXIT_FAILURE;
	}

	ppm_writer_close(&writer);

	std::cout << "All done! Have a fine day! :)" << std::endl;
	return EXIT_SUCCESS;
}
