cmake_minimum_required(VERSION 3.6)

#@+ ========================== User configuration ===========================
set(THIS Data_Consumer_Adapter_Interface)
#@- ===================== END OF USER CONFIGURATION =========================

project(${THIS} CXX)

enable_testing()

set(INCLUDES_DIR "${PROJECT_SOURCE_DIR}/includes")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(VERBOSE_FILE_INCLUSION ON)

option(POSITION_INDEPENDENT_CODE "Compiles the codebase with fPIC flag enabled." ON)
if(POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE on)
endif()

option(STATIC_CODE_ANALYSIS "Activates static code analysis during the code compilation" ON)

include(GNUInstallDirs)
include(macros)
include(compiler_settings)
include(linter)
include(ClangFormat)
include(conan)
conan_cmake_run(CONANFILE conanfile.txt 
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

add_subdirectory(sources)
add_subdirectory(unit_tests)
