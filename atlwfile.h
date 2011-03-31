#ifndef __ATLWFILE_H__
#define __ATLWFILE_H__

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Windows File API wrappers
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2001 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// source file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//


#ifndef __cplusplus
   #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef INVALID_SET_FILE_POINTER
   #define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif // INVALID_SET_FILE_POINTER

#ifndef _ASSERTE
   #define _ASSERTE(x)
#endif

#ifndef _ATL_DLL_IMPL
namespace ATL
{
#endif


/////////////////////////////////////////////////////////////////
// Standard file wrapper

// Win32 File wrapper class
// Important: Don't make the destructor "virtual" because we need
//            the class v-table to look like the HANDLE type!
// NOTE: Win 95/98 only supports < 4Gb files; see Q250301

template< bool t_bManaged >
class CFileT
{
public:
   HANDLE m_hFile;

   CFileT(HANDLE hFile = INVALID_HANDLE_VALUE) 
   {
      m_hFile = hFile;
   }
   CFileT(const CFileT<t_bManaged>& file) 
   {
      m_hFile = INVALID_HANDLE_VALUE;
      DuplicateHandle(file.m_hFile);
   }
   ~CFileT()
   { 
      if( t_bManaged ) Close(); 
   }
   operator HFILE() const 
   { 
      return (HFILE) m_hFile; 
   }
   operator HANDLE() const 
   { 
      return m_hFile; 
   }
   const CFileT<t_bManaged>& operator=(const CFileT<t_bManaged>& file)
   {
      DuplicateHandle(file.m_hFile);
      return *this;
   }
   BOOL Open(LPCTSTR pstrFileName, 
             DWORD dwAccess = GENERIC_READ, 
             DWORD dwShareMode = FILE_SHARE_READ,   
             LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL,
             DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL)
   {      
      return Create(pstrFileName, dwAccess, dwShareMode, lpSecurityAttributes, OPEN_EXISTING, dwFlagsAndAttributes);
   }
   BOOL Create(LPCTSTR pstrFileName,
               DWORD dwAccess = GENERIC_WRITE, 
               DWORD dwShareMode = 0 /*DENY ALL*/,  
               LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL,
               DWORD dwCreationDisposition = CREATE_ALWAYS,
               DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL)
   {
       _ASSERTE(!::IsBadStringPtr(pstrFileName,static_cast<UINT_PTR>(-1)));
      Close();
      // Attempt file creation
      HANDLE hFile = ::CreateFile(pstrFileName, 
         dwAccess, 
         dwShareMode, 
         lpSecurityAttributes,
         dwCreationDisposition, 
         dwFlagsAndAttributes, 
         NULL);
      if( hFile == INVALID_HANDLE_VALUE ) return FALSE;
      m_hFile = hFile;
      return TRUE;
   }
   void Close()
   {
      if( m_hFile == INVALID_HANDLE_VALUE ) return;
      ::CloseHandle(m_hFile);
      m_hFile = INVALID_HANDLE_VALUE;
   }
   BOOL IsOpen() const
   {
      return m_hFile != INVALID_HANDLE_VALUE;
   }
   void Attach(HANDLE hHandle)
   {
      Close();
      m_hFile = hHandle;
   }   
   HANDLE Detach()
   {
      HANDLE h = m_hFile;
      m_hFile = INVALID_HANDLE_VALUE;
      return h;
   }
   BOOL Read(LPVOID lpBuf, DWORD nCount)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      _ASSERTE(lpBuf!=NULL);
      _ASSERTE(!::IsBadWritePtr(lpBuf, nCount));
      if( nCount == 0 ) return TRUE;   // avoid Win32 "null-read"
      DWORD dwRead = 0;
      if( !::ReadFile(m_hFile, lpBuf, nCount, &dwRead, NULL) ) return FALSE;
      return TRUE;
   }
   BOOL Read(LPVOID lpBuf, DWORD nCount, LPDWORD pdwRead)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      _ASSERTE(lpBuf);
      _ASSERTE(!::IsBadWritePtr(lpBuf, nCount));
      _ASSERTE(pdwRead);
      *pdwRead = 0;
      if( nCount == 0 ) return TRUE;   // avoid Win32 "null-read"
      if( !::ReadFile(m_hFile, lpBuf, nCount, pdwRead, NULL) ) return FALSE;
      return TRUE;
   }
   BOOL Write(LPCVOID lpBuf, DWORD nCount)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      _ASSERTE(lpBuf!=NULL);
      _ASSERTE(!::IsBadReadPtr(lpBuf, nCount));   
      if( nCount == 0 ) return TRUE; // avoid Win32 "null-write" option
      DWORD dwWritten = 0;
      if( !::WriteFile(m_hFile, lpBuf, nCount, &dwWritten, NULL) ) return FALSE;
      return TRUE;
   }
   BOOL Write(LPCVOID lpBuf, DWORD nCount, LPDWORD pdwWritten)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      _ASSERTE(lpBuf);
      _ASSERTE(!::IsBadReadPtr(lpBuf, nCount));
      _ASSERTE(pdwWritten);    
      *pdwWritten = 0;
      if( nCount == 0 ) return TRUE; // avoid Win32 "null-write" option
      if( !::WriteFile(m_hFile, lpBuf, nCount, pdwWritten, NULL) ) return FALSE;
      return TRUE;
   }
   __int64 Seek(__int64 lOff, UINT nFrom)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      LARGE_INTEGER liOff, liPos;
      liOff.QuadPart = lOff;
      if(!::SetFilePointerEx(m_hFile, liOff, &liPos, (DWORD) nFrom))
          return (DWORD) -1;      
      return liPos.QuadPart;
   }

   __int64 GetPosition() const
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return Seek(0, FILE_CURRENT);      
   }
   BOOL Lock(DWORD dwOffset, DWORD dwSize)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::LockFile(m_hFile, dwOffset, 0, dwSize, 0);
   }
   BOOL Unlock(DWORD dwOffset, DWORD dwSize)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::UnlockFile(m_hFile, dwOffset, 0, dwSize, 0);
   }
   BOOL SetEOF()
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::SetEndOfFile(m_hFile);
   }
   BOOL Flush()
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::FlushFileBuffers(m_hFile);
   }
   __int64 GetSize() const
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      LARGE_INTEGER liSize;
      if(!::GetFileSizeEx(m_hFile, &liSize))
          return __int64(-1);
      return liSize.QuadPart;
   }
   DWORD GetType() const
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::GetFileType(m_hFile);
   }
   BOOL GetFileTime(FILETIME* ftCreate, FILETIME* ftAccess, FILETIME* ftModified)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::GetFileTime(m_hFile, ftCreate, ftAccess, ftModified);
   }
   BOOL SetFileTime(FILETIME* ftCreate, FILETIME* ftAccess, FILETIME* ftModified)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::SetFileTime(m_hFile, ftCreate, ftAccess, ftModified);
   }
   BOOL DuplicateHandle(HANDLE hOther)
   {
      ATLASSERT(m_hFile==INVALID_HANDLE_VALUE);
      ATLASSERT(hOther!=INVALID_HANDLE_VALUE);
      HANDLE process = ::GetCurrentProcess();
      BOOL res = ::DuplicateHandle(process, hOther, process, &m_hFile, NULL, FALSE, DUPLICATE_SAME_ACCESS);
      _ASSERTE(res);
      return res;
   }
   static BOOL FileExists(LPCTSTR pstrFileName)
   {
      _ASSERTE(!::IsBadStringPtr(pstrFileName, MAX_PATH));
      DWORD dwErrMode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
      BOOL bRes = ::GetFileAttributes(pstrFileName) != 0xFFFFFFFF;
      ::SetErrorMode(dwErrMode);
      return bRes;
   }
   static BOOL Delete(LPCTSTR pstrFileName)
   {
      _ASSERTE(!::IsBadStringPtr(pstrFileName, MAX_PATH));
      return ::DeleteFile(pstrFileName);
   }
   static BOOL Rename(LPCTSTR pstrSourceFileName, LPCTSTR pstrTargetFileName)
   {
      _ASSERTE(!::IsBadStringPtr(pstrSourceFileName, MAX_PATH));
      _ASSERTE(!::IsBadStringPtr(pstrTargetFileName, MAX_PATH));
      return ::MoveFile(pstrSourceFileName, pstrTargetFileName);
   }
};

