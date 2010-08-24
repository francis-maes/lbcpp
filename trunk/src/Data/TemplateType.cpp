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

static bool parseTypeList(const String& str, std::vector<TypePtr>& res, ErrorHandler& callback)
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
    TypePtr type = Type::get(identifier, callback);
    if (!type)
      return false;
    res.push_back(type);

    // eat comma and spaces
    while (e < str.length() && !juce::CharacterFunctions::isLetterOrDigit(str[e]))
      ++e;

    b = e;
  }
  return true;
}

bool TemplateType::parseInstanciatedTypeName(const String& typeName, String& templateName, std::vector<TypePtr>& templateArguments, ErrorHandler& callback)
{
  int b = typeName.indexOfChar('<');
  int e = typeName.lastIndexOfChar('>');
  if (b < 0 || e < 0)
  {
    callback.errorMessage(T("TemplateType::parseInstanciatedTypeName"), T("Invalid type syntax: ") + typeName.quoted());
    return false;
  }
  templateName = typeName.substring(0, b);
  return parseTypeList(typeName.substring(b + 1, e), templateArguments, callback);
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

TypePtr DefaultTemplateType::computeBaseType(const std::vector<TypePtr>& arguments, ErrorHandler& callback) const
{
  StringArray tokens;
  //std::cout << "baseTypeExpr: " << baseTypeExpr.quoted() << std::endl;
  tokenizeTypeName(baseTypeExpr, T("<>,"), tokens);
  for (int i = 0; i < tokens.size(); ++i)
  {
    String token = tokens[i].trim();
    //std::cout << "Token " << i << ": " << token.quoted() << std::endl;
    int parameterIndex = findParameter(token);
    if (parameterIndex >= 0)
    {
      jassert(parameterIndex < (int)arguments.size());
      TypePtr arg = arguments[parameterIndex];
      jassert(arg);
      tokens.set(i, arg->getName());
      //std::cout << " replace " << token << " by " << arg->getName() << std::endl;
    }
  }
  String baseTypeName = tokens.joinIntoString(String::empty);
  //std::cout << " ==> " << baseTypeName.quoted() << std::endl;
  return Type::get(baseTypeName, callback);
}

TypePtr DefaultTemplateType::instantiate(const std::vector<TypePtr>& arguments, ErrorHandler& callback) const
{
  if (!initialized && !const_cast<DefaultTemplateType* >(this)->initialize(callback))
    return TypePtr();
  TypePtr baseType = computeBaseType(arguments, callback);
  return baseType ? instantiate(arguments, baseType, callback) : TypePtr();
}
