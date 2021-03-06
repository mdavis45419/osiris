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
*
*  FileName: CMenuPlot.cpp
*  Author:   Douglas Hoffman
*
*/

#include "CMenuPlot.h"
#include "wxIDS.h"
#include "CKitColors.h"
#include "CPlotData.h"
#include "Platform.h"
#include "nwx/nwxString.h"
#include "nwx/nwxMenuItem.h"

//**********************************************  CMenuArtifact


CMenuArtifact::CMenuArtifact(int nOffset) 
  : wxMenu(_T(""),0), m_nOffset(nOffset)
{
  for(int i = 0; i < CArtifactDisplayList::ARTIFACT_DISPLAY_COUNT; i++)
  {
    AppendRadioItem(
      _Ndx2MenuID(i),_T(CArtifactDisplayList::apsDisplay[i]));
  }
  SetDefault();
}
int CMenuArtifact::GetIntValue()
{
  int nRtn = 0;
  for(int i = 0; i < CArtifactDisplayList::ARTIFACT_DISPLAY_COUNT; i++)
  {
    if(IsChecked(_Ndx2MenuID(i)))
    {
      nRtn = CArtifactDisplayList::anValue[i];
      i = 32000; // loop exit
    }
  }
  return nRtn;
}

void CMenuArtifact::SetIntValue(int n)
{
  for(int i = 0; i < CArtifactDisplayList::ARTIFACT_DISPLAY_COUNT; i++)
  {
    if(CArtifactDisplayList::anValue[i] == n)
    {
      Check(_Ndx2MenuID(i),true);
      i = 32000; // loop exit
    }
  }
}
int CMenuArtifact::_Ndx2MenuID(int ndx)
{
  return _ID(CArtifactDisplayList::anMenuID[ndx]);
}
void CMenuArtifact::SetDefault()
{
  Check(_Ndx2MenuID(CArtifactDisplayList::DEFAULT_NDX),true);
}


//**********************************************  CMenuPlot

IMPLEMENT_ABSTRACT_CLASS(CMenuPlot,wxMenu)

CMenuPlot::~CMenuPlot() {;}
CMenuPlot::CMenuPlot(
  CPlotData *pData, CKitColors *pColors)
    : wxMenu(_T(""),0),
    m_nOffset(0),
    m_nChannelCount(0),
    m_pKitColors(pColors),
    m_pData(pData),
    m_pMenuData(NULL),
    m_pMenuChannels(NULL),
    m_pMenuArtifact(NULL),
    m_pMenuLabels(NULL),
    m_nLastType(LABEL_NONE),
    m_bPreview(true),
    m_bUsedDefault(false)
{
  m_nOffset = ID_GET_PLOT_BASE(0);
  _Build(pData,pColors);
}

