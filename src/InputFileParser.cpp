// Assignment 1c - Triangles and Textures
// Work by Jacob Secunda

#include "InputFileParser.hpp"

#include <bitset>
#include <cmath>
#include <iostream>
#include <map>
#include <string>


// Defines

enum TokenType {
	// Extra
	TOKEN_COMMENT = 0,
	// Assignment 1a
	TOKEN_EYE_POS,
	TOKEN_VIEW_DIR,
	TOKEN_UP_DIR,
	TOKEN_VERT_FOV,
	TOKEN_IMAGE_SIZE,
	TOKEN_BACKGROUND_COLOR,
	TOKEN_MATERIAL_PROPERTIES,
	TOKEN_SPHERE,
	// Optional: Assignment 1a
	TOKEN_PARALLEL,
	TOKEN_CYLINDER,
	// Assignment 1b
	TOKEN_LIGHT,
	// Optional: Assignment 1b
	TOKEN_DEPTH_CUEING,
	TOKEN_LIGHT_ATTENUATION,
    // Assignment 1c
    TOKEN_VERTEX,
    TOKEN_TRIANGLE,
    TOKEN_VERTEX_NORMAL,
    TOKEN_TEXTURE,
    TOKEN_TEXTURE_COORDINATE
};

const std::map<const std::string_view, TokenType> kValidTokenMap = {
		{"#",           TOKEN_COMMENT},
		// Assignment 1a
		{"eye",         TOKEN_EYE_POS},
		{"viewdir",     TOKEN_VIEW_DIR},
		{"updir",       TOKEN_UP_DIR},
		{"vfov",        TOKEN_VERT_FOV},
		{"imsize",      TOKEN_IMAGE_SIZE},
		{"bkgcolor",    TOKEN_BACKGROUND_COLOR},
		{"mtlcolor",    TOKEN_MATERIAL_PROPERTIES},
		{"sphere",      TOKEN_SPHERE},
		// Optional: Assignment 1a
		{"parallel",    TOKEN_PARALLEL},
		{"cylinder",    TOKEN_CYLINDER},
		// Assignment 1b
		{"light",       TOKEN_LIGHT},
		{"depthcueing", TOKEN_DEPTH_CUEING},
		{"attlight",    TOKEN_LIGHT_ATTENUATION},
        {"v",           TOKEN_VERTEX},
        {"f",           TOKEN_TRIANGLE},
        {"vn",          TOKEN_VERTEX_NORMAL},
        {"texture",     TOKEN_TEXTURE},
        {"vt",          TOKEN_TEXTURE_COORDINATE}
};

/* Primary Functions */
InputFileParser::InputFileParser()
	:
	fInputFile()
{
}

InputFileParser::InputFileParser(const std::filesystem::path& filePath)
	:
	fInputFile(filePath)
{
}

InputFileParser::~InputFileParser()
{
	fInputFile.close();
}

bool
InputFileParser::Open(const std::filesystem::path& filePath)
{
	if (fInputFile.is_open()) {
		fInputFile.close();
	}

	fInputFile.open(filePath);
	return fInputFile.is_open();
}

bool
InputFileParser::IsOpen() const
{
	return fInputFile.is_open();
}

void
InputFileParser::Close()
{
	fInputFile.close();
}

