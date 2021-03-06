/*
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*  FileName: CPanelAlerts.cpp
*  Author:   Douglas Hoffman
*
*/

#include "mainApp.h"
#include "CPanelAlerts.h"
#include "CParmOsiris.h"
#include <wx/tglbtn.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "wxIDS.h"
#include "nwx/nwxKeyState.h"
DEFINE_EVENT_TYPE(wxEVT_ALERT_VIEW_CHANGED)

CPanelAlerts::~CPanelAlerts()
{
  CParmOsiris::GetGlobal()->SaveIfModified();
}
CPanelAlerts::CPanelAlerts(wxWindow *parent, wxWindowID id) :
  wxPanel(parent,id)
{
  {
    CParmOsirisGlobal px;
    int n = px->GetShowAlerts();
    m_ViewStatus.Set(n);
  }
  wxStaticText *pLabel = new wxStaticText(this,wxID_ANY,_T("Notices"));
  mainApp::SetBoldFont(pLabel);
  m_pButtonAll = 
    CreateToggleButton(_T("All"),0,
    _T("Show all notices or hold down the shift key to hide all notices"));
  wxToggleButton *pButtonDIRECTORY =
    CreateToggleButton(
      _T("Dir"),
      CAlertViewStatus::DIRECTORY,
      _T("Show or hide directory notices"));
  wxToggleButton *pButtonDIRECTORY_LOCUS =
    CreateToggleButton(
      _T("Dir Locus"),
      CAlertViewStatus::DIRECTORY_LOCUS,
      _T("Show or hide directory level locus notices"));
  wxToggleButton *pButtonSAMPLE = 
    CreateToggleButton(
      _T("Sample"),
      CAlertViewStatus::SAMPLE,
      _T("Show or hide sample level notices"));
  wxToggleButton *pButtonCHANNEL =
    CreateToggleButton(
      _T("Channel"),
      CAlertViewStatus::CHANNEL,
      _T("Show or hide channel notices for current sample"));
  wxToggleButton *pButtonILS =
    CreateToggleButton(
      _T("ILS"),
      CAlertViewStatus::ILS,
      _T("Show or hide ILS notices for current sample"));
  wxToggleButton *pButtonSAMPLE_LOCUS =
    CreateToggleButton(
      _T("Locus"),
      CAlertViewStatus::SAMPLE_LOCUS,
      _T("Show or hide locus notices for current sample"));
  _InitFromStatus();
  wxBoxSizer *pSizer(new wxBoxSizer(wxHORIZONTAL));
  pSizer->Add(
    pLabel,0,
    wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT,
    ID_BORDER);
  pSizer->Add(m_pButtonAll,0,wxALIGN_CENTRE_VERTICAL);
  pSizer->Add(pButtonDIRECTORY,0,wxALIGN_CENTRE_VERTICAL);
  pSizer->Add(pButtonDIRECTORY_LOCUS,0,wxALIGN_CENTRE_VERTICAL);
  pSizer->Add(pButtonSAMPLE,0,wxALIGN_CENTRE_VERTICAL);
  pSizer->Add(pButtonCHANNEL,0,wxALIGN_CENTRE_VERTICAL);
  pSizer->Add(pButtonILS,0,wxALIGN_CENTRE_VERTICAL);
  pSizer->Add(pButtonSAMPLE_LOCUS,0,wxALIGN_CENTRE_VERTICAL);
  pSizer->AddStretchSpacer(1);
  SetSizer(pSizer);
  Layout();
}

wxToggleButton *CPanelAlerts::CreateToggleButton(
  const wxString &sLabel, 
  int nFlagStatus, 
  const wxString &sToolTip)
{
  wxToggleButton *pRtn = new wxToggleButton(this,wxID_ANY,sLabel);
  if(nFlagStatus)
  {
    m_mapButtonValue.insert(
      map<wxToggleButton *, int>::value_type(pRtn,nFlagStatus));
  }
  if(!sToolTip.IsEmpty())
  {
    pRtn->SetToolTip(sToolTip);
  }
  return pRtn;
}

bool CPanelAlerts::_SetButtonValue(wxToggleButton *p, bool b)
{
  // return true if button state has changed

  bool bCurrent = p->GetValue();
  bool bRtn = (bCurrent != b);
  if(bRtn)
  {
    p->SetValue(b);
  }
  return bRtn;
}
bool CPanelAlerts::_SetStatusFromButton(bool bButtonSet, int n)
{
  // return true if a change is made

  bool bRtn = m_ViewStatus.IsValueSet(n) != bButtonSet;
  if(bRtn)
  {
    m_ViewStatus.SetValue(n,bButtonSet);
  }
  return bRtn;
}

bool CPanelAlerts::_InitFromStatus()
{
  map<wxToggleButton *, int>::iterator itr;
  bool bRtn = _SetButtonValue(m_pButtonAll,m_ViewStatus.IsAll());
  for(itr = m_mapButtonValue.begin();
    itr != m_mapButtonValue.end();
    ++itr)
  {
    if(_SetButtonValue(
        itr->first,m_ViewStatus.IsValueSet(itr->second)
       ))
    {
      bRtn = true;
    }
  }
  return bRtn;
}

bool CPanelAlerts::_SetupStatus()
{
  map<wxToggleButton *, int>::iterator itr;
  bool bRtn = false;
  for(itr = m_mapButtonValue.begin();
    itr != m_mapButtonValue.end();
    ++itr)
  {
    if(_SetStatusFromButton(
        itr->first->GetValue(),
        itr->second))
    {
      bRtn = true;
    }
  }
  _SetButtonValue(m_pButtonAll,m_ViewStatus.IsAll());
  return bRtn;
}

void CPanelAlerts::OnChange(wxCommandEvent &e)
{
  bool bChange = false;
  bool bShift = nwxKeyState::Shift();
  if(e.GetEventObject() != (wxObject *)m_pButtonAll)
  {
    bChange = _SetupStatus();
  }
  else if(bShift && m_ViewStatus.IsNone())
  {
    // no change, but make sure 'all' button is cleared
    _SetButtonValue(m_pButtonAll,false);
  }
  else if((!bShift) && m_ViewStatus.IsAll())
  {
    // no change, but make sure 'all' button is set
    _SetButtonValue(m_pButtonAll,true);
  }
  else
  {
    bChange = true;
    if(bShift)
    {
      m_ViewStatus.Clear();
    }
    else
    {
      m_ViewStatus.SetAll();
    }
    _InitFromStatus();
  }
  if(bChange)
  {
    CParmOsiris *pg = CParmOsiris::GetGlobal();
    pg->SetShowAlerts(m_ViewStatus.GetPlus());
    wxCommandEvent evt(wxEVT_ALERT_VIEW_CHANGED,GetId());
    evt.SetInt(m_ViewStatus.Get());
    GetEventHandler()->AddPendingEvent(evt);
  }
}



BEGIN_EVENT_TABLE(CPanelAlerts,wxPanel)
EVT_TOGGLEBUTTON(wxID_ANY,CPanelAlerts::OnChange)
END_EVENT_TABLE()
