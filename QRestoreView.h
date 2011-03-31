// QRestoreView.h : interface of the CQRestoreView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CQRestoreView : public CCheckListViewCtrlImpl<CQRestoreView>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CCheckListViewCtrl::GetWndClassName())


	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	BEGIN_MSG_MAP(CQRestoreView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu) 
		COMMAND_ID_HANDLER(ID_SELECTALL, PassToParent)
		COMMAND_ID_HANDLER(ID_RESTORE_ALL, PassToParent)
		COMMAND_ID_HANDLER(ID_RESTORE, PassToParent)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	void ResizeColumns()
	{
		WTL::CRect rcClient;
		GetClientRect(&rcClient);
		int w=rcClient.Width()-::GetSystemMetrics(SM_CXVSCROLL)-2;
		if (w>400)
		{
			int columns[]={0,  w/2 };
			columns[0]=w-columns[1];
			for (int i=0;i<2;++i)
				SetColumnWidth(i,columns[i]);			
		}
	}

	LRESULT PassToParent(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(wID, wNotifyCode), reinterpret_cast<LPARAM>(hWndCtl));
		bHandled = TRUE;
		return 0;
	}

	

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

		// Add the headers for the ListView control
		InsertColumn(0,  _T("Original location"), LVCFMT_LEFT, 400, 1);
		InsertColumn(1, _T("Quarantined file"), LVCFMT_LEFT, 400, 1);      
		

		SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);		
		bHandled = FALSE;
		return lRet;
	}

	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
    {
     
        CMenu menu;
        menu.LoadMenu(IDR_CONTEXT);
        CMenuHandle submenu = menu.GetSubMenu(0);
        submenu.SetMenuDefaultItem(ID_RESTORE);
		

        WTL::CPoint pt(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));        
		if (pt.x==-1)
		{
			for(int iItem = GetNextItem(-1, LVNI_FOCUSED); iItem != -1; iItem = GetNextItem(iItem, LVNI_FOCUSED)) 
			{
				WTL::CRect rc;
				GetItemRect(iItem,&rc,LVIR_LABEL);
				ClientToScreen(&rc);
				pt.x=rc.left+5;
				pt.y=rc.bottom;
				break;
			}
		}
		if (pt.x!=-1)
			submenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN |	TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, NULL);        
        return 0;
    }

};


