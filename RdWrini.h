//
// Created by frank on 17-8-6.
//

#ifndef TCP_WIN_RDWRINI_H
#define TCP_WIN_RDWRINI_H

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE  1
#define MAX_PATH 260
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef unsigned long DWORD;
typedef int BOOL;
typedef int INT;
typedef unsigned int UINT;
typedef unsigned char BYTE;
#define Min(a, b) (((a)<(b))?(a):(b))

BOOL CopyFile(LPCSTR lpExistingFileName, // name of an existing file
              LPCSTR lpNewFileName,      // name of new file
              BOOL bFailIfExists         // operation if file exists
);

INT GetLine(LPSTR pLine, DWORD dwOffset, DWORD dwSize);

BOOL IsComment(LPCSTR pLine);

BOOL IsSection(LPCSTR pLine);

BOOL IsSectionName(LPCSTR pLine, LPCSTR pSection);

BOOL IsKey(LPSTR pLine, LPCSTR pKeyName, LPSTR *pValue, DWORD *dwValLen);

DWORD GetString(LPCSTR lpAppName, LPCSTR lpKeyName,
                LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName);

INT ReadIniFile(LPCSTR lpFileName);

DWORD GetPrivateProfileString(
        LPCSTR lpAppName,
        LPCSTR lpKeyName,
        LPCSTR lpDefault,
        LPSTR lpReturnedString,
        DWORD Size,
        LPCSTR lpFileName);

UINT GetPrivateProfileInt(
        LPCSTR lpAppName,
        LPCSTR lpKeyName,
        INT nDefault,
        LPCSTR lpFileName);

void WriteLine(INT hOutput, LPCSTR pLine);

BOOL WritePrivateProfileString(
        LPCSTR lpAppName,
        LPCSTR lpKeyName,
        LPCSTR lpString,
        LPCSTR lpFileName);

BOOL WritePrivateProfileInt(
        LPCSTR lpAppName,
        LPCSTR lpKeyName,
        INT Value,
        LPCSTR lpFileName);

void WriteValue(INT fdOutput, LPCSTR pAppName, LPCSTR pKeyName, LPCSTR pString);

extern LPSTR g_pData;

#endif //TCP_WIN_RDWRINI_H