bool
InputFileParser::Parse(SceneDefinition& definition)
{
	// Bitmask for tracking tokens that were discovered.
	// This helps validate a Scene Description that had no invalid tokens,
	// though might still be missing necessary information.
	enum : uint8_t {
		HAS_IMSIZE = 0,
		HAS_EYE,
		HAS_VIEWDIR,
		HAS_VFOV,
		HAS_UPDIR,
		HAS_BKGCOLOR,
		HAS_MTLCOLOR,
		HAS_OBJECT
	};
	constexpr std::bitset<std::numeric_limits<uint8_t>::digits> kKeyTokens = HAS_IMSIZE | HAS_EYE | HAS_VIEWDIR | HAS_VFOV | HAS_UPDIR | HAS_BKGCOLOR;
	std::bitset<std::numeric_limits<uint8_t>::digits> parsedTokens = 0;

	while(!fInputFile.eof())
	{
		std::string currentLine;
		std::getline(fInputFile, currentLine);
		if (!fInputFile.eof() && fInputFile.fail()) {
			std::cerr << "Failed to read a line from the input file!" << std::endl;
			return false;
		}

		// Skip over blank lines!
		if (currentLine.empty() || currentLine.starts_with('\n') || currentLine.starts_with('\r'))
			continue;

		std::string_view lineToken = currentLine.substr(0, currentLine.find_first_of(' '));
		if (!kValidTokenMap.contains(lineToken)) {
			std::cerr << "An invalid keyword token was found in the file: " << lineToken << std::endl;
			return false;
		}

		static MaterialProps currentMaterialProps{};
        static std::filesystem::path currentTexturePath;
		switch (kValidTokenMap.at(lineToken)) {
			case TOKEN_COMMENT:
			{
				// This is a comment, skip the line!
				break;
			}

			case TOKEN_EYE_POS:
			{
				if (!parse_eye_position(currentLine, definition.eyePosition)) {
					std::cerr << "Error: Failed to parse eye position line: " << currentLine << std::endl;
					return false;
				}

				parsedTokens.set(HAS_EYE);
				break;
			}

			case TOKEN_VIEW_DIR:
			{
				if (!parse_view_direction(currentLine, definition.viewDirection)) {
					std::cerr << "Error: Failed to parse view direction line: " << currentLine << std::endl;
					return false;
				}

				parsedTokens.set(HAS_VIEWDIR);
				break;
			}

			case TOKEN_UP_DIR:
			{
				if (!parse_up_direction(currentLine, definition.upDirection)) {
					std::cerr << "Error: Failed to parse up direction line: " << currentLine << std::endl;
					return false;
				}

				parsedTokens.set(HAS_UPDIR);
				break;
			}

			case TOKEN_VERT_FOV:
			{
				if (!parse_vertical_fov(currentLine, definition.fovVertical)) {
					std::cerr << "Error: Failed to parse vertical field of view line: " << currentLine << std::endl;
					return false;
				}

				parsedTokens.set(HAS_VFOV);
				break;
			}

			case TOKEN_IMAGE_SIZE:
			{
				if (!parse_image_size(currentLine, definition.imagePixelSize)) {
					std::cerr << "Error: Failed to parse image size line: " << currentLine << std::endl;
					return false;
				}

				parsedTokens.set(HAS_IMSIZE);
				break;
			}

			case TOKEN_BACKGROUND_COLOR:
			{
				if (!parse_background_color(currentLine, definition.backgroundColor, definition.refractionIndexBgColor)) {
					std::cerr << "Error: Failed to parse background color line: " << currentLine << std::endl;
					return false;
				}

				parsedTokens.set(HAS_BKGCOLOR);
				break;
			}

			case TOKEN_MATERIAL_PROPERTIES:
			{
				if (!parse_material_properties(currentLine, currentMaterialProps)) {
					std::cerr << "Failed to parse material color/properties line: " << currentLine << std::endl;
					return false;
				}

				parsedTokens.set(HAS_MTLCOLOR);
				break;
			}

			case TOKEN_SPHERE:
			{
				// Make sure a material color was set
				if (!parsedTokens.test(HAS_MTLCOLOR)) {
					std::cerr << "Error: An object was encountered, though there's no material color set yet!" << std::endl;
					return false;
				}

				std::unique_ptr<Sphere> sphere = std::make_unique<Sphere>();
				if (!parse_sphere(currentLine, sphere->center, sphere->radius)) {
					std::cerr << "Error: Failed to parse sphere line: " << currentLine << std::endl;
					return false;
				}
				sphere->material = currentMaterialProps;
                sphere->texturePath = currentTexturePath;

				definition.objectList.push_back(std::move(sphere));

				parsedTokens.set(HAS_OBJECT);
				break;
			}

			case TOKEN_PARALLEL:
			{
				if (!parse_parallel(currentLine, definition.frustumHeight)) {
					std::cerr << "Error: Failed to parse parallel line: " << currentLine << std::endl;
					return false;
				}
				break;
			}

			case TOKEN_CYLINDER:
			{
				// Make sure a material color was set
				if (!parsedTokens.test(HAS_MTLCOLOR)) {
					std::cerr << "Error: An object was encountered, though there's no material color set yet!" << std::endl;
					return false;
				}

				auto cylinder = std::make_unique<Cylinder>();
				if (!parse_cylinder(currentLine, cylinder->center, cylinder->direction, cylinder->radius, cylinder->length)) {
					std::cerr << "Error: Failed to parse cylinder line: " << currentLine << std::endl;
					return false;
				}
				cylinder->material = currentMaterialProps;
                cylinder->texturePath = currentTexturePath;

				definition.objectList.push_back(std::move(cylinder));

				parsedTokens.set(HAS_OBJECT);
				break;
			}

			case TOKEN_LIGHT:
			{
				std::unique_ptr<Light> light = nullptr;
				if (!parse_light(currentLine, light)) {
					std::cerr << "Failed to parse light line: " << currentLine << std::endl;
					return false;
				}

				definition.lightList.push_back(std::move(light));

				break;
			}

            case TOKEN_VERTEX:
            {
                auto vertex = std::make_unique<Point3D>();
                if (!parse_vertex(currentLine, vertex)) {
                    std::cerr << "Failed to parse vertex line: " << currentLine << std::endl;
                    return false;
                }

                definition.vertexList.push_back(std::move(vertex));

                break;
            }

            case TOKEN_TRIANGLE:
            {
                // Indexes start at 1!
                // Pairs vertex index with possibly a vertex normal index
                VertNormTextIndex vertexA;
                VertNormTextIndex vertexB;
                VertNormTextIndex vertexC;

                if (!parse_triangle(currentLine, vertexA, vertexB, vertexC)) {
                    std::cerr << "Failed to parse triangle line: " << currentLine << std::endl;
                    return false;
                }

                auto [vertexIndexA, vertexNormalIndexA, vertexTextureIndexA] = vertexA;
                auto [vertexIndexB, vertexNormalIndexB, vertexTextureIndexB] = vertexB;
                auto [vertexIndexC, vertexNormalIndexC, vertexTextureIndexC] = vertexC;

                const auto checkVertex = [&definition](size_t index) {
                    if (index <= 0 || index > definition.vertexList.size()) {
                        std::cerr << "Triangle tried to use unavailable vertex #" << index << std::endl;
                        return false;
                    }

                    return true;
                };

                if (!checkVertex(vertexIndexA) || !checkVertex(vertexIndexB) || !checkVertex(vertexIndexC))
                    return false;

                auto triangle = std::make_unique<Triangle>();
                triangle->vertexA = *definition.vertexList[vertexIndexA - 1];
                triangle->vertexB = *definition.vertexList[vertexIndexB - 1];
                triangle->vertexC = *definition.vertexList[vertexIndexC - 1];

                // Check if vertex normals were parsed
                if (vertexNormalIndexA && vertexNormalIndexB && vertexNormalIndexC)
                {
                    const auto checkVertexNormal = [&definition](size_t index) {
                        if (index <= 0 || index > definition.vertexNormalList.size()) {
                            std::cerr << "Triangle tried to use unavailable vertex normal #" << index << std::endl;
                            return false;
                        }

                        return true;
                    };

                    if (checkVertexNormal(*vertexNormalIndexA) && checkVertexNormal(*vertexNormalIndexB) && checkVertexNormal(*vertexNormalIndexC)) {
                        triangle->vertexNormalA = *definition.vertexNormalList[*vertexNormalIndexA - 1];
                        triangle->vertexNormalB = *definition.vertexNormalList[*vertexNormalIndexB - 1];
                        triangle->vertexNormalC = *definition.vertexNormalList[*vertexNormalIndexC - 1];
                    }
                }

                // Check if texture coordinates were parsed
                if (vertexTextureIndexA && vertexTextureIndexB && vertexTextureIndexC) {
                    const auto checkTextureIndex = [&definition](size_t index) {
                        if (index <= 0 || index > definition.textureCoordinateList.size()) {
                            std::cerr << "Triangle tried to use unavailable texture coordinate index #" << index << std::endl;
                            return false;
                        }

                        return true;
                    };

                    if (checkTextureIndex(*vertexTextureIndexA) && checkTextureIndex(*vertexTextureIndexB) && checkTextureIndex(*vertexTextureIndexC)) {
                        triangle->textureCoordinateA = *definition.textureCoordinateList[*vertexTextureIndexA - 1];
                        triangle->textureCoordinateB = *definition.textureCoordinateList[*vertexTextureIndexB - 1];
                        triangle->textureCoordinateC = *definition.textureCoordinateList[*vertexTextureIndexC - 1];
                    }
                }

                triangle->material = currentMaterialProps;
                triangle->texturePath.assign(currentTexturePath);

                definition.objectList.push_back(std::move(triangle));

                break;
            }

            case TOKEN_VERTEX_NORMAL:
            {
                auto vertexNormal = std::make_unique<Vector3D>();
                if (!parse_vertex_normal(currentLine, vertexNormal)) {
                    std::cerr << "Failed to parse vertex normal line: " << currentLine << std::endl;
                    return false;
                }

                definition.vertexNormalList.push_back(std::move(vertexNormal));

                break;
            }

            case TOKEN_TEXTURE:
            {
                std::filesystem::path texturePath;
                if (!parse_texture(currentLine, texturePath)) {
                    std::cerr << "Failed to parse texture path line: " << currentLine << std::endl;
                    return false;
                }

                currentTexturePath = texturePath;

                break;
            }

            case TOKEN_TEXTURE_COORDINATE:
            {
                auto textureCoordinate = std::make_unique<TextureCoordinate>();
                if (!parse_texture_coordinate(currentLine, textureCoordinate)) {
                    std::cerr << "Failed to parse texture coordinate: " << currentLine << std::endl;
                    return false;
                }

                definition.textureCoordinateList.push_back(std::move(textureCoordinate));

                break;
            }

			default:
			{
				// The previous check should have caught everything...
				std::cerr << "We should not be able to get here!" << std::endl;
				__builtin_unreachable();
			}
		}
	}

	// Let's make sure we have all essential tokens for the Scene Definition
	if ((parsedTokens & kKeyTokens) != kKeyTokens) {
		std::cerr << "Critical tokens are missing from the Scene Definition file!" << std::endl;
		std::cerr << "Missing:" << std::endl;

		if (!parsedTokens.test(HAS_IMSIZE))
			std::cerr << "\timsize" << std::endl;

		if (!parsedTokens.test(HAS_EYE))
			std::cerr << "\teye" << std::endl;

		if (!parsedTokens.test(HAS_VIEWDIR))
			std::cerr << "\tviewdir" << std::endl;

		if (!parsedTokens.test(HAS_VFOV))
			std::cerr << "\tvfov" << std::endl;

		if (!parsedTokens.test(HAS_UPDIR))
			std::cerr << "\tupdir" << std::endl;

		if (!parsedTokens.test(HAS_BKGCOLOR))
			std::cerr << "\tbkgcolor" << std::endl;

		return false;
	}

	std::cout << "Read in and parsed all input! :D" << std::endl;
	return true;
}


