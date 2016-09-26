#include "CMenu.h"
#include "fixFont.h"

CMenu::CMenu() :
m_active(false),
m_activePage(NULL)
{
}

CMenu::~CMenu()
{
}

void CMenu::OnEvent(CEventHandler::stickEvent event)
{
  switch(event)
  {
    case CEventHandler::RollRight:
    if(m_active && m_activePage != NULL)
    {
      if(m_activePage->GetActiveItem()->GetNextPage() == NULL)
      {
        if(m_activePage == &m_orderPage)
        {
          m_activePage->ToggleSpace();
        }
        if(m_activePage == &m_mainPage && strcmp(m_activePage->GetActiveItem()->GetKey(), fixStr("save & exit")) == 0)
        {
          Hide();
          m_settings->WriteSettings();
        }
        if(m_activePage == &m_mainPage && strcmp(m_activePage->GetActiveItem()->GetKey(), fixStr("cancel")) == 0)
        {
          Hide();
          m_settings->ReadSettings();
        }
        m_activePage->GetActiveItem()->ValueRight();
      }
      else
      {
        CPage* tmp = m_activePage;
        tmp->Hide();
        m_activePage = m_activePage->GetActiveItem()->GetNextPage();
        m_activePage->Show();
      }
    }
    break;
    case CEventHandler::RollLeft:
    if(m_active && m_activePage != NULL)
    {
      if(m_activePage == &m_orderPage && m_activePage->Toggled())
      {
        m_orderPage.ToggleSpace();
      }
      m_activePage->GetActiveItem()->ValueLeft();
    }
    break;
    case CEventHandler::PitchUp:
    if(m_active && m_activePage != NULL && m_activePage == &m_orderPage)
    {
      if(m_orderPage.Toggled())
      {
        m_orderPage.MoveItemUp();
      }
    }
    break;
    case CEventHandler::PitchDown:
    if(m_active && m_activePage != NULL && m_activePage == &m_orderPage)
    {
      if(m_orderPage.Toggled())
      {
        m_orderPage.MoveItemDown();
      }
    }
    break;
    case CEventHandler::YawLeft:
    if(!m_active && m_activePage != NULL)
    {
      Show();
    }
    break;
  }
}

bool CMenu::IsActive()
{
  return m_active;
}

void CMenu::Show()
{
  m_active = true;
  m_activePage->Show();
}

void CMenu::Setup(CSettings *settings, MAX7456 *OSD)
{
  uint8_t topLeftRow = 3; //FIXME? need to check with 4:3 and 16:9 positions - maybe it doesn't matter?
  uint8_t topLeftCol = 4; //FIXME?
  m_dispatcher.AddHandler(this);
  m_mainPage.Setup(OSD,topLeftRow,topLeftCol);
  m_dispatcher.AddHandler(&m_mainPage);
  m_batteryPage.Setup(OSD,topLeftRow,topLeftCol);
  m_dispatcher.AddHandler(&m_batteryPage);
  m_orderPage.Setup(OSD,topLeftRow,topLeftCol);
  m_dispatcher.AddHandler(&m_orderPage);
  char aux[4][10];
  strcpy(aux[0], fixStr("aux1"));
  strcpy(aux[1], fixStr("aux2"));
  strcpy(aux[2], fixStr("aux3"));
  strcpy(aux[3], fixStr("aux4"));
  char key[CItem::MAX_KEY_LENGTH];
  m_dvChannelItem.Setup(OSD, fixStr("dv channel"), &settings->m_DVchannel, (char**)aux, (uint8_t)4);
  m_mainPage.AddItem(&m_dvChannelItem);
  m_orderItem.Setup(OSD, fixStr("order"), &m_orderPage);
  m_mainPage.AddItem(&m_orderItem);
  m_batteryItem.Setup(OSD, fixStr("battery"), &m_batteryPage);
  m_mainPage.AddItem(&m_batteryItem);
  char ratios[2][5];
  strcpy(ratios[0], fixStr("4:3"));
  strcpy(ratios[1], fixStr("16:9"));
  m_aspRatioItem.Setup(OSD, fixStr("asp ratio"), &settings->m_aspectRatio, (char**)ratios, 2);
  m_mainPage.AddItem(&m_aspRatioItem);
  m_saveExitItem.Setup(OSD, fixStr("save + exit"));
  m_mainPage.AddItem(&m_saveExitItem);
  m_cancelItem.Setup(OSD, fixStr("cancel"));
  m_mainPage.AddItem(&m_cancelItem);
  char onoff[2][5];
  strcpy(onoff[0], fixStr("off"));
  strcpy(onoff[1], fixStr("on"));
  m_batMahItem.Setup(OSD, fixStr("bat mah"), &settings->m_bat_mAh_warning, 300, 32000, 0, 500, fixStr(" mah"));
  m_batteryPage.AddItem(&m_batMahItem);
  m_batWarningItem.Setup(OSD, fixStr("bat warning"), &settings->m_batWarning, (char**)onoff, 2);
  m_batteryPage.AddItem(&m_batWarningItem);
  m_batChannelItem.Setup(OSD, fixStr("bat channel"), &settings->m_BAT_AUX_DV_CHANNEL, (char**)aux, 4);
  m_batteryPage.AddItem(&m_batChannelItem);
  m_batBackItem.Setup(OSD, fixStr("back"), &m_mainPage);
  m_batteryPage.AddItem(&m_batBackItem);
  uint8_t i;
  for(i=0; i < CSettings::MAX_DISPLAY_ITEMS; i++)
  {
    m_orderItems[i].Setup(OSD, settings->m_displayOrder[i]);
    m_orderPage.AddItem(&m_orderItems[i]);
  }
  m_orderBackItem.Setup(OSD, fixStr("back"), &m_mainPage);
  m_orderPage.AddItem(&m_orderBackItem);
  
  m_settings = settings;
}

CEventDispatcher* CMenu::GetDispatcher()
{
  return &m_dispatcher;
}

void CMenu::Hide()
{
  m_active = false;
  m_activePage->Hide();
}
