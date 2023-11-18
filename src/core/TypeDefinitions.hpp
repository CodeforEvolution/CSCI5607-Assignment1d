// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda
#ifndef TYPE_DEFINITIONS_H
#define TYPE_DEFINITIONS_H

#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>
#include <optional>
#include <memory>

#include "ColorRGB.hpp"

/** Helpers **/

#define MAKE_SHARED_NAME(TYPE) \
    using Shared##TYPE = std::shared_ptr<TYPE>

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
		<< "n: " << materialProps.specularHighlightFocus << ", "
		<< "α: " << materialProps.opacity << ", "
		<< "η: " << materialProps.refractionIndex;

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


struct PixelInfo {
	ColorRGB pixel;
	uint32_t x;
	uint32_t y;
};

#endif // TYPE_DEFINITIONS_H
