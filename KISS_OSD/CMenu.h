#ifndef CMenuh
#define CMenuh
#include "CPage.h"
#include "CSettings.h"
#include "CEventDispatcher.h"

class CMenu : public CEventHandler
{
  public:
  CMenu();
  ~CMenu();
  virtual void OnEvent(CEventHandler::stickEvent event);
  bool IsActive();
  void Show();  
  void Setup(CSettings *settings, MAX7456 *OSD);
  CEventDispatcher* GetDispatcher();
  
  private:
  bool m_active;
  CPage *m_activePage;
  CSettings* m_settings;
  CEventDispatcher m_dispatcher;
  CPage m_mainPage, m_batteryPage, m_orderPage;
  CItem m_orderItem, m_batteryItem, m_saveExitItem, m_cancelItem, m_batBackItem, m_orderBackItem;
  CStringItem m_dvChannelItem, m_aspRatioItem, m_batWarningItem, m_batChannelItem;
  CItem m_orderItems[CSettings::MAX_DISPLAY_ITEMS];
  CIntItem m_batMahItem;
  
  void Hide();
};

#endif
