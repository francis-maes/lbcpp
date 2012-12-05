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
    if (arg == T("--help") || arg == T("-h"))
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
  {return str.containsOnly(T("-+e0123456789"));}

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
    if (arguments[i].startsWith(T("--")))
    {
      int end = arguments[i].indexOfChar(T('='));
      if (end == -1)
        end = arguments[i].length();
      else
        argumentValue = arguments[i].substring(end + 1);

      argumentName = arguments[i].substring(2, end);
      namesMap = &variableNames;
    }
    else if (arguments[i].startsWith(T("-")))
    {
      argumentName = arguments[i].substring(1);
      namesMap = &variableShortNames;
    }

    if (!namesMap)
    {
      context.errorCallback(T("WorkUnit::parseArguments"), T("Unexpected expression : ") + arguments[i]);
      return false;
    }

    std::map<string, size_t>::const_iterator it = namesMap->find(argumentName);
    if (it == namesMap->end())
    {
      context.errorCallback(T("WorkUnit::parseArguments"), T("Unknown argument: ") + argumentName);
      return false;
    }

    size_t variableIndex = it->second;
    for (++i ; i < arguments.size() && (!arguments[i].startsWith(T("-")) || isNegativeNumber(arguments[i])); ++i)
      argumentValue += T(" ") + arguments[i];
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
        context.errorCallback(T("WorkUnit::parseArguments"), T("Incomprehensible value of") + argumentName.quoted() + T(" : ") + argumentValue);
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
  string argumentDescriptions = T("-h") + string::repeatedString(T(" "), longestShortName - 1)
                              + T(" --help") + string::repeatedString(T(" "), longestName - 4)
                              + T(" Display this help message.\n");
  for (size_t i = 0; i < getNumVariables(); ++i)
  {
    
    argumentDescriptions += (thisClass->getMemberVariableShortName(i).isNotEmpty() ? T("-") + thisClass->getMemberVariableShortName(i) : T(" "))
                        + string::repeatedString(T(" "), longestShortName - thisClass->getMemberVariableShortName(i).length())
                        + T(" --") + thisClass->getMemberVariableName(i)
                        + string::repeatedString(T(" "), longestName - thisClass->getMemberVariableName(i).length())
                        + T(" ") + thisClass->getMemberVariableDescription(i) + T("\n");
  }
  
  return toString() + T("\n\n")
    + T("Usage : ") + getClassName() + T(" file.xml\n")
    + T("   or : ") + getClassName() + T(" [argument ...]\n\n")
    + T("The arguments are as follows :\n\n")
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
