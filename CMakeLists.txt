cmake_minimum_required(VERSION 3.21)
project(Assignment1d_TransparencyAndMirrorReflections CXX)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fopenmp)
add_link_options(-fopenmp)
#add_compile_definitions(-D_GLIBCXX_PARALLEL)
#link_libraries(-lgomp)

add_executable(raytracer1d
        src/GraphicsEngine.cpp
        src/GraphicsEngine.hpp
        src/InputFileParser.cpp
        src/InputFileParser.hpp
        src/main.cpp
        src/PpmWriter.cpp
        src/PpmWriter.hpp
        src/core/Light.hpp
        src/core/Object.cpp
        src/core/Object.hpp
        src/core/TypeDefinitions.hpp
        src/core/Texture.hpp
        src/core/Texture.cpp
        src/TextureCache.cpp
        src/TextureCache.hpp
        src/core/Ray.cpp
        src/core/Ray.hpp
        src/core/Vector3D.cpp
        src/core/Vector3D.hpp
        src/core/Point.hpp
        src/core/ColorRGB.hpp
        src/tests.hpp
)
