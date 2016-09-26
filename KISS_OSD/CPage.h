#ifndef CPageh
#define CPageh
#include "CItem.h"
#include "CEventHandler.h"

class CItem;

class CPage : public CEventHandler
{
  public:
  CPage();
  ~CPage();
  void Setup(MAX7456 *OSD, uint8_t topLeftRow, uint8_t topLeftCol);
  void AddItem(CItem* newItem);
  CItem* GetActiveItem();
  void Show();
  void Hide(bool resetActive = false);
  void MoveItemDown();
  void MoveItemUp();
  virtual void OnEvent(CEventHandler::stickEvent event);
  void ToggleSpace();
  bool Toggled();
  
  private:
  static const uint8_t MAX_ITEMS = 10;
  uint8_t m_itemCount;
  CItem* m_items[MAX_ITEMS];
  uint8_t m_activeItem;
  uint8_t m_topLeftRow, m_topLeftCol;
  bool m_active;
  bool m_space;
  MAX7456 *m_OSD;
};

#endif