/* Helper Functions */

bool
InputFileParser::parse_eye_position(std::string_view line, Point3D& parsedPosition)
{
	std::istringstream stream(line.data());

	// Ignore first keyword...
	stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

	// Eye Position X
	stream >> parsedPosition.x;

	// Eye Position Y
	stream >> parsedPosition.y;

	// Eye Position Z
	stream >> parsedPosition.z;

	return !stream.fail();
}

bool
InputFileParser::parse_view_direction(std::string_view line, Vector3D& parsedDirection)
{
	std::istringstream stream(line.data());

	// Ignore first keyword...
	stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

	// View Direction X
	stream >> parsedDirection.dx;

	// View Direction Y
	stream >> parsedDirection.dy;

	// View Direction Z
	stream >> parsedDirection.dz;

	return !stream.fail();
}

bool
InputFileParser::parse_up_direction(std::string_view line, Vector3D& parsedDirection)
{
	std::istringstream stream(line.data());

	// Ignore first keyword...
	stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

	// Up Direction X
	stream >> parsedDirection.dx;

	// Up Direction Y
	stream >> parsedDirection.dy;

	// Up Direction Z
	stream >> parsedDirection.dz;

	return !stream.fail();
}

bool
InputFileParser::parse_vertical_fov(std::string_view line, float& parsedAngle)
{
	std::istringstream stream(line.data());

	// Ignore first keyword...
	stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

	// Vertical Field Of View Angle (float)
	stream >> parsedAngle;

	return !stream.fail();
}

