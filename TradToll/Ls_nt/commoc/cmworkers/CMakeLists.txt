cmake_minimum_required(VERSION 3.16)
project(CmWorkers LANGUAGES C )

 
# Project
message("-- 'CmWorkers _interface' INCLUDED in the build")
add_subdirectory("_interface")

message("-- 'CmWorkers cmwork_cmworkex' INCLUDED in the build")
add_subdirectory("cmwork_cmworkex")


 
