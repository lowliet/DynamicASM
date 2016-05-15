#include <cstdio>

#include "AsmJit\Src\AsmJit.h"
#pragma comment(lib, "AsmJit\\Bin\\AsmJit.lib")

using namespace AsmJit;

// This is type of function we will generate
typedef void (*MemCpy32Fn)(UINT32*, const UINT32*, sysuint_t);

void* GetAsmCode()
{
	Assembler a;

	// Arguments offset: 4 (first argument) + 4 (push ebp instructions).
	const int arg_offset = 4 + 4;

	// Labels.
	Label L_Loop = a.newLabel();
	Label L_Exit = a.newLabel();

	// Prolog.
	a.push(ebp);
	a.mov(ebp, esp);
	a.push(esi);
	a.push(edi);

	// Fetch arguments (arguments were passed in right-to-left direction, we must read
	// them in left-to-right direction).
	a.mov(edi, dword_ptr(ebp, arg_offset + 0)); // get dst
	a.mov(esi, dword_ptr(ebp, arg_offset + 4)); // get src
	a.mov(ecx, dword_ptr(ebp, arg_offset + 8)); // get len

	// Exit if length is zero.
	a.test(ecx, ecx);
	a.jz(L_Exit);

	// Bind L_Loop label to here.
	a.bind(L_Loop);

	a.mov(eax, dword_ptr(esi));
	a.mov(dword_ptr(edi), eax);

	a.add(esi, 4);
	a.add(edi, 4);

	// Loop until ecx is not zero.
	a.dec(ecx);
	a.jnz(L_Loop);

	// Exit.
	a.bind(L_Exit);

	// Epilog.
	a.pop(edi);
	a.pop(esi);
	a.mov(esp, ebp);
	a.pop(ebp);

	a.ret();

	return a.make();
}

bool Compare(uint32_t *a, uint32_t *b, unsigned int size)
{
	for (unsigned int i = 0; i < size; i++)
	{
		if (a[i] != b[i]) 
		{ 
			return false; 
		}
	}

	return true;
}

int main()
{
	MemCpy32Fn func = function_cast<MemCpy32Fn>(GetAsmCode());

	if (func == NULL) { return 1; }
	
	uint32_t src[32] = { 0xdead };
	uint32_t dst[32] = { 0xbabe };

	func(dst, src, 32);

	printf("%s\n", Compare(src, dst, 32) ? "OK" : "FAIL");

	MemoryManager::getGlobal()->free(func);

	puts("done");
	getchar();
	return 0;
}