cmake_minimum_required(VERSION 3.20)

project(liquid LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LIQUID_ENABLE_SANITIZERS "Build Liquid and tests with AddressSanitizer and UBSan" OFF)

add_library(liquid
    src/BehaviorRegistry.cpp
    src/ComponentRegistry.cpp
    src/Coordinator.cpp
    src/IntentRegistry.cpp
    src/SystemRegistry.cpp
)

target_compile_features(liquid PUBLIC cxx_std_20)

target_include_directories(liquid
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

function(liquid_apply_sanitizers target_name)
    if(LIQUID_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target_name}
            PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )
        target_link_options(${target_name}
            PRIVATE
                -fsanitize=address,undefined
        )
    endif()
endfunction()

liquid_apply_sanitizers(liquid)

include(CTest)

if(BUILD_TESTING)
    set(LIQUID_TESTS
        test_ids
        test_component_storage
        test_component_registry
        test_behavior_registry
        test_intent_registry
        test_coordinator
        test_system_registry
        test_stress
    )

    foreach(test_name IN LISTS LIQUID_TESTS)
        add_executable(${test_name} tests/${test_name}.cpp)
        target_link_libraries(${test_name} PRIVATE liquid)
        liquid_apply_sanitizers(${test_name})
        string(REPLACE "test_" "" ctest_name ${test_name})
        add_test(NAME ${ctest_name} COMMAND ${test_name})
    endforeach()

endif()