bool
InputFileParser::parse_image_size(std::string_view line, Size& parsedSize)
{
	std::istringstream stream(line.data());

	// Ignore first keyword...
	stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

	// Image Width
	stream >> parsedSize.width;

	// Image Height
	stream >> parsedSize.height;

	return !stream.fail();
}

bool
InputFileParser::parse_background_color(std::string_view line, ColorRGB& parsedColor, float& parsedRefractionIndex)
{
	std::istringstream stream(line.data());

	float colorValue = 0.f;

	// Ignore first keyword...
	stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

	// Background Color Red
	stream >> colorValue;

	parsedColor.red = (uint8_t)roundf(colorValue * 255);

	// Background Color Green
	stream >> colorValue;

	parsedColor.green = (uint8_t)roundf(colorValue * 255);

	// Background Color Blue
	stream >> colorValue;

	parsedColor.blue = (uint8_t)roundf(colorValue * 255);

	// Refraction Index (Assignment 1D)
	stream >> parsedRefractionIndex;


	return !stream.fail();
}

bool
InputFileParser::parse_legacy_material_color(std::string_view line, ColorRGB& parsedColor)
{
	std::istringstream stream(line.data());

	float colorValue = 0.0f;

	// Ignore first keyword...
	stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

	// Material Color Red
	stream >> colorValue;
	parsedColor.red = static_cast<uint8_t>(roundf(colorValue * 255));

	// Material Color Green
	stream >> colorValue;
	parsedColor.green = static_cast<uint8_t>(roundf(colorValue * 255));

	// Material Color Blue
	stream >> colorValue;
	parsedColor.blue = static_cast<uint8_t>(roundf(colorValue * 255));

	return !stream.fail();
}

