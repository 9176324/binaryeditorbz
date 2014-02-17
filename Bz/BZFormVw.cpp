/*
licenced by New BSD License

Copyright (c) 1996-2013, c.mos(original author) & devil.tamachan@gmail.com(Modder)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"
#include "BZ.h"
#include "BZDoc2.h"
#include "BZView.h"
#include "BZFormVw.h"


#define g_nTypes 15

char* g_datatypes[g_nTypes] = {
	"char", "byte", "BYTE", "short", "word", "WORD", "long", "dword", "DWORD", "double", "qword", "QWORD", "float", "int64","int"
};

DWORD g_datasizes[g_nTypes] = {1,1,1,2,2,2,4,4,4,8,8,8 ,4,8,4
};

TCHAR *s_MemberColLabel[MBRCOL_MAX] = { _T("+"), _T("Label"), _T("Value") };



void CBZFormView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
  {
		if(nChar == VK_RETURN) {
      HWND hWndFocus = GetFocus();
			if(hWndFocus == m_listTag.m_hWnd)
				m_listMember.SetFocus();
			else if(hWndFocus == m_listMember.m_hWnd) {
				m_pView->Activate();
        PostMessage2MainFrame(WM_COMMAND, ID_EDIT_VALUE);
			}
			return;
		}
		if(nChar == VK_TAB && (::GetKeyState(VK_SHIFT) < 0)) {
			m_pView->Activate();
			return;
		}
    SetMsgHandled(FALSE);
  }
void CBZFormView::SelchangeListTag()
  {
    if(m_listTag.GetCount()==0)return;

    int iItem = m_listTag.GetCurSel();
    if(iItem != LB_ERR) {
      int iTag = m_listTag.GetItemData(iItem);
      m_pView->m_dwStructTag = m_pView->m_dwCaret;
      m_pView->m_dwStruct = m_pView->m_dwCaret + m_tag[iTag].m_len;
      m_pView->m_nMember = INVALID;
      InitListMember(iTag);
      m_pView->Invalidate(FALSE);
    }
  }

LRESULT CBZFormView::OnDblclkListMember(LPNMHDR pnmh)
  {
    LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)pnmh;

    if(pnmia->iItem == m_listMember.GetItemCount() - 1) {
      int iItem = m_listTag.GetCurSel();
      int iTag = m_listTag.GetItemData(iItem);
      m_pView->m_dwCaret = m_pView->m_dwStructTag + m_tag[iTag].m_len;
      if(m_listTag.GetCount() > 1 && m_nTagSelect != -1) {
        m_listTag.SetCurSel(iItem + 1);
      }
      SelchangeListTag();
    } else {
      m_pView->Activate();
      PostMessage2MainFrame(WM_COMMAND, ID_EDIT_VALUE);
    }
    return 0;
  }

LRESULT CBZFormView::OnItemchangedListMember(LPNMHDR pnmh)
  {
    LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pnmh;

    if(pnmv->uNewState == (LVIS_FOCUSED | LVIS_SELECTED)) {
      int iTag = m_listTag.GetItemData(m_listTag.GetCurSel());
      if(iTag >= 0) {	// ### 1.62
        CStructMember& m = m_tag[iTag].m_member[pnmv->iItem];
        m_pView->m_nMember = m.m_ofs;
        m_pView->m_dwCaret = m_pView->m_dwStructTag + m.m_ofs;
        m_pView->m_nBytes = m.m_bytes ? m.m_bytes : 1;
        m_pView->m_nBytesLength = (m.m_bytes && (m.m_bytes != m.m_len)) ? m.m_len / m.m_bytes : 1;
        m_pView->GotoCaret();
        m_pView->Invalidate(FALSE);
        m_pView->UpdateStatusInfo();
      }
      //	m_pView->Activate();
    }	
    return 0;
  }


void RemoveCommentAll(char *src)
{
	for(; *src!='\0'; src++)
	{
		if(*src=='/')
		{
			if(*(src+1)=='/')
			{
				*src = *(src+1) = ' ';
				src+=2;
				for(; *src!='\r' && *src!='\n'; src++)
				{
					if(*src=='\0')return;
					*src=' ';
				}
			} else if(*(src+1)=='*') {
				*src = *(src+1) = ' ';
				src+=2;
				for(; *src!='*' || (*src!='\0' && *(src+1)!='/'); src++)
				{
					if(*src=='\0')return;
					*src=' ';
				}
				if(*src=='\0')return;
				*src = *(src+1) = ' ';
			}
		}
	}
}

HRESULT CBZFormView::ParseMember(CStringA& member, int iTag, int iType, CString& errMsg)
{
	//     m2[43]  
	const char *seps = "[ \t\r\n";
	int curPos4 = 0;
	CStringA memberName = member.Tokenize(seps, curPos4);
	if(memberName==_T(""))
	{
		errMsg.Format(_T("member name not found (in %dth struct"), iTag);
		return E_FAIL;
	}
	int iMember = m_tag[iTag].m_member.Add(CStructMember(memberName, iType));
	if(curPos4==-1) return S_OK; //name only(not array)
	CStringA arrayNum = member.Tokenize(seps, curPos4).TrimRight("]");
	if(arrayNum!=_T(""))
	{
		if(atoi(arrayNum)==0)
		{
			errMsg.Format(_T("Wrong array: %s"), member);
			return E_FAIL;//wrong number or member[0]
		}
		m_tag[iTag].m_member[iMember].m_len *= atoi(arrayNum);
	}
	/*if(curPos4==-1)*/ return S_OK; //with array

	// まだ要素があるのはおかしい
