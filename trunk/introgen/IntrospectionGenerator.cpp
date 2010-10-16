/*-----------------------------------------.---------------------------------.
| Filename: IntrospectionGenerator.cpp     | Introspection Generator         |
| Author  : Francis Maes                   |                                 |
| Started : 18/08/2010 17:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "../juce/juce_amalgamated.h"
#include <map>
#include <vector>
#include <iostream>

static File inputFile;

class CppCodeGenerator
{
public:
  CppCodeGenerator(XmlElement* xml, OutputStream& ostr) : xml(xml), ostr(ostr)
  {
    fileName = xml->getStringAttribute(T("name"), T("???"));
    directoryName = xml->getStringAttribute(T("directory"), String::empty);
  }

  void generate()
  {
    indentation = 0;
    generateHeader();
    newLine();
    generateCodeForChildren(xml);
    newLine();
    generateFooter();
    newLine();
  }

protected:
  static String xmlTypeToCppType(const String& typeName)
    {return typeName.replaceCharacters(T("[]"), T("<>"));}

  static String typeToRefCountedPointerType(const String& typeName)
  {
    String str = xmlTypeToCppType(typeName);
    int i = str.indexOfChar('<');
    if (i >= 0)
      str = str.substring(0, i);
    return str + T("Ptr");
  }

  static String replaceFirstLettersByLowerCase(const String& str)
  {
    if (str.isEmpty())
      return String::empty;
    int numUpper = 0;
    for (numUpper = 0; numUpper < str.length(); ++numUpper)
      if (!CharacterFunctions::isUpperCase(str[numUpper]))
        break;

    if (numUpper == 0)
      return str;

    String res = str;
    if (numUpper == 1)
      res[0] += 'a' - 'A';
    else
      for (int i = 0; i < numUpper - 1; ++i)
        res[i] += 'a' - 'A';
    return res;
  }

  void generateCodeForChildren(XmlElement* xml)
  {
    for (XmlElement* elt = xml->getFirstChildElement(); elt; elt = elt->getNextElement())
    {
      String tag = elt->getTagName();
      if (tag == T("include"))
        generateInclude(elt);
      else if (tag == T("type") || tag == T("templateType"))
        generateTypeDeclaration(elt);
      else if (tag == T("class"))
        generateClassDeclaration(elt, false);
      else if (tag == T("template"))
      {
        generateClassDeclaration(elt, true);
        newLine();
        generateTemplateClassDeclaration(elt);
      }
      else if (tag == T("enumeration"))
        generateEnumerationDeclaration(elt);
      else if (tag == T("namespace"))
        generateNamespaceDeclaration(elt);
      else if (tag == T("code"))
        generateCode(elt);
      else if (tag == T("import") || tag == T("declarationCode"))
        continue;
      else
        std::cerr << "Warning: unrecognized tag: " << (const char* )tag << std::endl;
    }
  }

  /*
  ** Header
  */
  void generateHeader()
  {
    // header
    ostr << "/* ====== Introspection for file '" << fileName << "', generated on "
      << Time::getCurrentTime().toString(true, true, false) << " ====== */";
    writeLine(T("#include <lbcpp/Data/Variable.h>"));

    OwnedArray<File> headerFiles;
    File directory = inputFile.getParentDirectory();
    directory.findChildFiles(headerFiles, File::findFiles, false, T("*.h"));
    for (int i = 0; i < headerFiles.size(); ++i)
    {
      String path = directoryName;
      if (path.isNotEmpty())
        path += T("/");
      path += headerFiles[i]->getRelativePathFrom(directory).replaceCharacter('\\', '/');
      writeLine(T("#include ") + path.quoted());
    }
  }

  /*
  ** Include
  */
  void generateInclude(XmlElement* xml)
  {
    writeLine(T("#include ") + xml->getStringAttribute(T("file"), T("???")).quoted());
  }

  /*
  ** Type (implemented in C++)
  */
  void generateTypeDeclaration(XmlElement* xml)
  {
    String typeName = xml->getStringAttribute(T("name"), T("???"));
    String baseTypeName = xmlTypeToCppType(xml->getStringAttribute(T("base"), T("Variable")));
    String suffix = xml->getTagName() == T("type") ? T("Type") : T("TemplateType");
    String implementation = xml->getStringAttribute(T("implementation"), String::empty);

    if (implementation.isEmpty())
    {
      implementation = typeName + suffix;
      openClass(implementation, T("Type"));
      
      writeLine(implementation + T("(const String& name, TypePtr baseType)"));
      writeLine(T(": Type(name, baseType) {}"), 1);
      
      forEachXmlChildElementWithTagName(*xml, elt, T("code"))
        {generateCode(elt); newLine();}

      closeClass();
    }

    String currentScope = getCurrentScopeFullName();
    String fullName = currentScope + T("::") + implementation + T("(T(") + typeName.quoted() + T(")");
    if (baseTypeName.isNotEmpty())
    {
      fullName += T(", ");
      if (xml->getTagName() == T("type"))
        fullName += T("lbcpp::Type::get(T(") + baseTypeName.quoted() + T("))");
      else
        fullName += T("T(") + baseTypeName.quoted() + T(")");
    }
    fullName += T(")");

    String singletonVariableName = replaceFirstLettersByLowerCase(typeName) + T("Type");

    types.push_back(std::make_pair(currentScope + T("::") + singletonVariableName, fullName));

    // Type declarator
    if (xml->getTagName() == T("type"))
      writeLine(T("TypePtr ") + singletonVariableName + T(";"));
      //writeShortFunction(T("TypePtr ") + replaceFirstLettersByLowerCase(typeName) + T("Type()"),
      //    T("static TypeCache cache(T(") + typeName.quoted() + T(")); return cache();"));
  }

  /*
  ** Enumeration
  */
  void generateEnumValueInConstructor(XmlElement* xml)
  {
    String name = xml->getStringAttribute(T("name"), T("???"));
    String oneLetterCode = xml->getStringAttribute(T("oneLetterCode"), String::empty);
    String threeLettersCode = xml->getStringAttribute(T("threeLettersCode"), String::empty);
    writeLine(T("addElement(T(") + name.quoted() + T("), T(") + oneLetterCode.quoted() + T("), T(") + threeLettersCode.quoted() + T("));"));
  }

  void generateEnumerationDeclaration(XmlElement* xml)
  {
    String enumName = xml->getStringAttribute(T("name"), T("???"));
    String declaratorName = replaceFirstLettersByLowerCase(enumName) + T("Enumeration");
    String className = enumName + T("Enumeration");

    String currentScope = getCurrentScopeFullName();
    if (currentScope.isNotEmpty())
      currentScope += T("::");

    currentScopes.push_back(enumName);
    
    types.push_back(std::make_pair(currentScope + declaratorName, currentScope + className));
    openClass(className, T("Enumeration"));

    // constructor
    openScope(className + T("() : Enumeration(T(") + enumName.quoted() + T("))"));
    forEachXmlChildElementWithTagName(*xml, elt, T("value"))
      generateEnumValueInConstructor(elt);
    closeScope();

    closeClass();
    currentScopes.pop_back();

    // enum declarator
    writeLine(T("EnumerationPtr ") + declaratorName + T(";"));
    //writeShortFunction(T("EnumerationPtr ") + declaratorName + T("()"),
    //  T("static TypeCache cache(T(") + enumName.quoted() + T(")); return (EnumerationPtr)cache();"));
  }

  /*
  ** Class
  */
  void generateClassDeclaration(XmlElement* xml, bool isTemplate)
  {
    String className = xml->getStringAttribute(T("name"), T("???"));
    String baseClassName = xmlTypeToCppType(xml->getStringAttribute(T("base"), T("Object")));
    bool isAbstract = xml->getBoolAttribute(T("abstract"), false);
    bool hasAutoImplementation = xml->getBoolAttribute(T("autoimpl"), false);
    String classNameWithFirstLowerCase = replaceFirstLettersByLowerCase(className);
    String classBaseClass = hasAutoImplementation ? T("DynamicClass") : T("DefaultClass");

    String currentScope = getCurrentScopeFullName();
    if (currentScope.isNotEmpty())
      currentScope += T("::");

    currentScopes.push_back(className);
    if (!isTemplate)
      types.push_back(std::make_pair(currentScope + classNameWithFirstLowerCase + T("Class"), currentScope + className + T("Class()")));

    openClass(className + T("Class"), classBaseClass);

    // constructor
    std::vector<XmlElement* > variables;
    if (isTemplate)
      openScope(className + T("Class(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)")
        + T(" : ") + classBaseClass + T("(templateType, templateArguments, baseClass)"));
    else
      openScope(className + T("Class() : ") + classBaseClass + T("(T(") + className.quoted() + T("), T(") + baseClassName.quoted() + T("))"));
    closeScope();
    newLine();

    openScope(T("virtual bool initialize(MessageCallback& callback)"));
      forEachXmlChildElementWithTagName(*xml, elt, T("variable"))
      {
        generateVariableDeclarationInConstructor(className, elt);
        variables.push_back(elt);
      }
      writeLine(T("return ") + classBaseClass + T("::initialize(callback);"));
    closeScope();
    newLine();

    // create() function
    if (!isAbstract && !hasAutoImplementation)
    {
      openScope(T("virtual VariableValue create() const"));
        writeLine(className + T("* res = new ") + className + T("();"));
        writeLine(T("res->setThisClass(nativePointerFromThis(this));"));
        writeLine(T("return res;"));
      closeScope();
      newLine();
    }

    // getStaticVariableReference() function
    if (variables.size() && !xml->getBoolAttribute(T("manualAccessors"), false))
    {
      // getObjectVariable
      openScope(T("virtual Variable getObjectVariable(const Object* __thisbase__, size_t __index__) const"));
        writeLine(T("TypePtr expectedType = getObjectVariableType(__index__);"));
        writeLine(T("if (__index__ < baseType->getObjectNumVariables())"));
        writeLine(T("return baseType->getObjectVariable(__thisbase__, __index__);"), 1);
        writeLine(T("__index__ -= baseType->getObjectNumVariables();"));
        writeLine(T("const ") + className + T("* __this__ = static_cast<const ") + className + T("* >(__thisbase__);"));
        writeLine(T("Variable __res__;"));
        newLine();
        openScope(T("switch (__index__)"));
          for (size_t i = 0; i < variables.size(); ++i)
          {
            String name = variables[i]->getStringAttribute(T("var"), String::empty);
            if (name.isEmpty())
              name = variables[i]->getStringAttribute(T("name"), T("???"));

            String code = T("case ") + String((int)i) + T(": lbcpp::nativeToVariable(__res__, ");
            bool isEnumeration = variables[i]->getBoolAttribute(T("enumeration"), false);
            if (isEnumeration)
            {
              code += T("Variable((int)__this__->") + name + T(", ")
                + replaceFirstLettersByLowerCase(variables[i]->getStringAttribute(T("type"))) + T("Enumeration)");
            }
            else
              code += T("__this__->") + name;

            code += T(", expectedType); break;");
            writeLine(code, -1);
          }
          writeLine(T("default: jassert(false); return Variable();"), -1);
        closeScope();
        writeLine(T("jassert(!__res__.isNil());"));
        writeLine(T("return __res__;"));
      closeScope();
      newLine();

      // setObjectVariable
      openScope(T("virtual void setObjectVariable(Object* __thisbase__, size_t __index__, const Variable& __subValue__) const"));
        writeLine(T("if (__index__ < baseType->getObjectNumVariables())"));
        writeLine(T("{baseType->setObjectVariable(__thisbase__, __index__, __subValue__); return;}"), 1);
        writeLine(T("__index__ -= baseType->getObjectNumVariables();"));
        writeLine(className + T("* __this__ = static_cast<") + className + T("* >(__thisbase__);"));
        newLine();
        openScope(T("switch (__index__)"));
          for (size_t i = 0; i < variables.size(); ++i)
          {
            String name = variables[i]->getStringAttribute(T("var"), String::empty);
            if (name.isEmpty())
              name = variables[i]->getStringAttribute(T("name"), T("???"));

            String code = T("case ") + String((int)i) + T(": lbcpp::variableToNative(");

            bool isEnumeration = variables[i]->getBoolAttribute(T("enumeration"), false);
            if (isEnumeration)
              code += T("(int& )(__this__->") + name + T(")");
            else
              code += T("__this__->") + name;
            code += T(", __subValue__); break;");
            writeLine(code, -1);
          }
          writeLine(T("default: jassert(false);"), -1);
        closeScope();
      closeScope();
    }
   
    forEachXmlChildElementWithTagName(*xml, elt, T("code"))
      {generateCode(elt); newLine();}

    closeClass();
    currentScopes.pop_back();

    // class declarator
    if (!isTemplate)
    {
      writeLine(T("ClassPtr ") + classNameWithFirstLowerCase + T("Class;"));
      //writeShortFunction(T("ClassPtr ") + classNameWithFirstLowerCase + T("Class()"),
      //  T("static TypeCache cache(T(") + className.quoted() + T(")); return cache();"));

      // class constructors
      forEachXmlChildElementWithTagName(*xml, elt, T("constructor"))
        generateClassConstructorMethod(elt, className, baseClassName);
    }
  }

  void generateVariableDeclarationInConstructor(const String& className, XmlElement* xml)
  {
    String type = xmlTypeToCppType(xml->getStringAttribute(T("type"), T("???")));
    String name = xml->getStringAttribute(T("name"), T("???"));
    
    String typeArgument = (type == className ? T("this") : T("T(") + type.quoted() + T(")"));
    writeLine(T("addVariable(") + typeArgument + T(", T(") + name.quoted() + T("));"));
  }

  void generateClassConstructorMethod(XmlElement* xml, const String& className, const String& baseClassName)
  {
    String arguments = xml->getStringAttribute(T("arguments"), String::empty);
    String parameters = xml->getStringAttribute(T("parameters"), String::empty);
    String returnType = xml->getStringAttribute(T("returnType"), String::empty);
    
    if (returnType.isEmpty())
      returnType = baseClassName;

    StringArray tokens;
    tokens.addTokens(arguments, T(","), NULL);
    String argNames;
    for (int i = 0; i < tokens.size(); ++i)
    {
      String argName = tokens[i];
      int n = argName.lastIndexOfChar(' ');
      if (n >= 0)
        argName = argName.substring(n + 1);
      if (argNames.isNotEmpty())
        argNames += T(", ");
      argNames += argName;
    }

    // class declarator
    String classNameWithFirstLowerCase = replaceFirstLettersByLowerCase(className);
    String returnTypePtr = typeToRefCountedPointerType(returnType);
    openScope(returnTypePtr + T(" ") + classNameWithFirstLowerCase + T("(") + arguments + T(")"));
      writeLine(returnTypePtr + T(" res = new ") + className + T("(") + argNames + T(");"));
      String code = T("res->setThisClass(") + classNameWithFirstLowerCase + T("Class");
      if (parameters.isNotEmpty())
        code += T("(") + parameters + T(")");
      writeLine(code + T(");"));
      writeLine(T("return res;"));
    closeScope();
    newLine();
  }

  /*
  ** Template
  */
  void generateTemplateClassDeclaration(XmlElement* xml)
  {
    String className = xml->getStringAttribute(T("name"), T("???"));
    String baseClassName = xmlTypeToCppType(xml->getStringAttribute(T("base"), T("Object")));
    
    currentScopes.push_back(className);
    types.push_back(std::make_pair(String::empty, getCurrentScopeFullName() + T("TemplateClass()")));

    openClass(className + T("TemplateClass"), T("DefaultTemplateType"));

    // constructor
    openScope(className + T("TemplateClass() : DefaultTemplateType(T(") + className.quoted() + T("), T(") + baseClassName.quoted() + T("))"));
    closeScope();
    newLine();

    // initialize()
    openScope(T("virtual bool initialize(MessageCallback& callback)"));
      std::vector<XmlElement* > parameters;
      forEachXmlChildElementWithTagName(*xml, elt, T("parameter"))
      {
        generateParameterDeclarationInConstructor(className, elt);
        parameters.push_back(elt);
      }
      writeLine(T("return DefaultTemplateType::initialize(callback);"));
    closeScope();
    newLine();

    // instantiate
    openScope(T("virtual TypePtr instantiate(const std::vector<TypePtr>& arguments, TypePtr baseType, MessageCallback& callback) const"));
      writeLine(T("return new ") + className + T("Class(refCountedPointerFromThis(this), arguments, baseType);"));
    closeScope();
    newLine();

    closeClass();
    currentScopes.pop_back();

    // class declarator
    String classNameWithFirstLowerCase = replaceFirstLettersByLowerCase(className);
    if (parameters.size() == 0)
      std::cerr << "Error: No parameters in template. Type = " << (const char *)className << std::endl;
    else if (parameters.size() == 1)
      writeShortFunction(T("ClassPtr ") + classNameWithFirstLowerCase + T("Class(TypePtr type)"),
          T("return lbcpp::Type::get(T(") + className.quoted() + T("), std::vector<TypePtr>(1, type));"));
          //T("static UnaryTemplateTypeCache cache(T(") + className.quoted() + T(")); return cache(type);"));
    else if (parameters.size() == 2)
      writeShortFunction(T("ClassPtr ") + classNameWithFirstLowerCase + T("Class(TypePtr type1, TypePtr type2)"),
        T("std::vector<TypePtr> types(2); types[0] = type1; types[1] = type2; return lbcpp::Type::get(T(") + className.quoted() + T("), types);"));
          //T("static BinaryTemplateTypeCache cache(T(") + className.quoted() + T(")); return cache(type1, type2);"));
    else
      std::cerr << "Error: Class declarator with more than 2 parameters is not implemented yet. Type: "
		<< (const char* )className << ", NumParams = " << parameters.size() << std::endl;

    // class constructors
    forEachXmlChildElementWithTagName(*xml, elt, T("constructor"))
      generateClassConstructorMethod(elt, className, baseClassName);
  }

  void generateParameterDeclarationInConstructor(const String& className, XmlElement* xml)
  {
    String type = xmlTypeToCppType(xml->getStringAttribute(T("type"), T("Variable")));
    String name = xml->getStringAttribute(T("name"), T("???"));
    writeLine(T("addParameter(T(") + name.quoted() + T("), T(") + type.quoted() + T("));"));
  }

  /*
  ** Namespace
  */
  void generateNamespaceDeclaration(XmlElement* xml)
  {
    String name = xml->getStringAttribute(T("name"), T("???"));
    newLine();
    openScope(T("namespace ") + name);

    currentScopes.push_back(name);
    generateCodeForChildren(xml);
    currentScopes.pop_back();

    closeScope(T("; /* namespace ") + name + T(" */"));
  }

  /*
  ** Code
  */
  void generateCode(XmlElement* elt)
  {
    StringArray lines;
    lines.addTokens(elt->getAllSubText(), T("\n"), NULL);
    int minimumSpaces = 0x7FFFFFFF;
    for (int i = 0; i < lines.size(); ++i)
    {
      String line = lines[i];
      int numSpaces = line.length() - line.trimStart().length();
      if (numSpaces && line.substring(numSpaces).trim().isNotEmpty())
        minimumSpaces = jmin(numSpaces, minimumSpaces);
    }

    int spaces = minimumSpaces == 0x7FFFFFFF ? 0 : minimumSpaces;
    for (int i = 0; i < lines.size(); ++i)
    {
      String line = lines[i];
      int numSpaces = line.length() - line.trimStart().length();
      writeLine(lines[i].substring(jmin(numSpaces, spaces)));
    }
  }

  /*
  ** Footer
  */
  void generateFooter()
  {
    bool hasImports = false;
    forEachXmlChildElementWithTagName(*xml, elt, T("import"))
    {
      String name = elt->getStringAttribute(T("name"), T("???"));
      writeLine(T("extern void declare") + name + T("Classes();"));
      hasImports = true;
    }
    if (hasImports)
      newLine();

    openScope(T("void declare") + fileName + T("Classes()"));
    
    if (hasImports)
    {
      newLine();
      forEachXmlChildElementWithTagName(*xml, elt, T("import"))
        if (elt->getBoolAttribute(T("pre"), false))
        {
          String name = elt->getStringAttribute(T("name"), T("???"));
          writeLine(T("declare") + name + T("Classes();"));
        }
    }

    for (size_t i = 0; i < types.size(); ++i)
    {
      String classVariableName = types[i].first;
      String typeName = types[i].second;
      String code = T("lbcpp::Type::declare(");
      if (classVariableName.isNotEmpty())
        code += classVariableName + T(" = ");
      code += T("new ") + typeName + T(");");
      writeLine(code);
    }

    if (hasImports)
    {
      newLine();
      forEachXmlChildElementWithTagName(*xml, elt, T("import"))
        if (!elt->getBoolAttribute(T("pre"), false))
        {
          String name = elt->getStringAttribute(T("name"), T("???"));
          writeLine(T("declare") + name + T("Classes();"));
        }
    }

    forEachXmlChildElementWithTagName(*xml, elt, T("declarationCode"))
    {
      writeLine(elt->getAllSubText());
    }
   
    closeScope();
  }

