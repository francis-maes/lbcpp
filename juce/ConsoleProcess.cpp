/* -------------------------------------- . ---------------------------------- .
| Filename : ConsoleProcess.cpp           | Child console process class.       |
| Author   : Alexandre Buge               | Manage process hosting and IO      |
| Started  : 29/01/2010 12:34             | redirection                        |
` --------------------------------------- . --------------------------------- */

#ifdef WIN32
# include <windows.h>
#endif // WIN32

#include "ConsoleProcess.h"
#ifdef WIN32
# include "Win32ConsoleProcess.h"
#else
# include "PosixConsoleProcess.h"
#endif // WIN32
using namespace juce;

ConsoleProcess* ConsoleProcess::create(const String& executable, const String& parameters /* = String::empty */, const String& workingDirectory /*= String::empty*/)
{
  ConsoleProcess* consoleProcess;
#ifdef WIN32
  consoleProcess = new Win32ConsoleProcess(executable, parameters, workingDirectory);
#else
  consoleProcess = new PosixConsoleProcess((const char* )executable, (const char* )parameters, (const char* )workingDirectory);
#endif
  if (!consoleProcess->createdCallback())
  {
    delete consoleProcess;
    return NULL;
  }
  return consoleProcess;
}