CMenuPlot::CMenuPlot(
  int nOffset, CPlotData *pData, CKitColors *pColors) 
    : wxMenu(_T(""),0),
    m_nOffset(nOffset),
    m_nChannelCount(0),
    m_pKitColors(pColors),
    m_pData(pData),
    m_pMenuData(NULL),
    m_pMenuChannels(NULL),
    m_pMenuArtifact(NULL),
    m_pMenuLabels(NULL),
    m_nLastType(LABEL_NONE),
    m_bPreview(false),
    m_bUsedDefault(false)
{
  _Build(pData,pColors);
}
CMenuPlot *CMenuPlot::Clone()
{
  CMenuPlot *pRtn = NULL;
  if(m_bPreview)
  {
    pRtn = new CMenuPlot(m_pData,m_pKitColors);
  }
  else
  {
    pRtn = new CMenuPlot(m_nOffset,m_pData,m_pKitColors);
  }
  pRtn->CopySettings(*this);
  return pRtn;
}
void CMenuPlot::_Build(CPlotData *pData, CKitColors *pColors) 
{
  wxString s;
  const CSingleKitColors *pKitColors =
    pColors->GetKitColors(pData->GetKitName());
  const CChannelColors *pChannelColors;
  wxMenuItem *pItem;
  int nID;
  unsigned int iu;

  m_nChannelCount = pData->GetChannelCount();
  
  // build data menu

  m_pMenuData = new wxMenu;
  m_pMenuData->AppendCheckItem(
    _ID(IDmenuPlotDataAnalyzed),
    _T("Analyzed"));
  m_pMenuData->AppendCheckItem(
    _ID(IDmenuPlotDataRaw),
    _T("Raw"));
  m_pMenuData->AppendCheckItem(
    _ID(IDmenuPlotDataLadder),
    _T("Ladder"));
  m_pMenuData->AppendCheckItem(
    _ID(IDmenuPlotDataBaseline),
    _T("Baseline"));
  EnableBaseline(pData->HasBaseline());
  if(!m_bPreview)
  {
    m_pMenuData->Append(
      IDmenuTable,
      _T("Show Table"));
  }


  // channel menu

  m_pMenuChannels = new wxMenu;
  if(pKitColors == NULL)
  {
    for(iu = 1; iu <= m_nChannelCount; iu++)
    {
      nID = _ID( ID_GET_CHANNEL_ID_FROM_NR(iu) );
      s.Printf(_T("Channel %u"),iu);
      m_pMenuChannels->AppendCheckItem(nID,s);
    }
  }
  else
  {
    for(iu = 1; iu <= m_nChannelCount; iu++)
    {
      pChannelColors = pKitColors->GetColorChannel(iu);
      if(pChannelColors != NULL)
      {
        nID = _ID(ID_GET_CHANNEL_ID_FROM_NR(iu));
        s = nwxString::FormatNumber((int)iu);
        s.Append(_T(" - "));
        s.Append(pChannelColors->m_sDyeName);
        pItem = new wxMenuItem(
          m_pMenuChannels,nID,s,_T(""),wxITEM_CHECK);
#if COLOR_MENU_ITEMS
        pItem->SetBackgroundColour(pChannelColors->m_ColorAnalyzed);
        pItem->SetTextColour(*wxWHITE);
#endif
        m_pMenuChannels->Append(pItem);
      }
    }
  }

  m_pMenuLabels = new CMenuLabels(true,m_nOffset);
  m_pMenuArtifact = new CMenuArtifact(m_nOffset);

  // now build the menu

  AppendSubMenu(m_pMenuData,_T("Data"));
  AppendSubMenu(m_pMenuChannels,_T("Channel"));
  if(!m_bPreview)
  {
    AppendCheckItem(
      _ID(IDmenuPlotSync),
      _T("Synchronize Axes"));
  }
  AppendCheckItem(
    _ID(IDmenuPlotILS),
    _T("Show ILS"));
  AppendCheckItem(
    _ID(IDmenuPlotRFU),
    _T("Show minimum RFU"));
  m_pMenuItemLabels = AppendSubMenu(m_pMenuLabels,_T("Labels"));
  AppendSubMenu(m_pMenuArtifact,_T("Artifacts"));
  AppendCheckItem(
    _ID(IDmenuPlotLadderLabels),
    _T("Show ladder labels"));
  if(!m_bPreview)
  {
    Append(_ID(IDmenuPlotResetAxes),
      _T("Reset Axes"),
      _T("Reset axes to display all alleles"));
    Append(_ID(IDmenuPlotResetAxesFull),
      _T("Show Primer Peaks"),
      _T("Reset axes to include primer peaks"));
    Append(_ID(IDmenuPlotMultiple),
      _T("Multiple Plots"),
      _T("Show a plot for each channel"));
    Append(_ID(IDmenuPlotAppend),
      _T("Append plot"),
      _T("Append a new plot directly below this plot"));
    Append(_ID(IDmenuPlotRemove),
      _T("Remove plot"),
      _T("Remove this plot, keep all others"));
    Append(_ID(IDmenuPlotRemoveOthers),
      _T("Remove other plots"),
      _T("Remove all plots except this"));
  }
  else
  {
    Append(IDMaxLadderLabels,
      LABEL_MAX_PEAK_LABELS,
      STATUS_MAX_PEAK_LABELS);
  }
}
const wxChar * const CMenuPlot::LABEL_MAX_PEAK_LABELS =
  _T("Max. ladder peak labels...");
const wxChar * const CMenuPlot::STATUS_MAX_PEAK_LABELS =
  _T("Set the maximum number of labels for ladder peaks");



void CMenuPlot::EnableLabelMenu(bool bEnable)
{
  m_pMenuItemLabels->Enable(bEnable);
}
bool CMenuPlot::IsLabelMenuEnabled()
{
  return m_pMenuItemLabels->IsEnabled();
}

bool CMenuPlot::AnalyzedValue()
{
  bool b = m_pMenuData->IsChecked(
    _ID(IDmenuPlotDataAnalyzed));
  return b;
}
bool CMenuPlot::RawValue()
{
  bool b = m_pMenuData->IsChecked(
    _ID(IDmenuPlotDataRaw));
  return b;
}
bool CMenuPlot::LadderValue()
{
  bool b = m_pMenuData->IsChecked(
    _ID(IDmenuPlotDataLadder));
  return b;
}

bool CMenuPlot::BaselineValue()
{
  bool b = m_pMenuData->IsChecked(
    _ID(IDmenuPlotDataBaseline));
  return b;
}
bool CMenuPlot::BaselineEnabled()
{
  bool b = m_pMenuData->IsEnabled(
    _ID(IDmenuPlotDataBaseline));
  return b;
}


