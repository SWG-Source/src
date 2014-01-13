// ======================================================================
//
// OsNewDel.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedMemoryManager/FirstSharedMemoryManager.h"
#include "sharedMemoryManager/OsNewDel.h"

// ======================================================================

// this is here because MSVC won't call MemoryManager::allocate() from asm directly
static void * __cdecl localAllocate(size_t size, uint32 owner, bool array, bool leakTest)
{
	return MemoryManager::allocate(size, owner, array, leakTest);
}

// ----------------------------------------------------------------------

// We are using the arguments (except for file and line), but MSVC can't tell that.
#pragma warning(disable: 4100)

// ----------------------------------------------------------------------

__declspec(naked) void *operator new(size_t size, MemoryManagerNotALeak)
{
	_asm
	{
		// setup local call stack
		push    ebp
		mov     ebp, esp

		// MemoryManager::alloc(size, [return address], false, false)
		push    0
		push    0
		mov     eax, dword ptr [ebp+4]
		push    eax
		mov     eax, dword ptr [ebp+8]
		push    eax
		call    localAllocate
		add     esp, 12

		mov     esp, ebp
		pop     ebp
		ret
	}
}

// ----------------------------------------------------------------------

__declspec(naked) void *operator new(size_t size)
{
	_asm
	{
		// setup local call stack
		push    ebp
		mov     ebp, esp

		// MemoryManager::alloc(size, [return address], false, true)
		push    1
		push    0
		mov     eax, dword ptr [ebp+4]
		push    eax
		mov     eax, dword ptr [ebp+8]
		push    eax
		call    localAllocate
		add     esp, 12

		mov     esp, ebp
		pop     ebp
		ret
	}
}

// ----------------------------------------------------------------------

__declspec(naked) void *operator new[](size_t size)
{
	_asm
	{
		// setup local call stack
		push    ebp
		mov     ebp, esp

		// MemoryManager::alloc(size, [return address], true, true)
		push    1
		push    1
		mov     eax, dword ptr [ebp+4]
		push    eax
		mov     eax, dword ptr [ebp+8]
		push    eax
		call    localAllocate
		add     esp, 12

		mov     esp, ebp
		pop     ebp
		ret
	}
}

// ----------------------------------------------------------------------

__declspec(naked) void *operator new(size_t size, const char *file, int line)
{
	_asm
	{
		// setup local call stack
		push    ebp
		mov     ebp, esp

		// MemoryManager::alloc(size, [return address], false, true)
		push    1
		push    0
		mov     eax, dword ptr [ebp+4]
		push    eax
		mov     eax, dword ptr [ebp+8]
		push    eax
		call    localAllocate
		add     esp, 12

		mov     esp, ebp
		pop     ebp
		ret
	}
}

// ----------------------------------------------------------------------

__declspec(naked) void *operator new[](size_t size, const char *file, int line)
{
	_asm
	{
		// setup local call stack
		push    ebp
		mov     ebp, esp

		// MemoryManager::alloc(size, [return address], true, true)
		push    1
		push    1
		mov     eax, dword ptr [ebp+4]
		push    eax
		mov     eax, dword ptr [ebp+8]
		push    eax
		call    localAllocate
		add     esp, 12

		mov     esp, ebp
		pop     ebp
		ret
	}
}

// ----------------------------------------------------------------------

#pragma warning(default: 4100)

// ----------------------------------------------------------------------

void operator delete(void *pointer)
{
	if (pointer)
		MemoryManager::free(pointer, false);
}

// ----------------------------------------------------------------------

void operator delete[](void *pointer)
{
	if (pointer)
		MemoryManager::free(pointer, true);
}

// ----------------------------------------------------------------------

void operator delete(void *pointer, const char *file, int line)
{
	UNREF(file);
	UNREF(line);

	if (pointer)
		MemoryManager::free(pointer, false);
}

// ----------------------------------------------------------------------

void operator delete[](void *pointer, const char *file, int line)
{
	UNREF(file);
	UNREF(line);

	if (pointer)
		MemoryManager::free(pointer, true);
}

// ======================================================================
// WARNING!!!!!!!
// 
// The init_seg pragma command is used to create certain static objects first, before other static objects are created.
// However, multiple static variables that use the same init_seg category(i.e. compiler) are not guaranteed to destroy in any 
// particular order. It is completely random based on how all the linking of static objects occurs. Since this command is being
// used on our memory manager(to overwrite new/delete) - NO OTHER STATIC SHOULD EVER USE INIT_SEG(COMPILER)!!!! This static object
// *MUST* be the final static object that is destroyed.
//
#pragma warning(disable: 4074)
#pragma init_seg(compiler) // ^-Read warning above.-^
static MemoryManager memoryManager;

// ======================================================================

