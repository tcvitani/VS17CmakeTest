##############################################
#                                            #
#		   RSSCOM library Tests      #
#                 @Emovis 2019               #
#                                            #
##############################################

cmake_minimum_required (VERSION 4.3)

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# # Add googletest directly to our build. This defines
# # the gtest and gtest_main targets.
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../Thirdparties/googletest/googletest
                 # ${CMAKE_CURRENT_SOURCE_DIR}/../../Thirdparties/googletest/build
                 # EXCLUDE_FROM_ALL)

file(GLOB_RECURSE RSSCOM_TESTS_SRCS "*.cpp" )

add_executable(RSSCom_Tests ${RSSCOM_TESTS_SRCS})
# link to gtest_main library
target_link_libraries(RSSCom_Tests gtest::gtest RSSCom)

# Add test specifying test name and command to execute it
add_test(NAME Full_RSSCom_Tests COMMAND RSSCom_Tests)

#install conan dependencies
install(TARGETS RSSCom_Tests DESTINATION ${CMAKE_BINARY_DIR}/bin)
#install(DIRECTORY ${CONAN_LIB_DIRS_RSSCORE}/ DESTINATION ${CMAKE_BINARY_DIR}/bin PATTERN "lib/*")
