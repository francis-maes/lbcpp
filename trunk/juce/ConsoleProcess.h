/* -------------------------------------- . ---------------------------------- .
| Filename : ConsoleProcess.h             | Child console process class.       |
| Author   : Alexandre Buge               | Manage process hosting and IO      |
| Started  : 29/01/2010 12:34             | redirection                        |
` --------------------------------------- . --------------------------------- */
                               
#ifndef SMODE_PROJECT_REPOSITORY_CONSOLE_PROCESS_H_
# define SMODE_PROJECT_REPOSITORY_CONSOLE_PROCESS_H_

# include "juce_amalgamated.h"

namespace juce
{

class JUCE_API ConsoleProcess
{
public:
  virtual ~ConsoleProcess() {}

  static ConsoleProcess* create(const String& executable, const String& parameters = String::empty, const String& workingDirectory = String::empty);

  virtual bool readStandardOutput(String& result) const = 0;
  virtual bool writeStandardInput(const String& input) const = 0;
  virtual bool isRunning(int& returnCode) = 0;
  virtual bool killProcess() = 0;

  juce_UseDebuggingNewOperator

protected:
  ConsoleProcess() {}
  virtual bool createdCallback() = 0;
};

}; /* namespace juce */

#endif // !SMODE_PROJECT_REPOSITORY_CONSOLE_PROCESS_H_
