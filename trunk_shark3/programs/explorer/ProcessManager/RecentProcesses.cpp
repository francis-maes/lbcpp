/*-----------------------------------------.---------------------------------.
| Filename: RecentProcesses.cpp            | Recent Processes                |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "RecentProcesses.h"
using namespace lbcpp;

void RecentProcesses::addRecentExecutable(const juce::File& file)
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

std::vector<string> RecentProcesses::getRecentArguments(const juce::File& executable) const
{
  if (!executable.exists())
    return std::vector<string>();
  int index = findRecentExecutable(executable);
  jassert(index >= 0);
  return index >= 0? v[index].arguments : std::vector<string>();
}

std::vector<juce::File> RecentProcesses::getRecentWorkingDirectories(const juce::File& executable) const
{
  if (!executable.exists())
    return std::vector<juce::File>();
  int index = findRecentExecutable(executable);
  jassert(index >= 0);
  return index >= 0? v[index].workingDirectories : std::vector<juce::File>();
}

ProcessConsoleSettingsPtr RecentProcesses::getExecutableConsoleSettings(const juce::File& executable) const
{
  int index = findRecentExecutable(executable);
  jassert(index >= 0);
  return index >= 0? v[index].consoleSettings : ProcessConsoleSettingsPtr();
}

void RecentProcesses::addRecent(const juce::File& executable, const string& arguments, const juce::File& workingDirectory)
{
  addRecentExecutable(executable);
  v[0].addRecentArguments(arguments);
  if (workingDirectory.exists())
    v[0].addRecentWorkingDirectory(workingDirectory);
  ExplorerConfiguration::save(defaultExecutionContext());
}

RecentProcesses::RecentExecutable::RecentExecutable(const juce::File& executable)
  : executable(executable), consoleSettings(new ProcessConsoleSettings)
{
  arguments.push_back(JUCE_T(" "));
  consoleSettings->addFilter(new ProcessConsoleFilter(string::empty, juce::Colours::red));
  consoleSettings->addFilter(new ProcessConsoleFilter(string::empty, juce::Colours::orange));
  consoleSettings->addFilter(new ProcessConsoleFilter(string::empty, juce::Colours::yellow));
  consoleSettings->addFilter(new ProcessConsoleFilter(string::empty, juce::Colours::green));
  consoleSettings->addFilter(new ProcessConsoleFilter(string::empty, juce::Colours::cyan));
  consoleSettings->addFilter(new ProcessConsoleFilter(string::empty, juce::Colours::white));
}

void RecentProcesses::RecentExecutable::addRecentArguments(const string& args)
{
  size_t i;
  for (i = 0; i < arguments.size(); ++i)
  if (arguments[i] == args)
    break;
  if (i < arguments.size())
    arguments.erase(arguments.begin() + i);
  arguments.insert(arguments.begin(), args);
}

void RecentProcesses::RecentExecutable::addRecentWorkingDirectory(const juce::File& workingDirectory)
{
  size_t i;
  for (i = 0; i < workingDirectories.size(); ++i)
  if (workingDirectories[i] == workingDirectory)
    break;
  if (i < workingDirectories.size())
    workingDirectories.erase(workingDirectories.begin() + i);
  workingDirectories.insert(workingDirectories.begin(), workingDirectory);
}

int RecentProcesses::findRecentExecutable(const juce::File& file) const
{
  for (size_t i = 0; i < v.size(); ++i)
    if (v[i].executable == file)
      return (int)i;
  return -1;
}
