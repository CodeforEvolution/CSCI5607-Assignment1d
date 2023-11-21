// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda

#include "PpmWriter.hpp"

#include <cstring>


bool
ppm_writer_open(const char* outputFileName, PPMWriter* writer)
{
	if (outputFileName == nullptr || outputFileName[0] == '\0' || writer == nullptr)
		return false;

	const size_t outputFileNameLength = strlen(outputFileName) + strlen(kPPMExtension);
	char name[outputFileNameLength];
	strcpy(name, outputFileName);
	strcat(name, kPPMExtension);

	writer->outputFile = fopen(name, "w+");
	if (writer->outputFile == nullptr)
		return false;

	writer->width = writer->height = 0;
	writer->maxColorValue = DEFAULT_MAX_COLOR_VALUE;

	std::cout << "Final PPM file is being written to: " << name << std::endl;
	std::cout.flush();

	return true;
}

void
ppm_writer_close(PPMWriter* writer)
{
	if (writer == nullptr)
		return;

	fclose(writer->outputFile);
	writer->outputFile = nullptr;
}

void
ppm_writer_set_image_size(PPMWriter* writer, Size imageSize)
{
	if (writer == nullptr)
		return;

	writer->width = imageSize.width;
	writer->height = imageSize.height;
}

bool
ppm_writer_write(PPMWriter* writer, const std::vector<PixelInfo>& pixels)
{
	if (writer == nullptr || writer->outputFile == nullptr)
		return false;

	// Write Out PPM Image Header
	int result = fprintf(writer->outputFile, "%s\n%s\n%u %u\n%u\n",
						 kPPMASCIIMagicNumber, kPPMHeaderComment, writer->width, writer->height, writer->maxColorValue);
	if (result < 0) {
		fprintf(stderr, "Failed to write out PPM image header...Error: %d\n", result);
		return false;
	}

	// Check if there is anything to do.
	const std::size_t pixelCount = writer->width * writer->height;
	if (pixelCount == 0)
		return true;

	// Ensure that the passed in pixels is equivalent to the desired size of the PPM image in pixels.
	if (pixelCount != pixels.size())
		return false;

	// Write Out Pixels
	for (size_t index = 0; index < pixelCount; index++) {
		// Limit to 70 characters per line, so we'll just print
		result = fprintf(writer->outputFile, "%d %d %d\n",
						 pixels[index].pixel.red,
						 pixels[index].pixel.green,
						 pixels[index].pixel.blue);
		if (result < 0) {
			fprintf(stderr, "Failed to write pixel out...Error: %d\n", result);
			return false;
		}
	}

	return true;
}