bool
InputFileParser::parse_sphere(std::string_view line, Point3D& parsedCenter, float& parsedRadius)
{
	std::istringstream stream(line.data());

	// Ignore first keyword...
	stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

	// Center Point X
	stream >> parsedCenter.x;

	// // Center Point Y
	stream >> parsedCenter.y;

	// Center Point Z
	stream >> parsedCenter.z;

	// Sphere's Radius
	stream >> parsedRadius;

	return !stream.fail();
}

// Parallel/frustum height handling is unsupported...
bool
InputFileParser::parse_parallel(std::string_view line, float& parsedFrustumHeight)
{
	std::cerr << "Unsupported parallel line: " << line << std::endl;
	return false;
}

// Cylinder handling is unsupported...
bool
InputFileParser::parse_cylinder(std::string_view line, Point3D& parsedCenter, Vector3D& parsedDirection, float& parsedRadius, float& parsedLength)
{
	std::cerr << "Unsupported cylinder line: " << line << std::endl;
	return false;
}

bool
InputFileParser::parse_material_properties(std::string_view line, MaterialProps& parsedMaterial)
{
	std::istringstream stream(line.data());

	// Ignore first keyword...
	stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

	float tempColorValue = 0.0f;

	// Intrinsic Material Color
	stream >> tempColorValue;
	parsedMaterial.intrinsicColor.red = static_cast<uint8_t>(roundf(tempColorValue * 255));
	stream >> tempColorValue;
	parsedMaterial.intrinsicColor.green = static_cast<uint8_t>(roundf(tempColorValue * 255));
	stream >> tempColorValue;
	parsedMaterial.intrinsicColor.blue = static_cast<uint8_t>(roundf(tempColorValue * 255));

	// Specular Highlight Color
	stream >> tempColorValue;
	parsedMaterial.specularHighlightColor.red = static_cast<uint8_t>(roundf(tempColorValue * 255));
	stream >> tempColorValue;
	parsedMaterial.specularHighlightColor.green = static_cast<uint8_t>(roundf(tempColorValue * 255));
	stream >> tempColorValue;
	parsedMaterial.specularHighlightColor.blue = static_cast<uint8_t>(roundf(tempColorValue * 255));

	// Diffuse Reflection Magnitude
	stream >> parsedMaterial.diffuseReflectionMagnitude;

	// Matte Magnitude
	stream >> parsedMaterial.matteMagnitude;

	// Shiny Magnitude
	stream >> parsedMaterial.shinyMagnitude;

	// Specular Highlight Focus
	stream >> parsedMaterial.specularHighlightFocus;

	// Opacity
	stream >> parsedMaterial.opacity;

	// Refraction Index
	stream >> parsedMaterial.refractionIndex;

	return !stream.fail();
}

