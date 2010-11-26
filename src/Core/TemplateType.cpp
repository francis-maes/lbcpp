/*-----------------------------------------.---------------------------------.
| Filename: TemplateType.h                 | Parameterized Type Generator    |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2010 17:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/TemplateType.h>
#include <lbcpp/Core/Variable.h>
#include <lbcpp/Data/Vector.h>
using namespace lbcpp;

bool TemplateType::isInstanciatedTypeName(const String& name)
  {return name.indexOfChar('<') >= 0;}

static bool parseTypeList(ExecutionContext& context, const String& str, std::vector<String>& res)
{
  int b = 0;
  while (b < str.length())
  {
    int e = b;

    // eat identifier
    while (e < str.length() && juce::CharacterFunctions::isLetterOrDigit(str[e]))
      ++e;

    // eat subtypelist
    if (e < str.length() && str[e] == '<')
    {
      int depth = 1;
      for (++e; e < str.length(); ++e)
      {
        if (str[e] == '<')
          ++depth;
        else if (str[e] == '>')
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
    while (e < str.length() && !juce::CharacterFunctions::isLetterOrDigit(str[e]))
      ++e;

    b = e;
  }
  return true;
}

bool TemplateType::parseInstanciatedTypeName(ExecutionContext& context, const String& typeName, String& templateName, std::vector<String>& arguments)
{
  int b = typeName.indexOfChar('<');
  int e = typeName.lastIndexOfChar('>');
  if (b < 0 || e < 0)
  {
    context.errorCallback(T("TemplateType::parseInstanciatedTypeName"), T("Invalid type syntax: ") + typeName.quoted());
    return false;
  }
  templateName = typeName.substring(0, b).trim();
  if (!parseTypeList(context, typeName.substring(b + 1, e).trim(), arguments))
    return false;
  return true;
}

bool TemplateType::parseInstanciatedTypeName(ExecutionContext& context, const String& typeName, String& templateName, std::vector<TypePtr>& templateArguments)
{
  std::vector<String> arguments; 
  if (!parseInstanciatedTypeName(context, typeName, templateName, arguments))
    return false;

  templateArguments.resize(arguments.size());
  for (size_t i = 0; i < templateArguments.size(); ++i)
  {
    templateArguments[i] = context.getType(arguments[i]);
    if (!templateArguments[i])
      return false;
  }
  return true;
}
  
String TemplateType::makeInstanciatedTypeName(const String& typeName, const std::vector<TypePtr>& arguments)
{
  jassert(typeName.isNotEmpty() && arguments.size());
  String res;
  for (size_t i = 0; i < arguments.size(); ++i)
  {
    TypePtr arg = arguments[i];
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
** DefaultTemplateType
*/
DefaultTemplateType::DefaultTemplateType(const String& name, const String& baseTypeExpr)
  : TemplateType(name), baseTypeExpr(baseTypeExpr) {}

size_t DefaultTemplateType::getNumParameters() const
  {return parameters.size();}

String DefaultTemplateType::getParameterName(size_t index) const
  {return parameters[index].first;}

TypePtr DefaultTemplateType::getParameterBaseType(size_t index) const
  {return parameters[index].second;}

void DefaultTemplateType::addParameter(const String& name, TypePtr baseType)
  {parameters.push_back(std::make_pair(name, baseType));}

void DefaultTemplateType::addParameter(ExecutionContext& context, const String& name, const String& baseTypeName)
{
  TypePtr baseType = context.getType(baseTypeName);
  if (baseType)
    addParameter(name, baseType);
}

int DefaultTemplateType::findParameter(const String& name) const
{
  for (size_t i = 0; i < parameters.size(); ++i)
    if (parameters[i].first == name)
      return (int)i;
  return -1;
}

TypePtr DefaultTemplateType::instantiateTypeName(ExecutionContext& context, const String& typeExpr, const std::vector<TypePtr>& arguments) const
{
  if (isInstanciatedTypeName(typeExpr))
  {
    String templateType;
    std::vector<String> templateArguments;
    if (!parseInstanciatedTypeName(context, typeExpr, templateType, templateArguments))
      return false;

    std::vector<TypePtr> templateArgumentTypes(templateArguments.size());
    for (size_t i = 0; i < templateArgumentTypes.size(); ++i)
    {
      templateArgumentTypes[i] = instantiateTypeName(context, templateArguments[i], arguments);
      if (!templateArgumentTypes[i])
        return false;
    }
    return context.getType(templateType, templateArgumentTypes);
  }
  else
  {
    String name = typeExpr.trim();
    jassert(arguments.size() == parameters.size());
    int index = findParameter(name);
    return index >= 0 ? arguments[index] : context.getType(name);
  }
}

TypePtr DefaultTemplateType::instantiate(ExecutionContext& context, const std::vector<TypePtr>& arguments) const
{
  if (!initialized && !const_cast<DefaultTemplateType* >(this)->initialize(context))
    return TypePtr();
  TypePtr baseType = instantiateTypeName(context, baseTypeExpr, arguments);
  return baseType ? instantiate(context, arguments, baseType) : TypePtr();
}
