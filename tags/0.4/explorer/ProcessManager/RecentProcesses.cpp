/*-----------------------------------------.---------------------------------.
| Filename: RecentProcesses.cpp            | Recent Processes                |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "RecentProcesses.h"
using namespace lbcpp;

void RecentProcesses::addRecentExecutable(const File& file)
{
  int index = findRecentExecutable(file);
  if (index >= 0)
  {
    RecentExecutable r = v[index];
    v.erase(v.begin() + index);
    v.insert(v.begin(), r);
  }
  else
    v.insert(v.begin(), RecentExecutable(file));
  ExplorerConfiguration::save(defaultExecutionContext());
}

std::vector<String> RecentProcesses::getRecentArguments(const File& executable) const
{
  if (!executable.exists())
    return std::vector<String>();
  int index = findRecentExecutable(executable);
  jassert(index >= 0);
  return index >= 0? v[index].arguments : std::vector<String>();
}

std::vector<File> RecentProcesses::getRecentWorkingDirectories(const File& executable) const
{
  if (!executable.exists())
    return std::vector<File>();
  int index = findRecentExecutable(executable);
  jassert(index >= 0);
  return index >= 0? v[index].workingDirectories : std::vector<File>();
}

ProcessConsoleSettingsPtr RecentProcesses::getExecutableConsoleSettings(const File& executable) const
{
  int index = findRecentExecutable(executable);
  jassert(index >= 0);
  return index >= 0? v[index].consoleSettings : ProcessConsoleSettingsPtr();
}

void RecentProcesses::addRecent(const File& executable, const String& arguments, const File& workingDirectory)
{
  addRecentExecutable(executable);
  v[0].addRecentArguments(arguments);
  if (workingDirectory.exists())
    v[0].addRecentWorkingDirectory(workingDirectory);
  ExplorerConfiguration::save(defaultExecutionContext());
}

RecentProcesses::RecentExecutable::RecentExecutable(const File& executable)
  : executable(executable), consoleSettings(new ProcessConsoleSettings)
{
  arguments.push_back(T(" "));
  consoleSettings->addFilter(new ProcessConsoleFilter(String::empty, juce::Colours::red));
  consoleSettings->addFilter(new ProcessConsoleFilter(String::empty, juce::Colours::orange));
  consoleSettings->addFilter(new ProcessConsoleFilter(String::empty, juce::Colours::yellow));
  consoleSettings->addFilter(new ProcessConsoleFilter(String::empty, juce::Colours::green));
  consoleSettings->addFilter(new ProcessConsoleFilter(String::empty, juce::Colours::cyan));
  consoleSettings->addFilter(new ProcessConsoleFilter(String::empty, juce::Colours::white));
}

void RecentProcesses::RecentExecutable::addRecentArguments(const String& args)
{
  size_t i;
  for (i = 0; i < arguments.size(); ++i)
  if (arguments[i] == args)
    break;
  if (i < arguments.size())
    arguments.erase(arguments.begin() + i);
  arguments.insert(arguments.begin(), args);
}

void RecentProcesses::RecentExecutable::addRecentWorkingDirectory(const File& workingDirectory)
{
  size_t i;
  for (i = 0; i < workingDirectories.size(); ++i)
  if (workingDirectories[i] == workingDirectory)
    break;
  if (i < workingDirectories.size())
    workingDirectories.erase(workingDirectories.begin() + i);
  workingDirectories.insert(workingDirectories.begin(), workingDirectory);
}

int RecentProcesses::findRecentExecutable(const File& file) const
{
  for (size_t i = 0; i < v.size(); ++i)
    if (v[i].executable == file)
      return (int)i;
  return -1;
}