bool
InputFileParser::parse_light(std::string_view line, std::unique_ptr<Light>& parsedLight)
{
	std::istringstream stream(line.data());

	// Ignore first keyword...
	stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

	float tempX = 0.0f;
	float tempY = 0.0f;
	float tempZ = 0.0f;
	stream >> tempX;
	stream >> tempY;
	stream >> tempZ;

	// Directional Light or Point Light (w -> 0 or 1)
	unsigned int tempLightType = 0;
	stream >> tempLightType;

	switch (tempLightType) {
		case Light::DIRECTIONAL_LIGHT: {
			std::unique_ptr<DirectionalLight> light = std::make_unique<DirectionalLight>();
			light->direction = Vector3D(tempX, tempY, tempZ);

			parsedLight = std::move(light);

			break;
		}

		case Light::POINT_LIGHT: {
			std::unique_ptr<PointLight> light = std::make_unique<PointLight>();
			light->position = Point3D(tempX, tempY, tempZ);

			parsedLight = std::move(light);

			break;
		}

		default: {
			std::cerr << "Error: Unknown Light Type: " << std::to_string(tempLightType) << std::endl;
			return false;
		}
	}

	// Light Color
	float tempColorValue = 0.0f;

	stream >> tempColorValue;
	parsedLight->color.red = static_cast<uint8_t>(roundf(tempColorValue * 255));
	stream >> tempColorValue;
	parsedLight->color.green = static_cast<uint8_t>(roundf(tempColorValue * 255));
	stream >> tempColorValue;
	parsedLight->color.blue = static_cast<uint8_t>(roundf(tempColorValue * 255));

	return !stream.fail();
}

bool
InputFileParser::parse_vertex(std::string_view line, std::unique_ptr<Point3D>& parsedVertex)
{
    std::istringstream stream(line.data());

    // Ignore first keyword...
    stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

    // Vertex X
    stream >> parsedVertex->x;

    // Vertex Y
    stream >> parsedVertex->y;

    // Vertex Z
    stream >> parsedVertex->z;

    return !stream.fail();
}

