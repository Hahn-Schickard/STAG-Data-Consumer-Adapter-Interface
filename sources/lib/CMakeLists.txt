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

find_package(GTest REQUIRED)

add_library(${THIS} INTERFACE)

target_link_libraries(${THIS} INTERFACE 
                              Event_Model::Event_Model
                              Information_Model::Information_Model
                              HaSLL::HaSLL
                              GTest::GTest
)

target_include_directories(${THIS} INTERFACE
                           $<BUILD_INTERFACE:${INCLUDES_DIR}>
                           $<BUILD_INTERFACE:${INCLUDES_DIR}/mocks>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
)

target_compile_features(${THIS} INTERFACE cxx_std_17)

install(
    TARGETS ${THIS}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    FILES ${THIS_INCLUDES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(
    FILES ${THIS_MOCK_INCLUDES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/mocks
)