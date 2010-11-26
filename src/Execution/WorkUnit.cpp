/*-----------------------------------------.---------------------------------.
| Filename: WorkUnit.cpp                   | Base class for Work Units       |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 20:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/Variable.h>
#include <lbcpp/Execution/WorkUnit.h>
#include <lbcpp/Execution/ExecutionContext.h>
using namespace lbcpp;

int WorkUnit::main(ExecutionContext& context, WorkUnitPtr workUnit, int argc, char* argv[])
{
  std::vector<String> arguments(argc - 1);
  for (size_t i = 1; i < (size_t)argc; ++i)
  {
    String arg = argv[i];
    arguments[i - 1] = arg;
    if (arg == T("-h") || arg == T("--help"))
    {
      context.informationCallback(workUnit->getUsageString());
      return 0;
    }
  }

  return workUnit->parseArguments(context, arguments) && context.run(workUnit) ? 0 : 1;
}

bool WorkUnit::parseArguments(ExecutionContext& context, const std::vector<String>& arguments)
{
  /* shortcut */
  std::map<String, size_t> variableNames;
  std::map<String, size_t> variableShortNames;
  std::map<String, size_t>& defaultMap = variableNames;
  ClassPtr thisClass = getClass();
  for (size_t i = 0; i < getNumVariables(); ++i)
  {
    variableNames[thisClass->getObjectVariableName(i)] = i;
    variableShortNames[thisClass->getObjectVariableShortName(i)] = i;
  }
  
  for (size_t i = 0; i < arguments.size(); )
  {
    String argumentName;
    String argumentValue;
    if (arguments[i].startsWith(T("--")))
    {
      int end = arguments[i].indexOfChar(T('='));
      if (end == -1)
        end = arguments[i].length();
      else
        argumentValue = arguments[i].substring(end + 1);

      argumentName = arguments[i].substring(2, end);
    }
    else if (arguments[i].startsWith(T("-")))
    {
      argumentName = arguments[i].substring(1);
      defaultMap = variableShortNames;
    }

    if (!defaultMap.count(argumentName))
    {
      context.informationCallback(getUsageString());
      context.errorCallback(T("WorkUnit::parseArguments"), T("Unexpected expression : ") + arguments[i]);
      return false;
    }

    size_t variableIndex = defaultMap[argumentName];

    for (++i ; i < arguments.size() && !arguments[i].startsWith(T("-")); ++i)
      argumentValue += T(" ") + arguments[i];
    argumentValue = argumentValue.trim();

    TypePtr argumentType = getVariableType(variableIndex);
    Variable value;
    if (argumentType == booleanType && argumentValue.isEmpty())
      value = Variable(true); // particular case for boolean arguments: if no value has been given, we take true by default
    else
    {
      value = Variable::createFromString(context, argumentType, argumentValue);
      if (value.isMissingValue())
      {
        context.errorCallback(T("WorkUnit::parseArguments"), T("Incomprehensible value of") + argumentName.quoted() + T(" : ") + argumentValue);
        return false;
      }
    }
    setVariable(context, variableIndex, value);
  }
  return true;
}

String WorkUnit::getUsageString() const
{
  ClassPtr thisClass = getClass();
  /* Compute the longest string */
  size_t longestName = 4;
  size_t longestShortName = 1;
  for (size_t i = 0; i < getNumVariables(); ++i)
  {
    size_t nameLength = thisClass->getObjectVariableName(i).length();
    if (nameLength > longestName)
      longestName = nameLength;
    size_t shortNameLength = thisClass->getObjectVariableShortName(i).length();
    if (shortNameLength > longestShortName)
      longestShortName = shortNameLength;
  }
  /* Generate usage */
  String argumentDescriptions = T("-h") + String::repeatedString(T(" "), longestShortName - 1)
                              + T(" --help") + String::repeatedString(T(" "), longestName - 4)
                              + T(" Display this help message.\n");
  for (size_t i = 0; i < getNumVariables(); ++i)
  {
    
    argumentDescriptions += T("-") + thisClass->getObjectVariableShortName(i)
                        + String::repeatedString(T(" "), longestShortName - thisClass->getObjectVariableShortName(i).length())
                        + T(" --") + thisClass->getObjectVariableName(i)
                        + String::repeatedString(T(" "), longestName - thisClass->getObjectVariableName(i).length())
                        + T(" ") + thisClass->getObjectVariableDescription(i) + T("\n");
  }
  
  return toString() + T("\n\n")
    + T("Usage : ") + toShortString() + T(" file.xml\n")
    + T("   or : ") + toShortString() + T(" [argument ...]\n\n")
    + T("The arguments are as follows :\n\n")
    + argumentDescriptions;
}
