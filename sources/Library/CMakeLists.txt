cmake_minimum_required(VERSION 3.6) 

set(MODULE ${PROJECT_NAME})

list(APPEND THIS_INCLUDES_DIRS
            ${INCLUDES_DIR}
)

list(APPEND THIS_MOCK_INCLUDES_DIRS
            ${INCLUDES_DIR}/mocks
)

INCLUDE_DIRS_AND_FILES("${THIS_INCLUDES_DIRS}" THIS_INCLUDES)
INCLUDE_DIRS_AND_FILES("${THIS_MOCK_INCLUDES_DIRS}" THIS_MOCK_INCLUDES)

find_package(GTest REQUIRED)

add_library(${MODULE} INTERFACE)

target_link_libraries(${MODULE} INTERFACE 
                              Event_Model::Event_Model
                              Information_Model::Information_Model
                              HaSLL::HaSLL
                              GTest::GTest
)

target_include_directories(${MODULE} INTERFACE
                           $<BUILD_INTERFACE:${INCLUDES_DIR}>
                           $<BUILD_INTERFACE:${INCLUDES_DIR}/mocks>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
)

target_compile_features(${MODULE} INTERFACE cxx_std_17)

install(
    TARGETS ${MODULE}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES ${THIS_INCLUDES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(
    FILES ${THIS_MOCK_INCLUDES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/mocks
)