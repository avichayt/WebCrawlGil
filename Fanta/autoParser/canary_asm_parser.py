import re


class CanaryASMParser:
    def __init__(self, asm_file):
        self._asm_file = open(asm_file, 'r')
        self._out_file = open('tmpFile.s', 'w')
        self._canary_functions_str = """_canaryStart:
	.intel_syntax noprefix
    sub esp, 8
    push eax
    push ebx
	mov eax, [esp + 16]
    mov [esp + 8], eax
    lea eax, [esp + 12]
    push eax
	call getNewCanary
	add esp, 4
	mov ebx, eax
	mov eax, [ebx]
	mov [esp + 12], eax
	mov eax, [ebx + 4]
	mov [esp + 16], eax
	mov eax, [esp + 20]
	xor [esp + 16], eax
	pop ebx
	pop eax
	ret
	.att_syntax 

_canaryEnd:
	.intel_syntax noprefix
	push eax
	push edx
	push ebx
	push ecx
	call popCanary
	
	mov ecx, [esp + 0x18]
	xor ecx, [esp + 0x1c]
	mov edx, [esp + 0x14]
	
	mov ebx, eax
	cmp [ebx + 4], ecx
	jnz EXIT
	cmp [ebx], edx
	jnz EXIT
	jmp DONE
	EXIT:
	    push FAILURE_CANARY_MSG
	    call printf
		push -1
		call exit
	DONE:
	    pop ecx
		pop ebx
		pop edx
		mov eax, [esp + 4]
		mov [esp + 0x0c], eax
		pop eax
		add esp, 8
		ret
	.att_syntax 
"""

    def parse(self):
        line = self._asm_file.readline()
        while line:

            mtch = re.match("([a-z_][a-zA-Z0-9_]*):", line)
            if mtch:
                if mtch.group(1) == 'main':
                    self._out_file.write('\n' + self._canary_functions_str)
                    self._out_file.write(line)
                    self.parse_func(True)
                elif mtch.group(1) != 'initCanary':
                    self._out_file.write(line)
                    self.parse_func(False)
                else:
                    self._out_file.write(line)
            else:
                self._out_file.write(line)

            line = self._asm_file.readline()


        self._asm_file.close()
        self._out_file.close()

    def parse_func(self, isMain):

        if isMain:
            #self._out_file.write('\tcall randomizeStack\n')
            self._out_file.write('\tpushl 8(%esp)\n')
            self._out_file.write('\tcall initProtector\n')
            self._out_file.write('\taddl $4, %esp\n')
            #self._out_file.write('\tcall _canaryStart\n')
        if not isMain:
            self._out_file.write('\tcall _canaryStart\n')

        line = self._asm_file.readline()
        while line:
            # push ebp
            mtch = re.match("([\\s]*)?push.*?[\\s].*%ebp.*", line)
            if mtch:
                x=1
            else:
                # ret
                mtch = re.match("([\\s]*)?ret[n]?.*", line)
                if mtch:
                    if not isMain:
                        self._out_file.write(mtch.group(1) + 'call _canaryEnd\n')
                    self._out_file.write(line)
                    return
                else:
                    #access ebp
                    mtch = re.match(".*[\\s](([0-9]+?)\\(%ebp\\)).*", line)
                    if mtch:
                        line = line.replace(mtch.group(1), str(int(mtch.group(2)) + 8) + "(%ebp)")


            self._out_file.write(line)
            line = self._asm_file.readline()


def main():
    cp = CanaryASMParser("main.s")
    cp.parse()
