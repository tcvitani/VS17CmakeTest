cmake_minimum_required(VERSION 3.16)
project(CmHosts LANGUAGES C )

 
# Project
message("-- 'CmHosts _interface' INCLUDED in the build")
add_subdirectory("_interface")


message("-- 'CmHosts cmftp' INCLUDED in the build")
add_subdirectory("cmftp")

message("-- 'CmHosts cmras' INCLUDED in the build")
add_subdirectory("cmras")
