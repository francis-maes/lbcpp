/*-----------------------------------------.---------------------------------.
| Filename: TemplateType.h                 | Parameterized Type Generator    |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2010 17:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/TemplateType.h>
#include <lbcpp/Data/Variable.h>
#include <lbcpp/Data/Vector.h>
using namespace lbcpp;

bool TemplateType::isInstanciatedTypeName(const String& name)
  {return name.indexOfChar('<') >= 0;}

static bool parseTypeList(const String& str, std::vector<String>& res, MessageCallback& callback)
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

bool TemplateType::parseInstanciatedTypeName(const String& typeName, String& templateName, std::vector<String>& arguments, MessageCallback& callback)
{
  int b = typeName.indexOfChar('<');
  int e = typeName.lastIndexOfChar('>');
  if (b < 0 || e < 0)
  {
    callback.errorMessage(T("TemplateType::parseInstanciatedTypeName"), T("Invalid type syntax: ") + typeName.quoted());
    return false;
  }
  templateName = typeName.substring(0, b).trim();
  if (!parseTypeList(typeName.substring(b + 1, e).trim(), arguments, callback))
    return false;
  return true;
}

bool TemplateType::parseInstanciatedTypeName(const String& typeName, String& templateName, std::vector<TypePtr>& templateArguments, MessageCallback& callback)
{
  std::vector<String> arguments; 
  if (!parseInstanciatedTypeName(typeName, templateName, arguments, callback))
    return false;

  templateArguments.resize(arguments.size());
  for (size_t i = 0; i < templateArguments.size(); ++i)
  {
    templateArguments[i] = Type::get(arguments[i], callback);
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

void DefaultTemplateType::addParameter(const String& name, const String& baseTypeName)
{
  TypePtr baseType = Type::get(baseTypeName);
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

static void tokenizeTypeName(const String& str, const String& separators, StringArray& res)
{
  String currentToken;
  for (int c = 0; c < str.length(); ++c)
  {
    if (separators.indexOfChar(str[c]) >= 0)
    {
      if (currentToken.isNotEmpty())
      {
        res.add(currentToken);
        currentToken = String::empty;
      }
      res.add(str.substring(c, c + 1));
    }
    else
      currentToken += str[c];
  }
  if (currentToken.isNotEmpty())
  {
    res.add(currentToken);
    currentToken = String::empty;
  }
}

TypePtr DefaultTemplateType::instantiateTypeName(const String& typeExpr, const std::vector<TypePtr>& arguments, MessageCallback& callback) const
{
  if (isInstanciatedTypeName(typeExpr))
  {
    String templateType;
    std::vector<String> templateArguments;
    if (!parseInstanciatedTypeName(typeExpr, templateType, templateArguments, callback))
      return false;

    std::vector<TypePtr> templateArgumentTypes(templateArguments.size());
    for (size_t i = 0; i < templateArgumentTypes.size(); ++i)
    {
      templateArgumentTypes[i] = instantiateTypeName(templateArguments[i], arguments, callback);
      if (!templateArgumentTypes[i])
        return false;
    }
    return Type::get(templateType, templateArgumentTypes, callback);
  }
  else
  {
    String name = typeExpr.trim();
    jassert(arguments.size() == parameters.size());
    int index = findParameter(name);
    return index >= 0 ? arguments[index] : Type::get(name, callback);
  }
}

TypePtr DefaultTemplateType::instantiate(const std::vector<TypePtr>& arguments, MessageCallback& callback) const
{
  if (!initialized && !const_cast<DefaultTemplateType* >(this)->initialize(callback))
    return TypePtr();
  TypePtr baseType = instantiateTypeName(baseTypeExpr, arguments, callback);
  return baseType ? instantiate(arguments, baseType, callback) : TypePtr();
}
