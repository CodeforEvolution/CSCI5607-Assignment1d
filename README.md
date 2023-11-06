# Assignment 1d - Transparency & Mirror Reflections
Work by Jacob Secunda

### Code Outline - RayCaster
#### main.cpp:
- Primary driver of the program
- Runs other functions handling:
    - Reading and parsing the input
    - Preloads Textures
    - Calculating the viewing window
    - Rendering the 3d scene on a 2d plane
    - Outputting the rendering as an ASCII PPM file

#### GraphicsEngine.cpp/.hpp:
- Defines Viewing window (ViewingWindow) and coordinate system (CoordSys) structs
    - Methods for calculating the coordinate system from a viewing direction vector and an up direction vector.
    - Methods for calculating the viewing window's corners given the CoordSys, eye position, image pixel size, viewing direction vector, and vertical FOV.
- Methods for ray tracing and shading calculations

#### InputFileParser.cpp/.hpp
- Reads in input file and delegates different types of line input to other line parsing functions
- Parses different input file data types.
- Performs range checks and other input validation to ensure a valid input file
- Ensures all necessary inputs in the file are handled

#### PpmWriter.(cpp, hpp):
- Manages the state of the output file
- Writes out PPM header to output file
- Writes out the final image pixel data to output file in ASCII PPM format

#### core/Light.hpp:
- Defines the Light struct, along with its sub-structs DirectionalLight and PointLight
- Defines methods for:
    - Generating shadow ray
    - Calculating 'L' (light vector) depending on the type of light
    - Printing information about a light to stream.

#### core/Object.(cpp, hpp):
- Defines the Object struct, along with its sub-structs Sphere, Triangle, and Cylinder
- Each sub-struct of Object defines methods for:
    - Printing information about the Object subclass to stream
    - Calculating the point where a Ray intersects the Object, or if there is an intersection in the first place.
    - Calculating the surface normal vector 'N', which is perpendicular to the Object's surface.
    - Calculating the intrinsic color of an object's surface depending on whether there's a texture available or not.

#### core/Texture.(cpp, hpp):
- Defines the Texture class
- Defines methods for:
  - Retrieving pixels using x,y coordinates or u,v texture coordinates.
  - Settings pixels using x,y coordinates or pixel index.
- Stores information about maximum color values and texture size.

#### core/TextureCache.(cpp, hpp):
- Defines the TextureCache class
- Defines methods for:
  - Loading a texture from a PPM file and caching it into memory
  - Checking for the availability of a cached texture
  - Retrieving a shared, cached texture from the cache using the texture's path 

#### core/TypeDefinitions.hpp:
- Defines primitives:
    - Vectors / Points
    - Size
    - Viewing Window / Coordinate System / Ray
- Defines core data structures
- Scene definition
- Object format
- Valid input file tokens


### How To Build
#### Dependencies:
- CMake version 3.21 or newer
- G++ 10 or newer

#### Configure:

    $ cmake -S "Path to Assignment Source Directory" -B "Path to Assignment Source Directory"/cmake_build_debug -G Ninja

#### Build:

    $ ninja -C "Path to Assignment Source Directory"/cmake_build_debug all

This should result in the built program called "raytracer1d" found in the "cmake_build_debug" folder.

### How To Use
In the cmake_build_debug folder, run:

    $ ./raytracer1d <Path to input file with scene definition information>

### Process
1. Upon its invocation, the RayCaster program will read in and parse the input file into a definition of a scene.
2. Once completed, the program will attempt to preload specified textures into a texture cache.
3. Following this, the program will calculate out the coordinate system and corresponding viewing window that will help project the 3d objects in the world onto a 2d plane.
4. Using this viewing window, we use rays (vectors) colliding with objects in the 3d space to calculate the color of each pixel on the 2d plane. 
    - A ray originating from the viewing window is shot out, and each Object in the scene is tested to intersect with the ray to find the closest intersection.
        - If there isn't an intersection, then a pixel's color is set to the background color.
    - Using the object we hit, our original ray, and all the lights and objects in the scene, we perform the Blinn-Phong Illumination Equation to emulate the properties of light hit the sphere's surface.
        - The object's material properties are factored in.
        - We calculate whether the point on the object is in shadow by calculating a shadow ray depending on the type of light source.
        - The Blinn-Phong Illumination Equation runs for each component of the color (red, green, and blue).
          - The intrinsic color will be replaced with a specific pixel of the texture if the object has a texture applied.
    - This is repeated for each and every pixel in the viewing window.
5. Finally, the calculated pixel data is written out in the ASCII PPM directory format to a file with the same name at the input file with ".ppm" appended.
    - This file will be found in the same directory as the input file.
    - The pixels must be converted from a float representation (0.0 to 1.0) to an integer representation (0 to 255).