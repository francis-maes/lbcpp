/*-----------------------------------------.---------------------------------.
| Filename: TemplateClass.h                 | Parameterized Type Generator    |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2010 17:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <oil/Core.h>
using namespace lbcpp;

bool TemplateClass::isInstanciatedTypeName(const string& name)
  {return name.indexOfChar('<') >= 0;}

namespace lbcpp
{
  bool parseListWithParenthesis(ExecutionContext& context, const string& str, char openParenthesis, char closeParenthesis, char comma, std::vector<string>& res)
  {
    int b = 0;
    while (b < str.length())
    {
      int e = b;

      // eat identifier
      while (e < str.length() && str[e] != openParenthesis && str[e] != closeParenthesis && str[e] != comma)
        ++e;

      // eat subtypelist
      if (e < str.length() && str[e] == openParenthesis)
      {
        int depth = 1;
        for (++e; e < str.length(); ++e)
        {
          if (str[e] == openParenthesis)
            ++depth;
          else if (str[e] == closeParenthesis)
          {
            --depth;
            if (depth == 0)
            {
              ++e;
              break;
            }
          }
        }
      }
      string identifier = str.substring(b, e);
      jassert(identifier.trim() == identifier);
      res.push_back(identifier);

      // eat comma and spaces
      while (e < str.length() && (str[e] == comma || juce::CharacterFunctions::isWhitespace(str[e])))
        ++e;
      b = e;
    }
    return true;
  }

};

static bool parseTypeList(ExecutionContext& context, const string& str, std::vector<string>& res)
  {return lbcpp::parseListWithParenthesis(context, str, '<', '>', ',', res);}

bool TemplateClass::parseInstanciatedTypeName(ExecutionContext& context, const string& typeName, string& templateName, std::vector<string>& arguments)
{
  int b = typeName.indexOfChar('<');
  int e = typeName.lastIndexOfChar('>');
  if (b < 0 || e < 0)
  {
    context.errorCallback(JUCE_T("TemplateClass::parseInstanciatedTypeName"), JUCE_T("Invalid type syntax: ") + typeName.quoted());
    return false;
  }
  templateName = typeName.substring(0, b).trim();
  if (!parseTypeList(context, typeName.substring(b + 1, e).trim(), arguments))
    return false;
  return true;
}

bool TemplateClass::parseInstanciatedTypeName(ExecutionContext& context, const string& typeName, string& templateName, std::vector<ClassPtr>& templateArguments)
{
  std::vector<string> arguments; 
  if (!parseInstanciatedTypeName(context, typeName, templateName, arguments))
    return false;

  templateArguments.resize(arguments.size());
  for (size_t i = 0; i < templateArguments.size(); ++i)
  {
    templateArguments[i] = typeManager().getType(context, arguments[i]);
    if (!templateArguments[i])
      return false;
  }
  return true;
}
  
string TemplateClass::makeInstanciatedTypeName(const string& typeName, const std::vector<ClassPtr>& arguments)
{
  jassert(typeName.isNotEmpty() && arguments.size());
  string res;
  for (size_t i = 0; i < arguments.size(); ++i)
  {
    ClassPtr arg = arguments[i];
    jassert(arg);
    if (i == 0)
      res = typeName + JUCE_T("<");
    else
      res += JUCE_T(", ");
    res += arg->getName();
  }
  res += JUCE_T(">");
  return res;
}

/*
** DefaultTemplateClass
*/
DefaultTemplateClass::DefaultTemplateClass(const string& name, const string& baseTypeExpr)
  : TemplateClass(name), baseTypeExpr(baseTypeExpr) {}

size_t DefaultTemplateClass::getNumParameters() const
  {return parameters.size();}

string DefaultTemplateClass::getParameterName(size_t index) const
  {return parameters[index].first;}

ClassPtr DefaultTemplateClass::getParameterBaseType(size_t index) const
  {return parameters[index].second;}

void DefaultTemplateClass::addParameter(const string& name, ClassPtr baseType)
  {parameters.push_back(std::make_pair(name, baseType));}

void DefaultTemplateClass::addParameter(ExecutionContext& context, const string& name, const string& baseTypeName)
{
  ClassPtr baseType = typeManager().getType(context, baseTypeName);
  if (baseType)
    addParameter(name, baseType);
}

bool DefaultTemplateClass::inheritsFrom(ExecutionContext& context, const std::vector<ClassPtr>& arguments, const string& parameterName, const string& baseTypeName) const
{
  int index = findParameter(parameterName);
  jassert(index >= 0);
  ClassPtr baseType = typeManager().getType(context, baseTypeName);
  return arguments[index]->inheritsFrom(baseType);
}

int DefaultTemplateClass::findParameter(const string& name) const
{
  for (size_t i = 0; i < parameters.size(); ++i)
    if (parameters[i].first == name)
      return (int)i;
  return -1;
}

ClassPtr DefaultTemplateClass::instantiateTypeName(ExecutionContext& context, const string& typeExpr, const std::vector<ClassPtr>& arguments) const
{
  if (isInstanciatedTypeName(typeExpr))
  {
    string templateType;
    std::vector<string> templateArguments;
    if (!parseInstanciatedTypeName(context, typeExpr, templateType, templateArguments))
      return ClassPtr();

    std::vector<ClassPtr> templateArgumentTypes(templateArguments.size());
    for (size_t i = 0; i < templateArgumentTypes.size(); ++i)
    {
      templateArgumentTypes[i] = instantiateTypeName(context, templateArguments[i], arguments);
      if (!templateArgumentTypes[i])
        return ClassPtr();
    }
    return typeManager().getType(context, templateType, templateArgumentTypes);
  }
  else
  {
    string name = typeExpr.trim();
    jassert(arguments.size() == parameters.size());
    int index = findParameter(name);
    return index >= 0 ? arguments[index] : typeManager().getType(context, name);
  }
}

ClassPtr DefaultTemplateClass::instantiate(ExecutionContext& context, const std::vector<ClassPtr>& arguments) const
{
  if (!initialized && !const_cast<DefaultTemplateClass* >(this)->initialize(context))
    return ClassPtr();
  ClassPtr baseType = instantiateTypeName(context, baseTypeExpr, arguments);
  return baseType ? instantiate(context, arguments, baseType) : ClassPtr();
}
