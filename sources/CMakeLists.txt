cmake_minimum_required(VERSION 3.6) 

set(THIS Data_Consumer_Adapter_Interface)


list(APPEND THIS_INCLUDES_DIRS
            ${INCLUDES_DIR}
)

list(APPEND THIS_MOCK_INCLUDES_DIRS
            ${INCLUDES_DIR}/mocks
)

INCLUDE_DIRS_AND_FILES("${THIS_INCLUDES_DIRS}" THIS_INCLUDES)
INCLUDE_DIRS_AND_FILES("${THIS_MOCK_INCLUDES_DIRS}" THIS_MOCK_INCLUDES)

add_library(${THIS} INTERFACE)

target_include_directories(${THIS} INTERFACE
                           $<BUILD_INTERFACE:${INCLUDES_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(${THIS}_MOCK INTERFACE)


target_link_libraries(${THIS}_MOCK INTERFACE 
                              CONAN_PKG::gtest
                              ${THIS}
)

target_include_directories(${THIS} INTERFACE
                           $<BUILD_INTERFACE:${INCLUDES_DIR}/mocks>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/mocks>
)

target_compile_features(${THIS} INTERFACE cxx_std_17)

install(
    TARGETS ${THIS}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${THIS_MOCK_INCLUDES_DIRS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES ${THIS_INCLUDES}
    TYPE INCLUDE 
)
