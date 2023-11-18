// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda

//#include <parallel/algorithm>
//#include <execution>

#include <iostream>

#include <omp.h>

#include "GraphicsEngine.hpp"
#include "InputFileParser.hpp"
#include "PpmWriter.hpp"
#include "core/Texture.hpp"
#include "TextureCache.hpp"
#include "core/TypeDefinitions.hpp"

#include "tests.hpp"

int
main(int argc, char* argv[])
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
	const size_t pixelBufferSize = scene.imagePixelSize.width * scene.imagePixelSize.height;

	std::vector<PixelInfo> pixelBuffer(pixelBufferSize);

	// Pre-populate the pixel array with information about the corresponding X and Y coordinate.
	size_t currentPixelIndex = 0;
	for (uint32_t currentY = 0; currentY < scene.imagePixelSize.height; currentY++) {
		for (uint32_t currentX = 0; currentX < scene.imagePixelSize.width; currentX++) {
			pixelBuffer[currentPixelIndex].x = currentX;
			pixelBuffer[currentPixelIndex].y = currentY;
			currentPixelIndex++;
		}
	}

	// (3) Define Viewing Window
	std::cout << "=== Defining View Window ===" << std::endl;
	CoordSys coordinateSystem(scene.viewDirection, scene.upDirection);
	ViewingWindow window(coordinateSystem, scene.viewDirection, scene.eyePosition, scene.fovVertical, scene.imagePixelSize);

	// (4) Ray Casting Time!
	std::cout << "=== Casting The Rays ===" << std::endl;
	Ray wildRay{};
	wildRay.origin = scene.eyePosition;

	// The depth to use!
	const uint32_t depthChoice = 2;

	// This does not run in multiple threads for some reason...
//	std::for_each(pixelBuffer.begin(), pixelBuffer.end(), [&](auto& pixelInfo) {
//		// Map the current pixel of the image to a point on the view window.
//		Point2D<uint32_t> currentPoint(pixelInfo.x, pixelInfo.y);
//		Point3D viewWindowPoint = window.MapImagePixelToPoint(scene.imagePixelSize, currentPoint);
//		// Point the ray towards the view window.
//		wildRay.SetDirectionFromIntersection(viewWindowPoint);
//
//		// Determine objects intersected by ray &
//		// which intersected object is closest to the camera!
//		pixelInfo.pixel = GraphicsEngine::TraceWithRay(wildRay, scene, depthChoice);
//	});

//	for (auto& pixelInfo : pixelBuffer)
//	{
//		// Map the current pixel of the image to a point on the view window.
//		Point2D<uint32_t> currentPoint(pixelInfo.x, pixelInfo.y);
//		Point3D viewWindowPoint = window.MapImagePixelToPoint(scene.imagePixelSize, currentPoint);
//		// Point the ray towards the view window.
//		wildRay.SetDirectionFromIntersection(viewWindowPoint);
//
//		pixelInfo.pixel = GraphicsEngine::TraceWithRay(wildRay, scene, depthChoice);
//	}

	#pragma omp parallel firstprivate(wildRay) shared(pixelBuffer)
	{
		#pragma omp for schedule(static)
		for (auto& pixelInfo : pixelBuffer)
		{
			// Map the current pixel of the image to a point on the view window.
			Point2D<uint32_t> currentPoint(pixelInfo.x, pixelInfo.y);
			Point3D viewWindowPoint = window.MapImagePixelToPoint(scene.imagePixelSize, currentPoint);
			// Point the ray towards the view window.
			wildRay.SetDirectionFromIntersection(viewWindowPoint);

			pixelInfo.pixel = GraphicsEngine::TraceWithRay(wildRay, scene, depthChoice);
		}
	}

	// Write out PPM File!
	std::cout << "=== Writing Out PPM File ===" << std::endl;
	PPMWriter writer{};

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
