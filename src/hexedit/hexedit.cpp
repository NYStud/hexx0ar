#include <cstring>
#include "hexedit.hpp"

HexEdit::HexEdit()
{
  // Settings
  Open = true;
  ReadOnly = false;
  Rows = 16;
  OptShowAscii = true;
  OptShowHexII = false;
  OptGreyOutZeroes = true;
  OptMidRowsCount = 8;
  OptAddrDigitsCount = 0;
  ReadFn = NULL;
  WriteFn = NULL;

  // State/Internals
  ContentsWidthChanged = false;
  DataEditingAddr = (size_t)-1;
  DataEditingTakeFocus = false;
  memset(DataInputBuf, 0, sizeof(DataInputBuf));
  memset(AddrInputBuf, 0, sizeof(AddrInputBuf));
  GotoAddr = (size_t)-1;
}
