// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "atlwfile.h"
#include <Shlobj.h>
#include "IniFile.h"

inline int WINAPI CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	PFILETIME ft1 = PFILETIME(lParam1);
	PFILETIME ft2 = PFILETIME(lParam2);
	return CompareFileTime(ft1, ft2);
}

inline bool EnsureDirExists(LPCTSTR filePath)
{ 
	TCHAR drive[_MAX_DRIVE]=_T("\0");
	TCHAR dir[_MAX_DIR]=_T("\0");
	TCHAR fname[_MAX_FNAME]=_T("\0");	
	TCHAR ext[_MAX_EXT]=_T("\0");

	if(_tsplitpath_s<_MAX_DRIVE, _MAX_DIR, _MAX_FNAME, _MAX_EXT>(filePath, drive, dir, fname, ext) != 0)
		return false;
	CString sDir(drive);
	sDir += dir;
	int retval = SHCreateDirectoryEx(NULL, sDir, NULL);	
	return (retval == ERROR_SUCCESS || retval == ERROR_FILE_EXISTS || retval == ERROR_ALREADY_EXISTS);
}


class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>, 
	public CUpdateUI<CMainFrame>,
	public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CQRestoreView m_view;

	CCommandBarCtrl m_CmdBar;

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		return m_view.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		static bool first;
		if(!first)
		{
			AddFilesFromDir(GetQuarantineDir());
			first = true;
		}
		UIUpdateToolBar();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)		
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)		
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_RESTORE_ALL, OnRestoreAll)
		COMMAND_ID_HANDLER(ID_RESTORE, OnRestore)
		COMMAND_ID_HANDLER(ID_SELECTALL, OnSelectAll)
		COMMAND_ID_HANDLER(ID_RECOVERYSCRIPT, OnRecoveryScript)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	
	void CreateProgressCtrlInStatusBar ( LPCTSTR msg, CProgressBarCtrl& wndProgressBar, int maxRange )
	{
		CStatusBarCtrl wndStatusBar = m_hWndStatusBar;
		CString sMessage = msg;
		WTL::CRect   rcBar;
		CClientDC   dc ( wndStatusBar );
		CFontHandle hFont = wndStatusBar.GetFont();
		CFontHandle hOldFont = dc.SelectFont ( hFont );
		WTL::CSize       sizeText;

		wndStatusBar.GetRect ( 0, &rcBar );

		dc.GetTextExtent ( sMessage, sMessage.GetLength(), &sizeText );
		dc.SelectFont ( hOldFont );

		rcBar.left += sizeText.cx + 10;

		wndStatusBar.SetText ( 0, sMessage );
		wndStatusBar.RedrawWindow();

		wndProgressBar.Create ( wndStatusBar, rcBar, NULL, WS_CHILD|WS_VISIBLE );
		wndProgressBar.SetRange ( 0, maxRange);
		wndProgressBar.SetPos(0);
		wndProgressBar.SetStep(1);
	}

	CString GetRestoreLogFilename()
	{
		TCHAR szTempDir[MAX_PATH], szTemp[MAX_PATH];		
		
		// Open a temporary file
		if(!GetTempPath(MAX_PATH, szTempDir))
			return _T("");
		if(!GetTempFileName(szTempDir, _T("CLAMW"), 0, szTemp))
			return _T("");	
		DeleteFile(szTemp);
		_tcscat_s(szTemp, MAX_PATH, _T(".txt"));
		return szTemp;
	}

	CString GeRecoveryScriptFilename()
	{
		TCHAR szTempDir[MAX_PATH], szTemp[MAX_PATH];		
		
		// Open a temporary file
		if(!GetTempPath(MAX_PATH, szTempDir))
			return _T("");
		if(!GetTempFileName(szTempDir, _T("QRECOVERY"), 0, szTemp))
			return _T("");	
		DeleteFile(szTemp);
		_tcscat_s(szTemp, MAX_PATH, _T(".bat"));
		return szTemp;
	}
	
	
	bool ShellOpen(LPCTSTR dstName, bool edit = false)
	{
		SHELLEXECUTEINFO info;			
		info.cbSize = sizeof(info);
		info.fMask = SEE_MASK_FLAG_DDEWAIT;
		info.hwnd = NULL;
		info.lpVerb = _T("Edit");
		info.lpFile = dstName;
		info.lpParameters = NULL;
		info.lpDirectory = NULL;
		info.nShow = SW_SHOWNORMAL;
		info.hProcess = 0;
		bool success = !!::ShellExecuteEx(&info);
        UINT_PTR result = (UINT_PTR) info.hInstApp;
		if(!success || result <= 32)			
			return false;
		return true;
	}

	CString ErrorString(DWORD err)
	{
		CString Error;
		LPTSTR s;
		if(::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			err,
			0,
			(LPTSTR)&s,
			0,
			NULL) == 0)
		{ /* failed */
			// Unknown error code %08x (%d)

			CString fmt = _T("Unknown Error");
			CString t;
			
			t.Format(fmt, err, LOWORD(err));
			Error = t;
		} /* failed */
		else
		{ /* success */
			LPTSTR p = _tcschr(s, _T('\r'));
			if(p != NULL)
			{ /* lose CRLF */
				*p = _T('\0');
			} /* lose CRLF */
			Error = s;
			::LocalFree(s);
		} /* success */
		return Error;
	} // ErrorString

	LRESULT OnRestore(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		
		CString logname = GetRestoreLogFilename();
		CFile log;
		CString logEntry = _T("Starting File Recovery\r\n");
		log.Create(logname, FILE_ALL_ACCESS);
		if(log.IsOpen())
		{
#ifdef _UNICODE
			unsigned short header = 0xFEFF;
			log.Write(PVOID(&header), 2);
#endif
			log.Write(logEntry.GetBuffer(logEntry.GetLength()), logEntry.GetLength()*sizeof(TCHAR));
		}

		int count = 0;
		for(int iItem = m_view.GetNextItem(-1, LVNI_SELECTED); iItem != -1; iItem = m_view.GetNextItem(iItem, LVNI_SELECTED))
			count++;
		CProgressBarCtrl wndProgress;
		CreateProgressCtrlInStatusBar (_T("Restoring files..."), wndProgress, count);
		m_view.SetRedraw (FALSE);

		CWaitCursor wait;

		for(int iItem = m_view.GetNextItem(-1, LVNI_SELECTED); iItem != -1; iItem = m_view.GetNextItem(iItem, LVNI_SELECTED))
        {
			CString src, dest;
			m_view.GetItemText(iItem, 0, dest);
			m_view.GetItemText(iItem, 1, src);  			
			EnsureDirExists(dest);
			CString logEntry;
			if(MoveFileEx(src, dest, MOVEFILE_COPY_ALLOWED))
			{
				DeleteFile(src + _T(".txt"));
				logEntry = dest + _T(": Successfully recovered ") + CString("\r\n");
				PFILETIME ft = (PFILETIME)m_view.GetItemData(iItem);
				if(ft)
					delete ft;				
				m_view.DeleteItem(iItem);
				iItem -= 1;
			}
			else 
				logEntry = dest + CString(" Error: ") + ErrorString(::GetLastError()) + CString("\r\n");

			if(log.IsOpen())
				log.Write(logEntry.GetBuffer(logEntry.GetLength()), logEntry.GetLength() * sizeof(TCHAR));
			
			wndProgress.StepIt();
        }

		m_view.SetRedraw ( TRUE );
		m_view.Invalidate();
		wndProgress.DestroyWindow();
		CStatusBarCtrl(m_hWndStatusBar).SetText(0, _T(""));

		log.Close();
		ShellOpen(logname);
		return 0;
		
	}

	LRESULT OnRestoreAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_view.SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
		BOOL b;
		OnRestore(0, 0, 0, b);
		return 0;
	}

	LRESULT OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        m_view.SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
        return 0;
    }

	LRESULT OnRecoveryScript(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		USES_CONVERSION;
		CString logname = GeRecoveryScriptFilename();
		CFile log;
		CString logEntry = _T("REM QRecover Quarantine File Recovery script\r\n");
		log.Create(logname, FILE_ALL_ACCESS);
		

		int count = 0;
		for(int iItem = m_view.GetNextItem(-1, LVNI_ALL); iItem != -1; iItem = m_view.GetNextItem(iItem, LVNI_BELOW))
			count++;
		CProgressBarCtrl wndProgress;
		CreateProgressCtrlInStatusBar (_T("Writing Recovery Script..."), wndProgress, count);
		m_view.SetRedraw (FALSE);

		CWaitCursor wait;
		
		for(int iItem = m_view.GetNextItem(-1, LVNI_ALL); iItem != -1; iItem = m_view.GetNextItem(iItem, LVNI_BELOW))
        {
			CString src, dest;
			m_view.GetItemText(iItem, 0, dest);
			m_view.GetItemText(iItem, 1, src);  			
			CString logEntry;
			logEntry = _T("copy /Y \"") + src + _T("\" \"") + dest + CString("\"\r\n");			
			if(log.IsOpen())
				log.Write(logEntry.GetBuffer(logEntry.GetLength()), logEntry.GetLength());
			
			wndProgress.StepIt();
        }

		m_view.SetRedraw ( TRUE );
		m_view.Invalidate();
		wndProgress.DestroyWindow();
		CStatusBarCtrl(m_hWndStatusBar).SetText(0, _T(""));

		log.Close();
		ShellOpen(logname);
		return 0;
		
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// create command bar window
		HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
		// attach menu
		m_CmdBar.AttachMenu(GetMenu());
		// load command bar images
		m_CmdBar.LoadImages(IDR_MAINFRAME);
		// remove old menu
		SetMenu(NULL);

		HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

		CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
		AddSimpleReBarBand(hWndCmdBar);
		AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

		CreateSimpleStatusBar();

		m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_REPORT | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);

		UIAddToolBar(hWndToolBar);
		UISetCheck(ID_VIEW_TOOLBAR, 1);
		UISetCheck(ID_VIEW_STATUS_BAR, 1);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);
		

		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CFolderDialog fldDlg(NULL, _T("Select a quarantine directory"),
                       BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		if (IDOK == fldDlg.DoModal())
		{

			m_view.SetRedraw ( FALSE );
			for(int iItem = m_view.GetNextItem(0, LVNI_ALL); iItem != -1; iItem = m_view.GetNextItem(iItem, LVNI_ALL))
			{
				PFILETIME ft = (PFILETIME)m_view.GetItemData(iItem);
				if(ft)
					delete ft;
			}
			m_view.DeleteAllItems();
			AddFilesFromDir(CString(fldDlg.m_szFolderPath));
			m_view.SetRedraw ( TRUE );
			m_view.Invalidate();
		}


		return 0;	
	}

	bool ParseLog(LPCTSTR filename)
	{
		const TCHAR pattern[]=_T("^{.*}: moved to '{.*}'$");
		CAtlRegExp<> regex;
		REParseError status = regex.Parse(pattern, FALSE);
		CAtlREMatchContext<> mc;
		if (REPARSE_ERROR_OK != status)
		{
			AtlMessageBox(m_hWnd, _T("Internal error: Invalid Regular Expression"), _T("QRecover"), MB_ICONSTOP|MB_OK);
			return false;
		}

		CTextFileRead log(filename);
	

		
		if(!log.IsOpen())
		{
			AtlMessageBox(m_hWnd, _T("Cannot open the log file"), _T("QRecover"), MB_ICONSTOP|MB_OK);
			return false;
		}

		m_view.DeleteAllItems();
		m_view.Invalidate();
		m_view.SetRedraw ( FALSE );

		DWORD size = GetFileSize(log.m_hFile, NULL);
		CProgressBarCtrl wndProgress;
		CreateProgressCtrlInStatusBar (_T("Processing Log file..."), wndProgress, size/100);

		CWaitCursor wait;
		tstring line;
		int i = 0;

		try{
			while(log.ReadLine(line))
			{		
				if (!regex.Match(line.c_str(), &mc))
					continue;			
		
				for (UINT nGroupIndex = 0; nGroupIndex < mc.m_uNumGroups; ++nGroupIndex)
				{
					const CAtlREMatchContext<>::RECHAR* szStart = 0;
					const CAtlREMatchContext<>::RECHAR* szEnd = 0;
					mc.GetMatch(nGroupIndex, &szStart, &szEnd);
					int nLength = (int) (szEnd - szStart);
					CString text(szStart, nLength);				
					m_view.AddItem(i, nGroupIndex, text);				
					if(nGroupIndex == mc.m_uNumGroups-1)
						i++;
				}
				DWORD pos = SetFilePointer(log.m_hFile, 0, NULL, FILE_CURRENT);								
				wndProgress.SetPos(pos/100);
			}
			
		}catch(CTextFileException)
		{
			AtlMessageBox(m_hWnd, _T("Error occured while reading the log file."), _T("QRecover"), MB_ICONSTOP|MB_OK);
			return false;
		}
		m_view.SetRedraw ( TRUE );
		m_view.Invalidate();
		wndProgress.DestroyWindow();
		CStatusBarCtrl(m_hWndStatusBar).SetText(0, _T(""));
		return true;
		
	}

	
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		static BOOL bVisible = TRUE;	// initially visible
		bVisible = !bVisible;
		CReBarCtrl rebar = m_hWndToolBar;
		int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
		rebar.ShowBand(nBandIndex, bVisible);
		UISetCheck(ID_VIEW_TOOLBAR, bVisible);
		UpdateLayout();
		return 0;
	}

	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
		::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
		UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
		UpdateLayout();
		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
    {
		if(m_view.IsWindow())
		{
			WTL::CRect rcClient(0,0,LOWORD(lParam), HIWORD(lParam));
			m_view.SetWindowPos(NULL, rcClient, SWP_NOACTIVATE | SWP_NOZORDER );
			m_view.ResizeColumns();
		}
		bHandled = FALSE;
        return 0;
    }

	CString GetQuarantineDir()
	{
		CString sPath, sConf;

		if(SHGetSpecialFolderPath(0, sConf.GetBuffer(MAX_PATH), CSIDL_APPDATA, FALSE))
		{
			sConf.ReleaseBuffer();
			if(sConf.Right(1) != _T("\\"))
				sConf += _T("\\");
			sConf += _T(".clamwin\\clamwin.conf");
			sPath = CIniFile::GetValue(_T("quarantinedir"), _T("ClamAV"), tstring(sConf)).c_str();
			if(sPath[0] == _T(' '))
				sPath = sPath.Right(sPath.GetLength() - 1);
		}   
		if(sPath.IsEmpty())
			MessageBox(_T("Could not find Quarantine directiory.\n Please use \"Open Quarantine Diirectory\" in the File menu and specify it manually"), _T("QRecover"), MB_ICONWARNING|MB_OK);
		return sPath;
	}

	

	bool AddFilesFromDir(CString& dir)
	{
		CString pattern(dir);		
		if(pattern.Right(1) != _T("\\"))
			pattern += _T("\\");
		pattern += _T("*.txt");
		int i = 0;
		CFindFile f;
		if(f.FindFile(LPCTSTR(pattern)))
		{
			do
			{
				if(f.IsDots() || f.IsDirectory())
					continue;
				CFile file;
				if(file.Open(f.GetFilePath()))
				{
					char* data = new char[(size_t)file.GetSize()];
					if(file.Read(data, (DWORD) file.GetSize()))
					{
						CString text(data, (int) file.GetSize());

						int pos = text.Find(_T('\t'));
						if(pos != -1)
						{
							CString src = text.Left(pos);
							if(src.Left(4) == _T("\\\\?\\"))
								src = src.Right(src.GetLength() - 4);
							CString dst = text.Right(text.GetLength() - pos - 1);
							if(dst.Left(4) == _T("\\\\?\\"))
								dst = dst.Right(dst.GetLength() - 4);
							m_view.AddItem(i, 0, src);	
							m_view.AddItem(i, 1, dst);	
							FILETIME *ft = new FILETIME;
							f.GetLastWriteTime(ft);
							m_view.SetItemData(i,(DWORD_PTR)ft);
							i++;
						}
					}
					delete[] data;
				}				
			}while(f.FindNextFile());
			m_view.SortItems(CompareFunc, 0);
		}

		if(!i)
		{
			CString s;
			s.Format(_T("Directory %s does not contain any quarantine information\n To open another directory use \"Open Quarantine Diirectory\" in the File menu"), (LPCTSTR)dir);
			MessageBox(s, _T("QRecover"), MB_ICONWARNING|MB_OK);
		}
		return i > 0;
	}
};


