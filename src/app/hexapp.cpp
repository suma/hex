#include "hexapp.h"

HexApp::HexApp(int argc, char *argv[])
  : QApplication(argc, argv)
{
#ifdef Q_WS_MAC
  // Mac application continue running
  setQuitOnLastWindowClosed(false);
#endif
}

HexApp::~HexApp()
{
}
