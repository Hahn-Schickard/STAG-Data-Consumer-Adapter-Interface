#@+ ======================== User TARGET NAME configuration ============================
set(THIS Example)
#@- =========================== END OF USER CONFIGURATION ===============================
set(TARGET ${PROJECT_NAME}_${THIS})

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_executable(${TARGET})

target_sources(${TARGET} 
    PRIVATE
        ${sources_list}
)
#@+ ======================== User DEPENDENCIES configuration ============================
target_link_libraries(${TARGET}
    PRIVATE
        ${PROJECT_NAME}
        Information_Model_Mocks::Information_Model_Mocks
        Variant_Visitor::Variant_Visitor
)
#@- =========================== END OF USER CONFIGURATION ===============================
target_compile_features(${TARGET} PUBLIC cxx_std_17)
IMPORT_TARGET_DLLS(${TARGET})
