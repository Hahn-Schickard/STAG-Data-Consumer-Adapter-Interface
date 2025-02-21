cmake_minimum_required(VERSION 3.6)

set(THIS Unit_Tests_Runner)

file(GLOB DCAI_TestSuite "${CMAKE_CURRENT_LIST_DIR}/Data_Consumer_Adapter_Interface_Tests/*")

#@+ ====================== User TEST_SUITES configuration ==========================
list(APPEND TEST_SUITES
    ${DCAI_TestSuite}
)
#@- ========================= END OF USER CONFIGURATION ============================

add_executable(${THIS})
target_sources(${THIS} PRIVATE
                       "testRunner.cpp"
                       ${TEST_SUITES}
)

#@+ ==================== User TEST_DEPENDCIES configuration =======================
list(APPEND TEST_DEPENDCIES
                       Data_Consumer_Adapter_Interface
                       HaSLL::HaSLL
)
#@- ========================= END OF USER CONFIGURATION ===========================

target_link_libraries(${THIS} PUBLIC
                              ${TEST_DEPENDCIES}
                              GTest::gtest
)

add_test(
    NAME ${THIS}
    COMMAND ${THIS}
)

set_target_properties(${THIS} PROPERTIES
                              CXX_STANDARD 17
)

if(WIN32)
    add_custom_command(TARGET ${THIS} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy
                        -t $<TARGET_FILE_DIR:${THIS}>
                        $<TARGET_RUNTIME_DLLS:${THIS}>
                        COMMAND_EXPAND_LISTS
    )
    # Workaround to get conan .dll files into build
    foreach(directory ${CONAN_RUNTIME_LIB_DIRS})
        file(GLOB dir_dependencies "${directory}/*.dll")
        if(dir_dependencies)
            list(APPEND dependencies "${dir_dependencies}")
        endif()
    endforeach()
    add_custom_command(TARGET ${THIS} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy
                        -t $<TARGET_FILE_DIR:${THIS}>
                        ${dependencies}
                        COMMAND_EXPAND_LISTS
    )
endif()
