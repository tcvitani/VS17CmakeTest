from conan import ConanFile
from conan.tools.files import copy
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeConfigDeps

import os
import re

class RssComConan(ConanFile):
    name = "rsscom"
    license = "Copyright Emovis (C) 2019"
    author = "Emovis"
    url = "https://192.168.100.220/rss/libs/rsscom"
    description = "RSS Asynchrone Communication Lib"
    topics = ("emovis", "rss", "rsscom")
    settings = "os", "compiler", "build_type", "arch"
    options = {"static_build": [True, False], "build_tests": [True, False]}
    default_options = {"static_build": False, "build_tests": False}
    generators = "CMakeConfigDeps"
    
    def layout(self):
       cmake_layout(self)

    def set_version(self):
       # open the version file
       filepath = os.path.join(self.recipe_folder, "./version")
       with open(filepath, "r") as f:
           str = f.read()
           majver   = re.compile('MAJOR_VERSION ([0-9]+)')
           minver   = re.compile('MINOR_VERSION ([0-9]+)')
           patchver = re.compile('PATCH_VERSION ([0-9]+)')
           majorversion = majver.search(str).group(1)
           minorversion = minver.search(str).group(1)
           patchversion = patchver.search(str).group(1)
           self.version = majorversion + '.' + minorversion + '.' + patchversion
           print("conan package",self.name,self.version)

    def requirements(self):
        self.requires("librdkafka/2.14.2")
        self.test_requires("gtest/1.18.0")

    def source(self):
        try:
            self.run("git clone https://192.168.100.220/rss/libs/rsscom.git")
        except:
            self.run("git clone https://${CONAN_GITLAB_USERNAME}:${CONAN_GITLAB_TOKEN}@192.168.100.220/rss/libs/rsscom.git")
        self.run("cd rsscom && git checkout "+ self.conan_data["ci_commit_sha"])

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables={
              "static_build": self.options.static_build,
              "build_tests": self.options.build_tests,
              "use_conan_cmake": False,
           })
        cmake.build()
        cmake.install()

    def package(self):
           copy(self, "*.h", src=os.path.join(self.source_folder, "rsscom/Includes"),
               dst=os.path.join(self.package_folder, "include"))
           copy(self, "*.lib", src=self.build_folder,
               dst=os.path.join(self.package_folder, "lib"), keep_path=False)
           copy(self, "*.dll", src=self.build_folder,
               dst=os.path.join(self.package_folder, "bin"), keep_path=False)
           copy(self, "libRSSCom.so*", src=self.build_folder,
               dst=os.path.join(self.package_folder, "lib"), keep_path=False, symlinks=True)
           copy(self, "*.a", src=self.build_folder,
               dst=os.path.join(self.package_folder, "lib"), keep_path=False)

    def package_info(self):
        self.cpp_info.libs          = ["RSSCom"]
        self.cpp_info.includedirs   = ['include']
        self.cpp_info.libdirs       = ['lib']
        self.cpp_info.bindirs       = ['bin']