/*	errMsg.Format(_T("too many arg: %s"), member);
	return E_FAIL;*/
}
int SearchType(const char *type)
{
	int i222;
	for(i222=0; i222<g_nTypes; i222++)
		if(!strcmp(type, g_datatypes[i222])) break;
	return i222;
}
HRESULT CBZFormView::ParseMemberLine(CStringA& memberline, int iTag, CString& errMsg)
{
	//    DWORD m1,m2[43],m424
	ATLTRACE("memberline %s(%d)\n", memberline,memberline.GetLength());
	int curPos2 = 0;
	CStringA type = memberline.Tokenize(" \t\r\n", curPos2);
	CStringA members = memberline.Right(memberline.GetLength() - type.GetLength()).Trim(" \t\r\n");

	int iType = SearchType(type);
	if(iType == g_nTypes || type.GetLength()==0 || members.GetLength()==0)
	{
		errMsg.Format(_T("Wrong line: %s"), memberline);
		return E_FAIL;
	}

	int curPos3 = 0;
	CStringA member;
	while((member = members.Tokenize(",", curPos3))!="")
	{
		//     m2[43]  
		if(FAILED(ParseMember(member, iTag, iType, errMsg)))
			return E_FAIL;
	}
	return S_OK;
}

BOOL CBZFormView::InitStructList() 
{
	CString errMsg;

	RemoveCommentAll(m_pDefFile);
	ATLTRACE("BZ.def: %s\n", m_pDefFile);
	char *p = m_pDefFile;
	const char seps[] = " ;[]\t\r\n\032";
	enum TokeMode { TK_STRUCT, TK_TAG, TK_BEGIN } mode;
	mode = TK_STRUCT;
	int iTag = 0;
	int type = 0;

	m_tag.RemoveAll();
	while(p = (char*)_mbstok((UCHAR*)p, (const UCHAR*)seps)) {
		switch (mode) {
		case TK_STRUCT:
			if (strcmp(p, "struct")) goto ErrorP;
			mode = TK_TAG;
			p = NULL;
			break;
		case TK_TAG:
			if (!isalpha(*p)) goto ErrorP;
			iTag = m_tag.Add(CStructTag(p));
			mode = TK_BEGIN;
			p = NULL;
			break;
		case TK_BEGIN:
			{
				if (*p != '{')
				{
					errMsg = _T("'{' not found. struct AAA { <-------");
					goto Error;
				}
				p+=2;
				char *pEnd = (char *)_mbsstr((UCHAR*)p, (UCHAR*)"}");
				if(pEnd==NULL)
				{
					errMsg = _T("'}' not found");
					goto Error;
				}
				CStringA members;
				members.SetString(p, pEnd - p);
				CStringA memberline;
				int curPos=0;
				while((memberline = members.Tokenize(";", curPos).Trim(" \t\r\n"))!=_T(""))
				{
					//    DWORD m1,m2[43],m424
					if(FAILED(ParseMemberLine(memberline, iTag, errMsg)))goto Error;
				}
				m_tag[iTag].m_member.Add(CStructMember(" <next>", -1));
				p=pEnd+1;
				char *pExtEnd = (char*)_mbsstr((UCHAR*)p, (UCHAR*)";");
				if(pExtEnd==NULL)
				{
					errMsg = _T("';' not found. struct AA{...}; <----");
					goto Error;
				}
				CStringA extall;
				extall.SetString(p, pExtEnd-p);
				int curPos11=0;
				CString ext;
				while((ext = extall.Tokenize(",", curPos11).Trim(" \t\r\n"))!=_T(""))
				{
					m_tag[iTag].m_sarrFileExt.Add(ext);
				}
				p = pExtEnd+1;
				mode = TK_STRUCT;
				break;
			}
		}
	}
	return TRUE;
ErrorP:
	errMsg = p;
Error:
  CString mes;
  mes.LoadString(IDS_ERR_SYNTAX);
  CString msgFormat;
  msgFormat.Format(mes, errMsg);
  MessageBox(msgFormat, _T("Error"), MB_OK);
	return FALSE;	
}

