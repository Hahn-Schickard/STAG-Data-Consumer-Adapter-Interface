cmake_minimum_required(VERSION 3.6)

set(THIS Unit_Tests_Runner)

file(GLOB Data_Consumer_Adapter_Interface_Test_Suite "${CMAKE_CURRENT_LIST_DIR}/Data_Consumer_Adapter_Interface_Tests/*")

find_package(GTest REQUIRED)

add_executable(${THIS})
target_sources(${THIS} PRIVATE 
                       "testRunner.cpp"
                       ${Data_Consumer_Adapter_Interface_Test_Suite}
)

target_link_libraries(${THIS} PUBLIC
                              GTest::GTest
                              Data_Consumer_Adapter_Interface
                              HaSLL::HaSLL
)

add_test(
    NAME ${THIS}
    COMMAND ${THIS}
)

set_target_properties(${THIS} PROPERTIES
                              CXX_STANDARD 17
)