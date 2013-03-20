/*-----------------------------------------.---------------------------------.
| Filename: WorkUnit.cpp                   | Base class for Work Units       |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 20:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <oil/Core.h>
#include <oil/Execution/WorkUnit.h>
#include <oil/Execution/ExecutionContext.h>
using namespace lbcpp;

/*
** WorkUnit
*/
int WorkUnit::main(ExecutionContext& context, WorkUnitPtr workUnit, int argc, char* argv[])
{
  std::vector<string> arguments(argc - 1);
  for (size_t i = 1; i < (size_t)argc; ++i)
  {
    string arg = argv[i];
    arguments[i - 1] = arg;
    if (arg == JUCE_T("--help") || arg == JUCE_T("-h"))
    {
      context.informationCallback(workUnit->getUsageString());
      return 0;
    }
  }

  std::vector< std::pair<size_t, ObjectPtr> > parsedArguments;
  if (!workUnit->parseArguments(context, arguments, parsedArguments))
  {
    context.informationCallback(workUnit->getUsageString());
    return 1;
  }

  workUnit->setArguments(context, parsedArguments);
  context.run(workUnit);
  return 0;
}

bool WorkUnit::parseArguments(ExecutionContext& context, const string& arguments, std::vector< std::pair<size_t, ObjectPtr> >& res)
{
  StringArray tokens;
  tokens.addTokens(arguments, true);
  std::vector<string> toks;
  toks.reserve(tokens.size());
  for (int i = 0; i < tokens.size(); ++i)
  {
    string str = tokens[i].isQuotedString() ? tokens[i].unquoted() : tokens[i];
    if (str.isNotEmpty())
      toks.push_back(str);
  }
  return parseArguments(context, toks, res);
}

inline bool isNegativeNumber(const string& str)
  {return str.containsOnly(JUCE_T("-+e0123456789"));}

bool WorkUnit::parseArguments(ExecutionContext& context, const std::vector<string>& arguments, std::vector< std::pair<size_t, ObjectPtr> >& res)
{
  /* shortcut */
  std::map<string, size_t> variableNames;
  std::map<string, size_t> variableShortNames;
  ClassPtr thisClass = getClass();
  for (size_t i = 0; i < getNumVariables(); ++i)
  {
    string name = thisClass->getMemberVariableName(i);
    string shortName = thisClass->getMemberVariableShortName(i);
    variableNames[name] = i;
    variableShortNames[shortName.isNotEmpty() ? shortName : name] = i;
  }
  
  for (size_t i = 0; i < arguments.size(); )
  {
    string argumentName;
    string argumentValue;
    std::map<string, size_t>* namesMap = NULL;
    if (arguments[i].startsWith(JUCE_T("--")))
    {
      int end = arguments[i].indexOfChar(JUCE_T('='));
      if (end == -1)
        end = arguments[i].length();
      else
        argumentValue = arguments[i].substring(end + 1);

      argumentName = arguments[i].substring(2, end);
      namesMap = &variableNames;
    }
    else if (arguments[i].startsWith(JUCE_T("-")))
    {
      argumentName = arguments[i].substring(1);
      namesMap = &variableShortNames;
    }

    if (!namesMap)
    {
      context.errorCallback(JUCE_T("WorkUnit::parseArguments"), JUCE_T("Unexpected expression : ") + arguments[i]);
      return false;
    }

    std::map<string, size_t>::const_iterator it = namesMap->find(argumentName);
    if (it == namesMap->end())
    {
      context.errorCallback(JUCE_T("WorkUnit::parseArguments"), JUCE_T("Unknown argument: ") + argumentName);
      return false;
    }

    size_t variableIndex = it->second;
    for (++i ; i < arguments.size() && (!arguments[i].startsWith(JUCE_T("-")) || isNegativeNumber(arguments[i])); ++i)
      argumentValue += JUCE_T(" ") + arguments[i];
    argumentValue = argumentValue.trim();

    ClassPtr argumentType = getVariableType(variableIndex);
    ObjectPtr value;
    if (argumentType == booleanClass && argumentValue.isEmpty())
      value = new Boolean(true); // particular case for boolean arguments: if no value has been given, we take true by default
    else
    {
      value = Object::createFromString(context, argumentType, argumentValue);
      if (!value)
      {
        context.errorCallback(JUCE_T("WorkUnit::parseArguments"), JUCE_T("Incomprehensible value of") + argumentName.quoted() + JUCE_T(" : ") + argumentValue);
        return false;
      }
    }
    res.push_back(std::make_pair(variableIndex, value));
  }
  return true;
}

