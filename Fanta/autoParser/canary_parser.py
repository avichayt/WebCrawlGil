import re
import os
import subprocess
import canary_asm_parser


class CanaryParser:
    def __init__(self, c_file, prog_name, library_path):
        self._prog_name = prog_name
        self._library_path = library_path
        self._c_file = open(c_file, 'r')
        self._c_out_file = open('tmp_c.c', 'w')
        self._declare_canary_str = """#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "library.h"

char* FAILURE_CANARY_MSG = \"buffer overflow!\\n\";

/**
long long ___CANARY___;
void initCanary()
{
    srand(time(NULL));
	___CANARY___ = rand();
	*(((int*)&___CANARY___) + 1) = rand();
	*(((char*)&___CANARY___) + 4) = '\\0';
}**/

"""

    def create_new_c_file(self):
        self._c_out_file.write(self._declare_canary_str)

        line = self._c_file.readline()
        found_main = False
        while line:
            self._c_out_file.write(line)
            line = self._c_file.readline()

        self._c_out_file.close()
        self._c_file.close()

    def compile(self):
        self.create_new_c_file()

        ld = " -Wl,-wrap=fork "

        output = subprocess.check_output("gcc -m32 -fno-stack-protector -z execstack -S tmp_c.c -L" +  self._library_path + " -lprotectLib ", shell=True)
        cap = canary_asm_parser.CanaryASMParser("tmp_c.s")
        cap.parse()
        s = subprocess.check_output("gcc -m32 -no-pie -fno-stack-protector -z execstack tmpFile.s -L" +
                                    self._library_path + " -lprotectLib " + ld + " -o " + self._prog_name, shell=True)
        print('done!')


cp = CanaryParser('test.c', 'test.out', '/home/navot/Desktop/Fanta/protectLib/cmake-build-debug')
cp.compile()
