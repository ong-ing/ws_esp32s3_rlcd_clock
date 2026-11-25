#include "ClockModel.h"
namespace rlcd_clock {
void navigate(Model &m, bool boot, Press p) {
  if(handleFiles(m,boot,p))return;
  if(handlePreferences(m,boot,p))return;
  if(handleTools(m,boot,p))return;
  if (p == Press::None)
    return;
  if (boot) {
    if (p == Press::Long) {
      if (m.page == Page::Home)
        m.refreshRequested = true;
    } else if (m.page == Page::Home)
      m.selected = (m.selected + 1) % 3;
    else if (m.page == Page::Menu)
      m.selected = (m.selected + 1) % 4;
    return;
  }
  if (p == Press::Long) {
    m.page = m.page == Page::Home ? Page::Menu : Page::Home;
    m.selected = 0;
    return;
  }
  if (m.page == Page::Home) {
    if(m.selected==1){openFiles(m);return;}
    m.page = m.selected==2?Page::Tools:Page::Info;
    if(m.page==Page::Tools)m.selected=0;
  }
  else if (m.page == Page::Info) {
    m.page = Page::Home;
    m.selected %= 3;
  } else {
    switch (m.selected) {
    case 0:
      m.page = Page::Home;
      m.selected = 0;
      break;
    case 1:
      m.refreshRequested = true;
      break;
    case 2:
      m.animated = !m.animated;
      break;
    case 3:
      m.page = Page::Info;
      m.selected = 3;
      break;
    }
  }
}
void shortSsid(const char *s, char *out, size_t capacity) {
  size_t pos = 0;
  int chars = 0;
  while (*s && chars < 4) {
    unsigned char c = *s;
    size_t bytes = c < 0x80             ? 1
                   : (c & 0xe0) == 0xc0 ? 2
                   : (c & 0xf0) == 0xe0 ? 3
                   : (c & 0xf8) == 0xf0 ? 4
                                        : 1;
    for (size_t i = 1; i < bytes; i++)
      if (!s[i] || (static_cast<unsigned char>(s[i]) & 0xc0) != 0x80) {
        bytes = 1;
        break;
      }
    if (pos + bytes >= capacity)
      break;
    for (size_t i = 0; i < bytes; i++)
      out[pos++] = *s++;
    chars++;
  }
  if (capacity)
    out[pos] = 0;
}
} // namespace rlcd_clock
