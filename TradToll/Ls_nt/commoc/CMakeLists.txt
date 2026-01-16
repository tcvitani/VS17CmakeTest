cmake_minimum_required(VERSION 3.16)
project(CommocSvc LANGUAGES C )

 
# Project
message("-- 'cmhosts' INCLUDED in the build")
add_subdirectory(cmhosts)

message("-- 'cmworkers' INCLUDED in the build")
add_subdirectory(cmworkers)

message("-- 'CommocSvc' INCLUDED in the build")
add_subdirectory(CommocSvc)

 