typedef CFileT<true> CFile;
typedef CFileT<false> CFileHandle;


/////////////////////////////////////////////////////////////////
// Temporary file (temp filename and auto delete)

class CTemporaryFile : public CFileT<true>
{
public:
   TCHAR m_szFileName[MAX_PATH];

   ~CTemporaryFile()
   { 
      Close();
      Delete(m_szFileName);
   }
   BOOL Create(LPTSTR pstrFileName, 
               UINT cchFilename,
               DWORD dwAccess = GENERIC_WRITE, 
               DWORD dwShareMode = 0 /*DENY ALL*/,                
               DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL)
   {
      _ASSERTE(!::IsBadStringPtr(pstrFileName,cchFilename));
      // If a valid filename buffer is supplied we'll create
      // and return a new temporary filename.
      if( cchFilename > 0 ) {
         ::GetTempPath(cchFilename, pstrFileName);
         ::GetTempFileName(pstrFileName, _T("BV"), 0, pstrFileName);
      }
      ::lstrcpy(m_szFileName, pstrFileName);
      return Create(pstrFileName, dwAccess, dwShareMode, CREATE_ALWAYS, dwFlagsAndAttributes);
   }
};


#ifndef _ATL_DLL_IMPL
}; //namespace ATL
#endif


#endif // __ATLWFILE_H___
