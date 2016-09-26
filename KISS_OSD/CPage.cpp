#include "CPage.h"

CPage::CPage() :
m_OSD(NULL),
m_topLeftRow(-1),
m_topLeftCol(-1),
m_space(true),
m_itemCount(0)
{
  uint8_t i;
  for(i=0; i<MAX_ITEMS; i++)
  {
    m_items[i] = NULL;
  }
}

CPage::~CPage()
{
}

void CPage::AddItem(CItem* newItem)
{
  if(m_itemCount < MAX_ITEMS)
  {
    m_items[m_itemCount] = newItem;
    m_itemCount++;
  }
}

void CPage::Setup(MAX7456 *OSD, uint8_t topLeftRow, uint8_t topLeftCol)
{
  m_OSD = OSD;
  m_topLeftRow = topLeftRow;
  m_topLeftCol = topLeftCol;
}

CItem* CPage::GetActiveItem()
{
  if(m_activeItem >= 0 && m_activeItem < m_itemCount)
  {
    return m_items[m_activeItem];
  }
  return NULL;
}

void CPage::Show()
{
  m_active = true;
  uint8_t i;
  for(i=0; i < m_itemCount; i++)
  {
    m_OSD->setCursor(m_topLeftCol, m_topLeftRow + i);
    if(i = m_activeItem)
    {
      if(m_space)
      {
        m_OSD->print("> ");
      }
      else
      {
        m_OSD->print(" >");
      }
    }
    else
    {
      m_OSD->print("  ");
    }
    m_items[i]->Print();
  }
}

void CPage::Hide(bool resetActive)
{
  m_active = false;
  if(resetActive)
  {
    m_activeItem = 0;
  }
}

void CPage::MoveItemDown()
{
  if(m_activeItem < (m_itemCount-1))
  {
    CItem* tmp = m_items[m_activeItem+1];
    m_items[m_activeItem+1] = m_items[m_activeItem];
    m_items[m_activeItem] = tmp;
    m_activeItem++;
  }
}

void CPage::MoveItemUp()
{
  if(m_activeItem > 0)
  {
    CItem* tmp = m_items[m_activeItem-1];
    m_items[m_activeItem-1] = m_items[m_activeItem];
    m_items[m_activeItem] = tmp;
    m_activeItem--;
  }
}

void CPage::OnEvent(CEventHandler::stickEvent event)
{
  if(m_active)
  {
    switch(event)
    {
      case CEventHandler::PitchUp:
      if(m_activeItem > 0)
      {
        m_activeItem--;
      }
      break;
      case CEventHandler::PitchDown:
      if(m_activeItem < (m_itemCount-1))
      {
        m_activeItem ++;
      }
      break;
    }
  }
}

void CPage::ToggleSpace()
{
  m_space = !m_space;
}

bool CPage::Toggled()
{
  return !m_space;
}
