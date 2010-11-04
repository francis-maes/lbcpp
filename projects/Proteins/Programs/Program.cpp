
#include <lbcpp/lbcpp.h>
#include "Programs/Program.h"

using namespace lbcpp;

bool Program::parseArguments(const std::vector<String>& arguments, MessageCallback& callback)
{
  if (helpRequired(arguments))
  {
    callback.infoMessage(T("Program::parseArguments"), getUsage());
    return false;
  }
  
  /* shortcut */
  std::map<String, size_t> variableNames;
  std::map<String, size_t> variableShortNames;
  std::map<String, size_t>& defaultMap = variableNames;
  for (size_t i = 0; i < getNumVariables(); ++i)
  {
    variableNames[getVariableName(i)] = i;
    //variableShortNames[getVariableShortName(i)] = i; // FIXME
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
      callback.infoMessage(T("Program::parseArguments"), getUsage());
      callback.errorMessage(T("Program::parseArguments"), T("Unexpected expression : ") + arguments[i]);
      return false;
    }

    size_t variableIndex = defaultMap[argumentName];

    for (++i ; i < arguments.size() && !arguments[i].startsWith(T("-")); ++i)
      argumentValue += T(" ") + arguments[i];
    argumentValue = argumentValue.trim();

    Variable value = Variable::createFromString(getVariableType(variableIndex), argumentValue, callback);
    if (value.isMissingValue())
    {
      callback.errorMessage(T("Program::parseArguments"), T("Incomprehensible value of") + argumentName.quoted() + T(" : ") + argumentValue);
      return false;
    }

    setVariable(variableIndex, value);
  }

  return true;
}
  
String Program::getUsage() const
{
  String argumentDescriptions = T("-h --help Display this help message.\n");
  for (size_t i = 0; i < getNumVariables(); ++i)
    argumentDescriptions += T("--") + getVariableName(i) + T(" ") /*+ getVariableDescription(i)*/ + T("\n");  // FIXME
  
  return toString() + T("\n\n")
    + T("Usage : ") + toShortString() + T(" file.xml\n")
    + T("   or : ") + toShortString() + T(" [argument ...]\n\n")
    + T("The arguments are as follows :\n\n")
    + argumentDescriptions;
}

bool Program::helpRequired(const std::vector<String>& arguments) const
{
  for (size_t i = 0; i < arguments.size(); ++i)
    if (arguments[i] == T("-h") || arguments[i] == T("--help"))
      return true;
  return false;
}
