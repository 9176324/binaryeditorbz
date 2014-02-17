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
#include "ComboBox4ToolBar.h"
#include "BZView.h"

class CBZView;

void CComboBox4ToolBar::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  CBZView* pBZView = GetActiveBZView();
  BOOL bCtrl  = (GetKeyState(VK_CONTROL) < 0);
  switch (nChar)
  {
  case VK_ESCAPE:
    pBZView->SetFocus();
    return;
  case VK_RETURN:
    if(!GetDroppedState()) {
      ::PostMessage(pBZView->m_hWnd, WM_COMMAND, ID_JUMP_FINDNEXT, 0);
      return;
    }
    break;
  case VK_DOWN:
    if(!GetDroppedState()) {
      ShowDropDown();
      return;
    }
    break;
  case 'X':
    if(bCtrl) {
      Cut();
      return;
    }
    break;
  case 'C':
    if(bCtrl) {
      Copy();
      return;
    }
    break;
  case 'V':
    if(bCtrl) {
      Paste();
      return;
    }
    break;
  }
}
