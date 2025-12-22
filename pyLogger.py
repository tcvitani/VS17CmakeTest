'''
Created on 30. 9. 2013.

@author: TIC
'''

import sys
import re

class Logger(object):
    def __init__(self, filename="Default.log"):
        self.terminal = sys.stdout
        self.log = open(filename, "a")
        self.cr_pattern = re.compile("^.*\r", re.M)
        self.bs_pattern = re.compile(".\b")

    def write(self, message):
        self.terminal.write(message)
        message = self.bs_pattern.sub('', self.cr_pattern.sub('', message))
        self.log.write(message)
    
    def flush(self):
        self.log.flush()