private:
  XmlElement* xml;
  OutputStream& ostr;
  String fileName;
  String directoryName;
  int indentation;
  
  std::vector<String> currentScopes;

  std::vector<std::pair<String, String> > types;
  
  String getCurrentScopeFullName() const
  {
    String res;
    for (size_t i = 0; i < currentScopes.size(); ++i)
    {
      if (res.isNotEmpty())
        res += T("::");
      res += currentScopes[i];
    }
    return res;
  }


  void newLine(int indentationOffset = 0)
  {
    ostr << "\n";
    for (int i = 0; i < jmax(0, indentation + indentationOffset); ++i)
      ostr << "  ";
  }

  void writeLine(const String& line, int indentationOffset = 0)
    {newLine(indentationOffset); ostr << line;}

  void openScope(const String& declaration)
  {
    newLine();
    ostr << declaration;
    newLine();
    ostr << "{";
    ++indentation;
  }

  void closeScope(const String& closingText = String::empty)
  {
    jassert(indentation > 0);
    --indentation;
    newLine();
    ostr << "}" << closingText;
  }

  void openClass(const String& className, const String& baseClass)
  {
    openScope(T("class ") + className + T(" : public ") + baseClass);
    writeLine(T("public:"), -1);
  }

  void closeClass()
    {writeLine(T("juce_UseDebuggingNewOperator")); closeScope(T(";")); newLine();}

  void writeShortFunction(const String& declaration, const String& oneLineBody)
  {
    writeLine(declaration);
    writeLine(T("{") + oneLineBody + T("}"), 1);
    newLine();
  }
};

