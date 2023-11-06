// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda

#ifndef ASSIGNMENT1B_LIGHTANDSHADOW_INPUTFILEPARSER_HPP
#define ASSIGNMENT1B_LIGHTANDSHADOW_INPUTFILEPARSER_HPP

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>
#include <optional>

#include "core/Light.hpp"
#include "core/TypeDefinitions.hpp"
#include "GraphicsEngine.hpp"

class InputFileParser {
public:
	InputFileParser();
	explicit InputFileParser(const std::filesystem::path& filePath);

	~InputFileParser();

	bool Open(const std::filesystem::path& filePath);
	bool IsOpen() const;
	void Close();

	bool Parse(SceneDefinition& definition);

private:
	bool parse_eye_position(std::string_view line, Point3D& parsedPosition);
	bool parse_view_direction(std::string_view line, Vector3D& parsedDirection);
	bool parse_up_direction(std::string_view line, Vector3D& parsedDirection);
	bool parse_vertical_fov(std::string_view line, float& parsedAngle);
	bool parse_image_size(std::string_view line, Size& parsedSize);
	bool parse_background_color(std::string_view line, ColorRGB& parsedColor);
	bool parse_legacy_material_color(std::string_view line, ColorRGB& parsedColor);
	bool parse_sphere(std::string_view line, Point3D& parsedCenter, float& parsedRadius);
	// Optional Assignment 1A
	bool parse_parallel(std::string_view line, float& parsedFrustumHeight);
	bool parse_cylinder(std::string_view line, Point3D& parsedCenter, Vector3D& parsedDirection, float& parsedRadius, float& parsedLength);

	// Assignment 1B
	bool parse_material_properties(std::string_view line, MaterialProps& parsedMaterial);
	bool parse_light(std::string_view line, std::unique_ptr<Light>& parsedLight);

    // Assignment 1C
    bool parse_vertex(std::string_view line, std::unique_ptr<Point3D>& parsedVertex);
    bool parse_triangle(std::string_view line, VertNormTextIndex& parsedA, VertNormTextIndex& parsedB, VertNormTextIndex& parsedC);
    bool parse_vertex_normal(std::string_view line, std::unique_ptr<Vector3D>& parsedVertexNormal);
    bool parse_texture(std::string_view line, std::filesystem::path& parsedTexturePath);
    bool parse_texture_coordinate(std::string_view line, std::unique_ptr<TextureCoordinate>& parsedTextureCoordinate);

private:
	std::ifstream fInputFile;
};


#endif //ASSIGNMENT1B_LIGHTANDSHADOW_INPUTFILEPARSER_HPP
