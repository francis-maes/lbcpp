/*-----------------------------------------.---------------------------------.
| Filename: TemplateClass.h                 | Parameterized Type Generator    |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2010 17:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core.h>
using namespace lbcpp;

bool TemplateClass::isInstanciatedTypeName(const String& name)
  {return name.indexOfChar('<') >= 0;}

namespace lbcpp
{
  bool parseListWithParenthesis(ExecutionContext& context, const String& str, char openParenthesis, char closeParenthesis, char comma, std::vector<String>& res)
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
      String identifier = str.substring(b, e);
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

static bool parseTypeList(ExecutionContext& context, const String& str, std::vector<String>& res)
  {return lbcpp::parseListWithParenthesis(context, str, '<', '>', ',', res);}

bool TemplateClass::parseInstanciatedTypeName(ExecutionContext& context, const String& typeName, String& templateName, std::vector<String>& arguments)
{
  int b = typeName.indexOfChar('<');
  int e = typeName.lastIndexOfChar('>');
  if (b < 0 || e < 0)
  {
    context.errorCallback(T("TemplateClass::parseInstanciatedTypeName"), T("Invalid type syntax: ") + typeName.quoted());
    return false;
  }
  templateName = typeName.substring(0, b).trim();
  if (!parseTypeList(context, typeName.substring(b + 1, e).trim(), arguments))
    return false;
  return true;
}

bool TemplateClass::parseInstanciatedTypeName(ExecutionContext& context, const String& typeName, String& templateName, std::vector<ClassPtr>& templateArguments)
{
  std::vector<String> arguments; 
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
  
String TemplateClass::makeInstanciatedTypeName(const String& typeName, const std::vector<ClassPtr>& arguments)
{
  jassert(typeName.isNotEmpty() && arguments.size());
  String res;
  for (size_t i = 0; i < arguments.size(); ++i)
  {
    ClassPtr arg = arguments[i];
    jassert(arg);
    if (i == 0)
      res = typeName + T("<");
    else
      res += T(", ");
    res += arg->getName();
  }
  res += T(">");
  return res;
}

/*
** DefaultTemplateClass
*/
DefaultTemplateClass::DefaultTemplateClass(const String& name, const String& baseTypeExpr)
  : TemplateClass(name), baseTypeExpr(baseTypeExpr) {}

size_t DefaultTemplateClass::getNumParameters() const
  {return parameters.size();}

String DefaultTemplateClass::getParameterName(size_t index) const
  {return parameters[index].first;}

ClassPtr DefaultTemplateClass::getParameterBaseType(size_t index) const
  {return parameters[index].second;}

void DefaultTemplateClass::addParameter(const String& name, ClassPtr baseType)
  {parameters.push_back(std::make_pair(name, baseType));}

void DefaultTemplateClass::addParameter(ExecutionContext& context, const String& name, const String& baseTypeName)
{
  ClassPtr baseType = typeManager().getType(context, baseTypeName);
  if (baseType)
    addParameter(name, baseType);
}

int DefaultTemplateClass::findParameter(const String& name) const
{
  for (size_t i = 0; i < parameters.size(); ++i)
    if (parameters[i].first == name)
      return (int)i;
  return -1;
}

ClassPtr DefaultTemplateClass::instantiateTypeName(ExecutionContext& context, const String& typeExpr, const std::vector<ClassPtr>& arguments) const
{
  if (isInstanciatedTypeName(typeExpr))
  {
    String templateType;
    std::vector<String> templateArguments;
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
    String name = typeExpr.trim();
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
