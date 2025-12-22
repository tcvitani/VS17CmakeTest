'''
Created on 16. 12. 2025.

@author: Tihomir Cvitanic
'''

import subprocess
import sys
import io
import os.path
import re
import glob
import shutil
import codecs
import pyLogger 


def removeDoubleBackslashes(sPath):
    return sPath.replace("\\\\", "\\")

def replaceBackslashes2Slashes(sPath):
    return sPath.replace("\\", "/")

def replaceSlashes2Backslashes(sPath):
    return sPath.replace("/", "\\")

def removeLastSlash(sPath):
    if sPath[-1:]=="/":
        sPath = sPath[:-1]
    return sPath

def removeLastBackslash(sPath):
    sPath = sPath.replace("/", "\\")
    if sPath[-1:]=="\\":
        sPath = sPath[:-1]
    return sPath  
def removeFirstBackslash(sPath):
    sPath = sPath.replace("/", "\\")
    if sPath[0]=="\\":
        sPath = sPath[1:]
    return sPath  


def prepareBuildEnviron(aVSenvironment):
    print("prepareBuildEnviron:...")
    cmd = r'"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" && set'
    
    try:
        #subprocess.CREATE_NEW_CONSOLE
        #os.path.expandvars()
        #for entry in os.environ:
        #    print(entry + "=" + os.environ[entry])

        pProcess = subprocess.Popen(cmd, shell=True, text=True, start_new_session=False, bufsize=100, stdout=subprocess.PIPE)
        stdout, _ = pProcess.communicate()

        for line in stdout.splitlines():
            if '=' in line:
                key, value = line.split('=', 1)
                aVSenvironment[key] = value

        return pProcess.returncode 
    except:
        print("prepareBuildEnviron Error detected...") 
        return pProcess.returncode





def callProcessAndShowOutput(sCommand, build_environment):
    #print("Executing:[" + sCommand + "]")
    
    try:
        #subprocess.CREATE_NEW_CONSOLE
        #os.path.expandvars()
        #for entry in os.environ:
        #    print(entry + "=" + os.environ[entry])
        bIsBuildError = False

        pProcess = subprocess.Popen(args=sCommand, shell=True, start_new_session=False, bufsize=100, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=build_environment)
        
        while(pProcess.poll() == None):
            message = pProcess.stdout.readline()
            if message!=None:
                typeMessage = type(message)
                if(typeMessage == str or (typeMessage == bytes and message != b"")):
                    strMessage = str(message)
                    
                    if len(strMessage)>0:
                        print(strMessage)
                        strMessage = strMessage.lower()
                        if(strMessage.find("error")!=-1):
                            bIsBuildError = True
                        
        if(bIsBuildError):
            return False

        return True       
    except:
        print("Error detected...") 
        return False

def testExtension(fname, replace_extensions=""):
    if replace_extensions:
        return fname.lower().endswith(replace_extensions)
    return True    




class CBuildAllRecursively:
    """Class to buld all CMAKE projects in repository recursevly"""
    def __init__(self, sRootFolderPath):
        self.m_sRootFolderPath = sRootFolderPath   

    def setBuildEnvironment(self, build_environment):
        self.build_environment = build_environment

    def BuildFolder(self, full_dirpath):
        print("Building folder ****" + full_dirpath)
        os.chdir(full_dirpath) 
        bAllOK = False

        if(callProcessAndShowOutput(["cmake", "--preset", "Debug-x64"], self.build_environment) == True):
            bAllOK = callProcessAndShowOutput(["ninja", "-C", "out/build/debug"], self.build_environment)


        if(bAllOK and callProcessAndShowOutput( ["cmake", "--preset", "Release-x64"], self.build_environment) == True):
            bAllOK = callProcessAndShowOutput(["ninja", "-C", "out/build/release"], self.build_environment)

        if(bAllOK):
            print("****Building module [" + full_dirpath + "] Success !!! ******************** ")
        else:
            print("************************** ERROR building module [" + full_dirpath + "] !!! ******************** ")


    def RecrusiveFind(self, full_dirpath):
        #print ("Checking folder:" + full_dirpath)
        with os.scandir(full_dirpath) as it:
             for entry in it:
                if (entry.is_file() and entry.name.lower() == "cmakelists.txt"):
                    self.BuildFolder(full_dirpath)
                    return
                
        with os.scandir(full_dirpath) as it:        
            for entry in it:
                if (entry.is_dir()):
                    dir_path = os.path.join(full_dirpath, entry.name)
                    self.RecrusiveFind(dir_path)       

       
    def BuildAll(self):
        root = self.m_sRootFolderPath
        try:
            self.RecrusiveFind(root)
        except:
            print ("Error  building [" + self.m_sRootFolderPath + "]")
            return 
        

repository = "c:\\D_DISK\\Work\\Test_projects\\QT6_test\\VS17CmakeTest\\TradToll\\"

gRepLocalSourcesDirBase = os.curdir
gLogFilePath = os.path.join(gRepLocalSourcesDirBase, "BuildAll.log")

sys.stdout = pyLogger.Logger(gLogFilePath)
print("************************** Starting BuildAll script for folder " + repository)    

aVSenvironment = os.environ.copy()

if (prepareBuildEnviron(aVSenvironment) == 0):
    sys.stdout = pyLogger.Logger(gLogFilePath)

    print("Extracted environment:")
    for entry in aVSenvironment:
        print(entry + "=" + aVSenvironment[entry])

    x = CBuildAllRecursively(repository)   
    x.setBuildEnvironment(aVSenvironment)
    x.BuildAll()
else:
    print("Error preparing build environment!")