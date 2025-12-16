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

def callProcessAndShowOutput(sCommand):
    print("Executing:[" + sCommand + "]")
    
    bDoRun = True
    
    try:
        pProcess = subprocess.Popen(sCommand, shell=True, bufsize=0, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        
        while bDoRun:
            line = pProcess.stdout.readline()
            #line = line.replace('\r', '').replace('\n', '')
            print(line)
            sys.stdout.flush()
            #if line.find("Error")!=-1:
            #    bDoRun = False
            if pProcess.poll()!=None:
                return pProcess.returncode
                
    except:
        print("Error detected...") 
        return pProcess.returncode

def testExtension(fname, replace_extensions=""):
    if replace_extensions:
        return fname.lower().endswith(replace_extensions)
    return True    




class CBuildAllRecursively:
    """Class to buld all CMAKE projects in repository recursevly"""
    def __init__(self, sRootFolderPath):
        self.m_sRootFolderPath = sRootFolderPath   

    def BulildFolder(self, full_dirpath):
        print("Building folder ****" + full_dirpath)
        os.chdir(full_dirpath) #callProcessAndShowOutput("cd " + );
        if(callProcessAndShowOutput("cmake --preset \"Debug-x64\"") == 0):
            callProcessAndShowOutput("ninja -C out/build/debug")

    def RecrusiveFind(self, full_dirpath):
        #print ("Checking folder:" + full_dirpath)
        
        for dirpath, dirnames, filenames in os.walk(full_dirpath):
            cmakelistsFound = False
            
            for file_name in filenames:
                if(file_name.lower() == "cmakelists.txt"):
                    cmakelistsFound = True
                    self.BulildFolder(full_dirpath)
                    return

            if(cmakelistsFound == False):        
                for dir_name in dirnames:
                    dir_path = os.path.join(dirpath, dir_name)
                    self.RecrusiveFind(dir_path)       

        
    def BuildAll(self):
        root = self.m_sRootFolderPath
        try:
            self.RecrusiveFind(root)
        except:
            print ("Error  building [" + self.m_sRootFolderPath + "]")
            return 
        
#        #everything OK by now...
#        #enter "Script" folder and process iss files
#        #  replace "_r.dll" with ".dll" in  all files
#        self.sTargetDebugSubFolderScript = os.path.join(self.sTargetDebugFolder , self.m_sSubDirScripts)
#        
#        self.processReleaseToDebugFile(os.path.join(self.sTargetDebugSubFolderScript, "FilesAll.iss"))
#        self.processReleaseToDebugFile(os.path.join(self.sTargetDebugSubFolderScript, "SetupScript.iss"))
#        self.processFileToExcludeUnwantedBlocks(os.path.join(self.sTargetDebugSubFolderScript, "SetupScript.iss"))
#        
#        #replace "_r.dll" with ".dll" in  all files in all other files (reg and bat)
#        self.processMassReplaceForBAT(os.path.join(self.sTargetDebugFolder, self.m_sSubDirParameters))
#        self.processMassReplaceForUnicodeReg(os.path.join(self.sTargetDebugFolder, self.m_sSubDirParameters))
#        
#        print ("Debug installation processed ok to folder:" + self.sTargetDebugFolder)
#        return
    
                                
#    def processReleaseToDebugFile(self, sCurrentFile):
#        print ("processReleaseToDebugFile: Processing file:" + sCurrentFile)
#        findAndReplaceInFile(sCurrentFile, "_r.dll", ".dll")
#        findAndReplaceInFile(sCurrentFile, "_r.exe", ".exe")
#        findAndReplaceInFile(sCurrentFile, "_r_", "_")
#        findAndReplaceInFile(sCurrentFile, "OutputBaseFilename=", "OutputBaseFilename=Dbg_")
#       
#    def processReleaseToDebugFileUnicode(self, sCurrentFile):
#        print ("processReleaseToDebugFileUnicode: Processing file:" + sCurrentFile)
#        if findAndReplaceInUnicodeFile(sCurrentFile, u"_r.dll", u".dll") == False:
#            print ("findAndReplaceInUnicodeFile:returned False")
#            sys.exit(1) 
#        if findAndReplaceInUnicodeFile(sCurrentFile, u"_r.exe", u".exe") == False:
#            print ("findAndReplaceInUnicodeFile:returned False")
#            sys.exit(1) 
#        if findAndReplaceInUnicodeFile(sCurrentFile, u"_r_", u"_") == False:
#            print ("findAndReplaceInUnicodeFile:returned False")
#            sys.exit(1)     
      
#    def processFileToExcludeUnwantedBlocks(self, sFilePath):
#        print ("processFileToExcludeUnwantedBlocks: Processing file:" + sFilePath)
#        try:
#            sResultFilePath = sFilePath + ".tmp"
#            #create a new file
#            resultFile = io.open(sResultFilePath,'w')
#            #load the original content
#            sourceFile = io.open(sFilePath)
#
#            rePatStart = re.compile(r"\*\*\*START REMOVE IN DEBUG",re.IGNORECASE)
#            rePatEnd = re.compile(r"\*\*\*END REMOVE IN DEBUG",re.IGNORECASE)
#            
#            bInBlockToComment = False
#            
#            for line in sourceFile:
#                if bInBlockToComment == False:
#                    if re.search(rePatStart, line) != None:
#                        bInBlockToComment = True
#                    resultFile.write(line)
#                else:
#                    if re.search(rePatEnd, line) != None:
#                        bInBlockToComment = False
#                        resultFile.write(line)
#                    else:
#                        resultFile.write("//"+line)
#        except:
#            print ("processFileToExcludeUnwantedBlocks: Error replacing in file:" + sFilePath)
#            return False
#        finally:
#            resultFile.close()
#            sourceFile.close()
#            
#        try:
#            shutil.move(sResultFilePath, sFilePath)
#        except:
#            print ("processFileToExcludeUnwantedBlocks: Error moving file:" + sResultFilePath + " to:" + sFilePath)
#            return False       
#    
#        return True 
        
#    def processMassReplaceForBAT(self, dir_name):
#        for dirpath, dirnames, filenames in os.walk(dir_name):
#            for fname in filenames:
#                if testExtension(fname,".bat"):
#                    fullname = os.path.join(dirpath, fname)
#                    self.processReleaseToDebugFile(fullname)
                    
#    def processMassReplaceForUnicodeReg(self, dir_name):
#        for dirpath, dirnames, filenames in os.walk(dir_name):
#            for fname in filenames:
#                if testExtension(fname,".reg"):
#                    fullname = os.path.join(dirpath, fname)
#                    self.processReleaseToDebugFileUnicode(fullname)



repository = "c:\\D_DISK\\Work\\Test_projects\\QT6_test\\VS17CmakeTest\\TradToll\\"
callProcessAndShowOutput("C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Auxiliary\\Build\\vcvars64.bat")

x = CBuildAllRecursively(repository)   
x.BuildAll()

#run = True

#while(run == True):
#    # Do not attempt clean up on first run
#    #print("Cleaning up repository...")
#    #if callProcessAndShowOutput('svn ' + 'cleanup ' + repository) == 0: 
#    #    print("Cleaning up completed")
#    #else:
#    #    print "Error cleaning up!"
#    #    break
#    
#    print("Updating repository...")
#    #p = subprocess.Popen("cmd", shell=True, bufsize=0, stdout=subprocess.PIPE)
#    #commandSvnUP = "svn " + "update --set-depth infinity  " + repository
#    #updating = True
#    
#    p = subprocess.Popen(commandSvnUP, shell=True, bufsize=0, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
#    
#    while(p.poll() == None and run == True):
#        message = p.stdout.readline()
#        typeMessage = type(message)
#        if(typeMessage == str or (typeMessage == bytes and message != b"")):
#            strMessage = str(message)
#            print(strMessage)
#            sys.stdout.flush()
#            
#            resultFind = strMessage.find("At revision")
#            if(resultFind != -1):
#                print("Update completed")
#                updating = False
#                run = False
#            elif strMessage.find("svn: E155037")!=-1 or strMessage.find("svn: E175012")!=-1:
#                print("Restarting...")
#                updating = False
#                run = True 
#                break              
#            elif strMessage.find("svn: E")!=-1:
#                print("Fatal error detected!")
#                updating = False
#                run = False
#        else:
#            updating = False
#        

