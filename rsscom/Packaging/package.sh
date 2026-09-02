#!/bin/bash

#conan source . --source-folder=tmp/source
#conan install . --install-folder=tmp/build
#conan build . --source-folder=tmp/source --build-folder=tmp/build
conan package . --source-folder=/var/tmp/rsscom/Linux-Debug/src/ --build-folder=/var/tmp/rsscom/Linux-Debug/build/ --package-folder=/var/tmp/rsscom/Linux-Debug/build/package
conan export-pkg -f . rss/test --source-folder=/var/tmp/rsscom/Linux-Debug/src/ --build-folder=/var/tmp/rsscom/Linux-Debug/build/ -s compiler.cppstd=17 -s arch=x86_64 -s arch_build=x86_64 -s build_type=Debug
#conan test test_package rsscom/0.1.2@rss/alpha

#--To test the dependencies:
#conan info -g graph.html .