void CBZFormView::InitListTag()
{
	for_to(i, m_tag.GetCount()) {
		int len = 0;
		for_to(j, m_tag[i].m_member.GetCount()) {
			CStructMember& member = m_tag[i].m_member[j];
			member.m_ofs = len;
			len += member.m_len;
		}
		m_tag[i].m_len = len;

	}
	m_listTag.ResetContent();

  DoDataExchange(DDX_SAVE);
	if(!m_bTagAll) {
		CString sExt;
		CBZDoc2* pDoc = GetBZDoc2();
		if(pDoc) {
			CString sPath = pDoc->GetFilePath();
			int nExt = sPath.ReverseFind(TCHAR('.'));
			if(nExt >= 0) {
				sExt = sPath.Mid(nExt + 1);
			}
		}

		m_nTagSelect = -1;
		if(!sExt.IsEmpty()) {
			for_to_(i, m_tag.GetCount()) {
				CStructTag& tag = m_tag[i];
				if(tag.m_sarrFileExt.GetCount() == 0)
					AddTag(i);
				else {
					for_to(j, tag.m_sarrFileExt.GetCount()) {
						if(sExt.CompareNoCase(tag.m_sarrFileExt[j]) == 0) {
							int nItem = AddTag(i);
							if(m_nTagSelect == -1)
								m_nTagSelect = nItem;
						}
					}
				}
			}
		}
	}
	if(m_listTag.GetCount() == 0) {
		for_to_(i, m_tag.GetCount()) {
			AddTag(i);
		}
	}
}

int CBZFormView::AddTag(int iTag)
{
	int iItem = m_listTag.AddString(m_tag[iTag].m_name);
	m_listTag.SetItemData(iItem, iTag);
	return iItem;
}

void CBZFormView::SelectTag()
{
	if(m_nTagSelect != -1) {
		m_listTag.SetCurSel(m_nTagSelect);
		SelchangeListTag();
	}
}

void CBZFormView::InitListMember(int iTag)
{
	m_listMember.DeleteAllItems();

	for_to(i, m_tag[iTag].m_member.GetCount()) {
		CStructMember& m = m_tag[iTag].m_member[i];
		CString s;

		s.Format(_T("+%2X"), m.m_ofs);
		LV_ITEM lvitem;
		lvitem.mask = LVIF_TEXT;
		lvitem.iItem = i;
		lvitem.iSubItem = MBRCOL_OFFSET;
		lvitem.pszText = (LPTSTR)(LPCTSTR)s;
		lvitem.iItem = m_listMember.InsertItem(&lvitem);

		lvitem.pszText =  (LPTSTR)(LPCTSTR)m.m_name;
		lvitem.iSubItem = MBRCOL_LABEL;
		m_listMember.SetItem(&lvitem);

		if(m.m_bytes) {
//			char* fval[3] = { "%d", "%u", "0x%X" };
			int val = m_pView->GetValue(m_pView->m_dwCaret + m.m_ofs, m.m_bytes);
			CString sVal;
			ULONGLONG qval = 0;
			
			switch(m.m_type)
			{
			case 0://char
				val = (int)(char)val;
				sVal = SeparateByComma(val, true);
				break;
			case 3://short
				val = (int)(short)val;
				sVal = SeparateByComma(val, true);
				break;
			case 6://long
			case 14://int
				sVal = SeparateByComma(val, true);
				break;
			case 13://int64
				qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
				sVal = SeparateByComma64(qval, true);
				break;
			case 12://float
				sVal.Format(_T("%f"), *((float*)&val));
				break;
			case 9://double
				qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
				sVal.Format(_T("%f"), qval);
				break;
			case 1: // byte(unsigned char)
			case 4: // word(unsigned short)
			case 7: // dword(unsigned int)
				sVal = SeparateByComma(val, false);
				break;
			case 10://qword(digit)
				qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
				sVal = SeparateByComma64(qval, false);
				//sVal.Format("%I64u", qval);
				break;
			case 2://BYTE(Hex)
				sVal.Format(_T("0x%02X"), val);
				break;
			case 5://WORD(Hex)
				sVal.Format(_T("0x%04X"), val);
				break;
			case 8://DWORD(Hex)
				sVal.Format(_T("0x%08X"), val);
				break;
			case 11://QWORD(hex)
				qval = m_pView->GetValue64(m_pView->m_dwCaret + m.m_ofs);
				sVal.Format(_T("0x%016I64X"), qval);
				break;
			}

			if(m.m_len != m.m_bytes)
				sVal += _T(" ...");
			lvitem.pszText =  (LPTSTR)(LPCTSTR)sVal;
			lvitem.iSubItem = MBRCOL_VALUE;
			m_listMember.SetItem(&lvitem);
		}
	}
}


void CBZFormView::Activate()
{
	if(m_listMember.GetSelectedCount() == 0)
		m_listTag.SetFocus();
	else
		m_listMember.SetFocus();
}
