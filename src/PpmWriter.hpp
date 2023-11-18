// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda
#ifndef PPM_WRITER_H
#define PPM_WRITER_H

#include <cstdio>
#include <cstdint>

#include "core/TypeDefinitions.hpp"


static const char* kPPMExtension = ".ppm";
static const char* kPPMASCIIMagicNumber = "P3";
static const char* kPPMHeaderComment = "# PPM file created by Jacob Secunda's program!";

#define DEFAULT_MAX_COLOR_VALUE 255;

struct PPMWriter {
	FILE* outputFile;

	uint32_t width;
	uint32_t height;
	uint32_t maxColorValue;
};

bool ppm_writer_open(const char* outputFileName, PPMWriter* writer);
void ppm_writer_close(PPMWriter* writer);

void ppm_writer_set_image_size(PPMWriter* writer, Size imageSize);
bool ppm_writer_write(PPMWriter* writer, const std::vector<PixelInfo>& pixels);

#endif // PPM_WRITER_H