//   data

void CMenuPlot::ShowAnalyzed(bool b)
{
  m_pMenuData->Check(_ID(IDmenuPlotDataAnalyzed),b);
}
void CMenuPlot::ShowRaw(bool b)
{
  m_pMenuData->Check(_ID(IDmenuPlotDataRaw),b);
}
void CMenuPlot::ShowLadder(bool b)
{
  m_pMenuData->Check(_ID(IDmenuPlotDataLadder),b);
}
void CMenuPlot::ShowBaseline(bool b)
{
  m_pMenuData->Check(_ID(IDmenuPlotDataBaseline),b);
}
void CMenuPlot::EnableBaseline(bool b)
{
  m_pMenuData->Enable(_ID(IDmenuPlotDataBaseline),b);
}

// channels

bool CMenuPlot::ChannelValue(unsigned int n)
{
  int nID = ID_GET_CHANNEL_ID_FROM_NR(n);
  return m_pMenuChannels->IsChecked(_ID(nID));
}
void CMenuPlot::ShowChannel(unsigned int n,bool b)
{
  int nID = ID_GET_CHANNEL_ID_FROM_NR(n);
  m_pMenuChannels->Check(_ID(nID),b);
}
int CMenuPlot::ChannelCount()
{
  return m_nChannelCount;
}

//  Sync, ILS, RFU

bool CMenuPlot::SyncValue()
{
  bool bRtn = false;
  if(!m_bPreview)
  {
    bRtn = IsChecked(_ID(IDmenuPlotSync));
  }
  return bRtn;
}
bool CMenuPlot::MinRfuValue()
{
  return IsChecked(_ID(IDmenuPlotRFU));
}

bool CMenuPlot::LadderLabels()
{
  return IsChecked(_ID(IDmenuPlotLadderLabels));
}   

bool CMenuPlot::ILSValue()
{
  return IsChecked(_ID(IDmenuPlotILS));
}


void CMenuPlot::SetSync(bool b)
{
  if(!m_bPreview)
  {
    Check(_ID(IDmenuPlotSync),b);
  }
}
void CMenuPlot::ShowILS(bool b)
{
  Check(_ID(IDmenuPlotILS),b);
}
void CMenuPlot::ShowMinRfu(bool b)
{
  Check(_ID(IDmenuPlotRFU),b);
}
void CMenuPlot::ShowLadderLabels(bool b)
{
  Check(_ID(IDmenuPlotLadderLabels),b);
}
//  labels, artifacts

LABEL_PLOT_TYPE CMenuPlot::LabelType()
{
  LABEL_PLOT_TYPE n = 
    (LABEL_PLOT_TYPE)m_pMenuLabels->GetCheckedOffset();
  return n;
}
int CMenuPlot::ArtifactValue()
{
  return m_pMenuArtifact->GetIntValue();
}
void CMenuPlot::SetLabelType(LABEL_PLOT_TYPE n,LABEL_PLOT_TYPE nDefault)
{
  LABEL_PLOT_TYPE nType = CheckLabelType(n,nDefault);
  m_bUsedDefault = (nType != n);
  m_nLastType = nType;
  m_pMenuLabels->SelectByOffset((int)nType);
}
void CMenuPlot::SetArtifactValue(int nLevel)
{
  m_pMenuArtifact->SetIntValue(nLevel);
}

//  enable/disable append, delete item

void CMenuPlot::EnableAppend(bool b)
{
  if(!m_bPreview)
  {
    Enable(_ID(IDmenuPlotAppend),b);
  }
}
bool CMenuPlot::IsAppendEnabled()
{
  bool bRtn = false;
  if(!m_bPreview)
  {
    bRtn = IsEnabled(_ID(IDmenuPlotAppend));
  }
  return bRtn;
}
void CMenuPlot::EnableDelete(bool b)
{
  if(!m_bPreview)
  {
    Enable(_ID(IDmenuPlotRemove),b);
    Enable(_ID(IDmenuPlotRemoveOthers),b);
  }
}
bool CMenuPlot::IsDeleteEnabled()
{
  bool bRtn = false;
  if(!m_bPreview)
  {
    bRtn = IsEnabled(_ID(IDmenuPlotRemove));
  }
  return bRtn;
}

void CMenuPlot::EnablePeakAreaLabel(bool b)
{
  m_pMenuLabels->EnablePeakAreaLabel(b);
}
bool CMenuPlot::PeakAreaLabelEnabled()
{
  return m_pMenuLabels->PeakAreaLabelEnabled();
}