void WorkUnit::setArguments(ExecutionContext& context, const std::vector< std::pair<size_t, ObjectPtr> >& arguments)
{
  for (size_t i = 0; i < arguments.size(); ++i)
    setVariable(arguments[i].first, arguments[i].second);
}

bool WorkUnit::parseArguments(ExecutionContext& context, const string& arguments)
{
  std::vector< std::pair<size_t, ObjectPtr> > args;
  bool res = parseArguments(context, arguments, args);
  setArguments(context, args);
  return res;
}

bool WorkUnit::parseArguments(ExecutionContext& context, const std::vector<string>& arguments)
{
  std::vector< std::pair<size_t, ObjectPtr> > args;
  bool res = parseArguments(context, arguments, args);
  setArguments(context, args);
  return res;
}

string WorkUnit::getUsageString() const
{
  ClassPtr thisClass = getClass();
  /* Compute the longest string */
  size_t longestName = 4;
  size_t longestShortName = 1;
  for (size_t i = 0; i < getNumVariables(); ++i)
  {
    size_t nameLength = thisClass->getMemberVariableName(i).length();
    if (nameLength > longestName)
      longestName = nameLength;
    size_t shortNameLength = thisClass->getMemberVariableShortName(i).length();
    if (shortNameLength > longestShortName)
      longestShortName = shortNameLength;
  }
  /* Generate usage */
  string argumentDescriptions = JUCE_T("-h") + string::repeatedString(JUCE_T(" "), longestShortName - 1)
                              + JUCE_T(" --help") + string::repeatedString(JUCE_T(" "), longestName - 4)
                              + JUCE_T(" Display this help message.\n");
  for (size_t i = 0; i < getNumVariables(); ++i)
  {
    
    argumentDescriptions += (thisClass->getMemberVariableShortName(i).isNotEmpty() ? JUCE_T("-") + thisClass->getMemberVariableShortName(i) : JUCE_T(" "))
                        + string::repeatedString(JUCE_T(" "), longestShortName - thisClass->getMemberVariableShortName(i).length())
                        + JUCE_T(" --") + thisClass->getMemberVariableName(i)
                        + string::repeatedString(JUCE_T(" "), longestName - thisClass->getMemberVariableName(i).length())
                        + JUCE_T(" ") + thisClass->getMemberVariableDescription(i) + JUCE_T("\n");
  }
  
  return toString() + JUCE_T("\n\n")
    + JUCE_T("Usage : ") + getClassName() + JUCE_T(" file.xml\n")
    + JUCE_T("   or : ") + getClassName() + JUCE_T(" [argument ...]\n\n")
    + JUCE_T("The arguments are as follows :\n\n")
    + argumentDescriptions;
}

/*
** CompositeWorkUnit
*/
ObjectPtr CompositeWorkUnit::run(ExecutionContext& context)
{
  const size_t n = getNumWorkUnits();
  OVectorPtr results = new OVector(objectClass, n);
  for (size_t i = 0; i < n; ++i)
  {
    WorkUnitPtr workUnit = getWorkUnit(i);
    ObjectPtr result = context.run(workUnit, pushChildrenIntoStack);
    results->setElement(i, result);
  }
  return results;
}
