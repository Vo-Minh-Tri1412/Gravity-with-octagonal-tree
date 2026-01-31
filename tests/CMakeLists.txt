enable_testing()

find_package(gtest CONFIG REQUIRED)

add_executable(core_tests core_tests.cpp)
target_link_libraries(core_tests PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME core_tests COMMAND core_tests)

add_executable(physics_tests physics_tests.cpp)
target_link_libraries(physics_tests PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME physics_tests COMMAND physics_tests)

add_executable(octree_tests octree_tests.cpp)
target_link_libraries(octree_tests PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME octree_tests COMMAND octree_tests)