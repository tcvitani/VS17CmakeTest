##############################################
#                                            #
#		   RSSCOM library Tests				 #
#                 @Emovis 2019               #
#                                            #
##############################################

cmake_minimum_required (VERSION 4.3)

file(GLOB_RECURSE RSSCOM_CONSUMER_SRCS "*.cpp" )

add_executable(RSSCom_Consumer ${RSSCOM_CONSUMER_SRCS})

target_link_libraries(RSSCom_Consumer RSSCom RdKafka::rdkafka++)

#install conan dependencies
install(TARGETS RSSCom_Consumer DESTINATION ${CMAKE_BINARY_DIR}/bin)
#install(DIRECTORY ${CONAN_LIB_DIRS_RSSCORE}/ DESTINATION ${CMAKE_BINARY_DIR}/bin PATTERN "lib/*")
