cmake_minimum_required(VERSION 3.24)
project(MyGame)

set(CMAKE_CXX_STANDARD 17)
if(APPLE)
    set(CMAKE_CXX_FLAGS "-framework Cocoa -framework IOKit")
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif()

add_executable(MyGame MyGame.cpp ShaderMaker.cpp ShaderMaker.h Nemico.h Basic.h Nemico.cpp GeomUtils.h GeomUtils.cpp Background.h Background.cpp Stelle.cpp Stelle.h PartSystem.cpp PartSystem.h)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

target_link_libraries(MyGame glfw OpenGL::GL GLEW::GLEW glm::glm)