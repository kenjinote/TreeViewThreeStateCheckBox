#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32")

#include <windows.h>
#include <commctrl.h>
#include "resource.h"

TCHAR szClassName[] = TEXT("Window");
WNDPROC DefaultTreeViewProc;

LRESULT CALLBACK TreeViewProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_F2:
			SendMessage(GetParent(hWnd), WM_COMMAND, 1002, 0);
			break;
		case VK_DELETE:
			SendMessage(GetParent(hWnd), WM_COMMAND, 1003, 0);
			break;
		}
		break;
	}
	return CallWindowProc(DefaultTreeViewProc, hWnd, msg, wParam, lParam);
}

VOID TreeView_SetCheckChild(HWND hTree, HTREEITEM hItem, BOOL bCheck)
{
	hItem = TreeView_GetChild(hTree, hItem);
	while (hItem)
	{
		TreeView_SetCheckState(hTree, hItem, bCheck);
		TreeView_SetCheckChild(hTree, hItem, bCheck);
		hItem = TreeView_GetNextItem(hTree, hItem, TVGN_NEXT);
	}
}

VOID TreeView_SetCheckParent(HWND hTree, HTREEITEM hItem)
{
	hItem = TreeView_GetParent(hTree, hItem);
	while (hItem)
	{
		int nState = 0;
		{
			// 子要素チェック状態を確認して親のチェック状態をどうすべきか判定する
			HTREEITEM hChild = TreeView_GetChild(hTree, hItem);
			BOOL bCheckOff = FALSE;
			BOOL bCheckOn = FALSE;
			while (hChild)
			{
				DWORD dwState = TreeView_GetItemState(hTree, hChild, TVIS_USERMASK) & TVIS_USERMASK;
				switch (TreeView_GetItemState(hTree, hChild, TVIS_USERMASK) & TVIS_USERMASK)
				{
				case 0x1000:// OFF
					if (bCheckOn)
					{
						nState = 3;
						goto LOOPEND;
					}
					else
					{
						bCheckOff = TRUE;
						nState = 1;
					}
					break;
				case 0x2000: // ON
					if (bCheckOff)
					{
						nState = 3;
						goto LOOPEND;
					}
					else
					{
						bCheckOn = TRUE;
						nState = 2;
					}
					break;
				case 0x3000: // どちらでもない
					nState = 3;
					goto LOOPEND;
					break;
				}
				hChild = TreeView_GetNextItem(hTree, hChild, TVGN_NEXT);
			}
		LOOPEND:
			TreeView_SetItemState(hTree, hItem, INDEXTOSTATEIMAGEMASK(nState), TVIS_USERMASK);
		}
		hItem = TreeView_GetNextItem(hTree, hItem, TVGN_PARENT);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hTree;
	static HWND hButton1;
	static HWND hButton2;
	static HWND hButton3;
	static HWND hButton4;
	static BOOL bReEnter;
	static HIMAGELIST hImageList;
	switch (msg)
	{
	case WM_CREATE:
		InitCommonControls();
		hImageList = ImageList_Create(13, 13, ILC_COLOR | ILC_MASK, 0, 0);
		{
			HBITMAP hBitmap = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, (LPCTSTR)IDB_BITMAP1);
			ImageList_Add(hImageList, hBitmap, NULL);
			DeleteObject(hBitmap);
		}
		hButton1 = CreateWindow(TEXT("BUTTON"), TEXT("ルートノードを追加"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)1000, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton2 = CreateWindow(TEXT("BUTTON"), TEXT("子ノードを追加"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)1001, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton3 = CreateWindow(TEXT("BUTTON"), TEXT("名前の変更(F2)"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)1002, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton4 = CreateWindow(TEXT("BUTTON"), TEXT("削除(Delete)"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)1003, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hTree = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			WC_TREEVIEW,
			0,
			WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_EDITLABELS | TVS_CHECKBOXES | TVS_SHOWSELALWAYS,
			10, 10, 200, 300,
			hWnd,
			(HMENU)2000,
			((LPCREATESTRUCT)lParam)->hInstance,
			0);
		TreeView_SetImageList(hTree, hImageList, TVSIL_STATE);
		DefaultTreeViewProc = (WNDPROC)GetWindowLongPtr(hTree, GWLP_WNDPROC);
		SetWindowLongPtr(hTree, GWLP_WNDPROC, (LONG_PTR)TreeViewProc);
		break;
	case WM_SIZE:
		MoveWindow(hButton1, 0, 0, 256, 32, TRUE);
		MoveWindow(hButton2, 256, 0, 256, 32, TRUE);
		MoveWindow(hButton3, 512, 0, 256, 32, TRUE);
		MoveWindow(hButton4, 768, 0, 256, 32, TRUE);
		MoveWindow(hTree, 0, 32, LOWORD(lParam), HIWORD(lParam) - 32, TRUE);
		break;
	case WM_NOTIFY:
		if (wParam == 2000)
		{
			LPNMTVDISPINFO pDispInfo = (LPNMTVDISPINFO)lParam;
			if (pDispInfo->hdr.code == TVN_ENDLABELEDIT)
			{
				TreeView_SetItem(hTree, &pDispInfo->item);
			}
			else if (pDispInfo->hdr.code == TVN_ITEMCHANGED)
			{
				NMTVITEMCHANGE* tvItemChange = (NMTVITEMCHANGE*)(&(pDispInfo->hdr));
				if ((TVIS_USERMASK & tvItemChange->uStateOld) != (TVIS_USERMASK & tvItemChange->uStateNew))
				{
					if (!bReEnter)
					{
						bReEnter = TRUE;
						switch (TVIS_USERMASK & tvItemChange->uStateNew)
						{
						case 0x1000: // チェック OFF にした
							TreeView_SetCheckChild(hTree, tvItemChange->hItem, FALSE);
							break;
						case 0x2000: // チェック ON にした
							TreeView_SetCheckChild(hTree, tvItemChange->hItem, TRUE);
							break;
						case 0x3000: // ON / OFF どちらでもない状態にした → OFF にしたと解釈する
							TreeView_SetItemState(hTree, tvItemChange->hItem, INDEXTOSTATEIMAGEMASK(1), TVIS_USERMASK);
							TreeView_SetCheckChild(hTree, tvItemChange->hItem, FALSE);
							break;
						}
						TreeView_SetCheckParent(hTree, tvItemChange->hItem);
						bReEnter = FALSE;
					}
				}
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 1000:
		{
			// 親ノードを追加する
			SetFocus(hTree);
			TV_INSERTSTRUCT tv = { 0 };
			tv.hInsertAfter = TVI_LAST;
			tv.item.mask = TVIF_TEXT;
			tv.hParent = TVI_ROOT;
			tv.item.pszText = TEXT("Node0");
			HTREEITEM hParent = TreeView_InsertItem(hTree, &tv);
			TreeView_Select(hTree, hParent, TVGN_CARET);
		}
		break;
		case 1001:
		{
			// 子ノードを追加して選択状態にする
			SendMessage(hTree, WM_SETREDRAW, (WPARAM)FALSE, 0); // 再描画を止めるとSelectItemによるスクロールが止められる
			SetFocus(hTree);
			HTREEITEM hParent = TreeView_GetSelection(hTree);
			TV_INSERTSTRUCT tv = { 0 };
			tv.hInsertAfter = TVI_LAST;
			tv.item.mask = TVIF_TEXT;
			tv.hParent = hParent;
			tv.item.pszText = TEXT("Node0");
			HTREEITEM hItem = TreeView_InsertItem(hTree, &tv);
			if (!bReEnter)
			{
				bReEnter = TRUE;
				TreeView_SetCheckParent(hTree, hItem);
				bReEnter = FALSE;
			}
			TreeView_Select(hTree, hItem, TVGN_CARET);
			SendMessage(hTree, WM_SETREDRAW, (WPARAM)TRUE, 0);
		}
		break;
		case 1002:
		{
			SetFocus(hTree);
			HTREEITEM hTreeItem = TreeView_GetSelection(hTree);
			if (hTreeItem)
			{
				TreeView_EditLabel(hTree, hTreeItem);
			}
		}
		break;
		case 1003:
		{
			SetFocus(hTree);
			HTREEITEM hTreeItem = TreeView_GetSelection(hTree);
			if (hTreeItem && MessageBox(hWnd, TEXT("選択されたアイテムを削除しますか?"), TEXT("確認"), MB_YESNO) == IDYES)
			{
				HTREEITEM hParent = TreeView_GetParent(hTree, hTreeItem);
				TreeView_DeleteItem(hTree, hTreeItem);
				if (hParent)
				{
					HTREEITEM hChild = TreeView_GetChild(hTree, hParent);
					if (hChild)
					{
						if (!bReEnter)
						{
							bReEnter = TRUE;
							TreeView_SetCheckParent(hTree, hChild);
							bReEnter = FALSE;
						}
					}
				}
			}
		}
		break;
		}
		break;
	case WM_DESTROY:
		ImageList_Destroy(hImageList);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