bool
InputFileParser::parse_triangle(std::string_view line, VertNormTextIndex& parsedA, VertNormTextIndex& parsedB, VertNormTextIndex& parsedC)
{
    std::istringstream stream(line.data());

    // Ignore first keyword...
    stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

    const auto extractVertexAndCoords = [&stream](VertNormTextIndex& parsedIndexes) -> bool {
        std::string stringPiece;
        stream >> stringPiece;
        const std::size_t maybeDoubleSlashIndex = stringPiece.find("//");
        const std::size_t slashCount = std::count(stringPiece.begin(), stringPiece.end(), '/');
        if (maybeDoubleSlashIndex != std::string::npos && slashCount == 2) {
            // Smooth shaded untextured triangle

            // Vertex Index
            parsedIndexes.vertexIndex = std::strtoul(stringPiece.substr(0, maybeDoubleSlashIndex).c_str(), nullptr, 10);

            // Vertex Surface Normal Index
            parsedIndexes.vertexNormalIndex = std::strtoul(stringPiece.substr(maybeDoubleSlashIndex + slashCount).c_str(), nullptr, 10);
        } else if (slashCount == 0) {
            // Flat shaded, untextured triangle

            // Vertex Index
            parsedIndexes.vertexIndex = std::strtoul(stringPiece.c_str(), nullptr, 10);
        } else if (slashCount == 1) {
            // Flat shaded, textured triangle

            const std::size_t slashIndex = stringPiece.find('/');

            // Vertex Index
            parsedIndexes.vertexIndex = std::strtoul(stringPiece.substr(0, slashIndex).c_str(), nullptr, 10);

            // Texture Normal Index
            parsedIndexes.textureCoordIndex = std::strtoul(stringPiece.substr(slashIndex + 1).c_str(), nullptr, 10);
        } else if (slashCount == 2) {
            // Smooth shaded, textured triangle

            const std::size_t slashIndexA = stringPiece.find('/');
            const std::size_t slashIndexB = stringPiece.find('/', slashIndexA + 1);

            // Vertex Index
            parsedIndexes.vertexIndex = std::strtoul(stringPiece.substr(0, slashIndexA).c_str(), nullptr, 10);

            // Texture Coordinate Index
			parsedIndexes.textureCoordIndex = std::strtoul(stringPiece.substr(slashIndexA + 1, slashIndexB).c_str(), nullptr, 10);

            // Vertex Surface Normal Index
            parsedIndexes.vertexNormalIndex = std::strtoul(stringPiece.substr(slashIndexB + 1).c_str(), nullptr, 10);
        } else {
            // Bad triangle! Bad!
            return false;
        }

        return true;
    };

    // Indexes A
    if (!extractVertexAndCoords(parsedA))
        return false;

    // Indexes B
    if (!extractVertexAndCoords(parsedB))
        return false;

    // Indexes C
    if (!extractVertexAndCoords(parsedC))
        return false;

    return !stream.fail();
}

bool
InputFileParser::parse_vertex_normal(std::string_view line, std::unique_ptr<Vector3D>& parsedVertexNormal)
{
    std::istringstream stream(line.data());

    // Ignore first keyword...
    stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

    // Vertex Normal X
    stream >> parsedVertexNormal->dx;

    // Vertex Normal Y
    stream >> parsedVertexNormal->dy;

    // Vertex Normal Z
    stream >> parsedVertexNormal->dz;

    return !stream.fail();
}

bool
InputFileParser::parse_texture(std::string_view line, std::filesystem::path& parsedTexturePath)
{
    std::istringstream stream(line.data());

    // Ignore first keyword...
    stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

    // Texture Path
    std::string pathString;
    stream >> pathString;
    parsedTexturePath.assign("./");
    parsedTexturePath.append(pathString);

    return !stream.fail();
}

bool
InputFileParser::parse_texture_coordinate(std::string_view line, std::unique_ptr<TextureCoordinate>& parsedTextureCoordinate)
{
    std::istringstream stream(line.data());

    // Ignore first keyword...
    stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

    // Texture Coordinate U
    stream >> parsedTextureCoordinate->u;

    // Texture Coordinate V
    stream >> parsedTextureCoordinate->v;

    return !stream.fail();
}