class XmlMacros
{
public:
  void registerMacrosRecursively(const XmlElement* xml)
  {
    if (xml->getTagName() == T("defmacro"))
      m[xml->getStringAttribute(T("name"))] = xml;
    else
      for (int i = 0; i < xml->getNumChildElements(); ++i)
        registerMacrosRecursively(xml->getChildElement(i));
  }

  typedef std::map<String, String> variables_t;

  String processText(const String& str, const variables_t& variables)
  {
    String res = str;
    for (std::map<String, String>::const_iterator it = variables.begin(); it != variables.end(); ++it)
      res = res.replace(T("%{") +  it->first + T("}"), it->second);
    return res;
  }

  // returns a new XmlElement
  XmlElement* process(const XmlElement* xml, const variables_t& variables = variables_t())
  {
    if (xml->getTagName() == T("defmacro"))
      return NULL;

    jassert(xml->getTagName() != T("macro"));

    if (xml->isTextElement())
      return XmlElement::createTextElement(processText(xml->getText(), variables));

    XmlElement* res = new XmlElement(xml->getTagName());
    for (int i = 0; i < xml->getNumAttributes(); ++i)
      res->setAttribute(xml->getAttributeName(i), processText(xml->getAttributeValue(i), variables));

    for (int i = 0; i < xml->getNumChildElements(); ++i)
    {
      XmlElement* child = xml->getChildElement(i);
      if (child->getTagName() == T("macro"))
      {
        String name = processText(child->getStringAttribute(T("name")), variables);
        macros_t::iterator it = m.find(name);
        if (it == m.end())
        {
          std::cerr << "Could not find macro " << (const char* )name << std::endl;
          return NULL;
        }
        else
        {
          variables_t nvariables(variables);
          for (int j = 0; j < child->getNumAttributes(); ++j)
            if (child->getAttributeName(j) != T("name"))
              nvariables[child->getAttributeName(j)] = child->getAttributeValue(j);
          const XmlElement* macro = it->second;
          for (int j = 0; j < macro->getNumChildElements(); ++j)
            addChild(res, process(macro->getChildElement(j), nvariables));
        }
      }
      else
        addChild(res, process(child, variables));
    }
    return res;
  }

private:
  typedef std::map<String, const XmlElement* > macros_t;

  void addChild(XmlElement* xml, XmlElement* child)
  {
    if (xml && child)
      xml->addChildElement(child);
  }

  macros_t m;
};

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    std::cout << "Usage: " << argv[0] << " input.xml output.cpp" << std::endl;
    return 1;
  }

  SystemStats::initialiseStats();

  File output(argv[2]);
  output.deleteFile();
  OutputStream* ostr = File(argv[2]).createOutputStream();

  inputFile = File(argv[1]);
  
  XmlDocument xmldoc(inputFile);
  XmlElement* iroot = xmldoc.getDocumentElement();
  String error = xmldoc.getLastParseError();
  if (error != String::empty)
  {
    std::cerr << "Parse error: " << (const char* )error << std::endl;
    if (iroot)
      delete iroot;
    return 2;
  }

  XmlMacros macros;
  macros.registerMacrosRecursively(iroot);
  XmlElement* root = macros.process(iroot);
  delete iroot;
  if (!root)
    return 2;

  CppCodeGenerator(root, *ostr).generate();

  delete root;
  delete ostr;
  return 0;
}
