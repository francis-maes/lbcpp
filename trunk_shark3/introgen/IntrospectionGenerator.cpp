/*-----------------------------------------.---------------------------------.
| Filename: IntrospectionGenerator.cpp     | Introspection Generator         |
| Author  : Francis Maes                   |                                 |
| Started : 18/08/2010 17:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "../extern/juce/juce_amalgamated.h"
#include <map>
#include <vector>
#include <iostream>
#include <set>

static File inputFile;

class CppCodeGenerator
{
public:
  CppCodeGenerator(XmlElement* xml, OutputStream& ostr) : xml(xml), ostr(ostr)
  {
    fileName = xml->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
    directoryName = xml->getStringAttribute(JUCE_T("directory"), String::empty);
  }

  void generate()
  {
    indentation = 0;
    generateHeader();
    newLine();

    String namespaceName = xml->getStringAttribute(JUCE_T("namespace"), JUCE_T("lbcpp"));
    newLine();
    openScope(JUCE_T("namespace ") + namespaceName);

    generateCodeForChildren(xml);
    newLine();
    generateLibraryClass();
    //generateFooter();
    newLine();

    if (xml->getBoolAttribute(JUCE_T("dynamic")))
    {
      generateDynamicLibraryFunctions();
      newLine();
    }

    newLine();
    closeScope(JUCE_T("; /* namespace ") + namespaceName + JUCE_T(" */\n"));
  }

protected:
  static String xmlTypeToCppType(const String& typeName)
    {return typeName.replaceCharacters(JUCE_T("[]"), JUCE_T("<>"));}

  static String typeToRefCountedPointerType(const String& typeName)
  {
    String str = xmlTypeToCppType(typeName);
    int i = str.indexOfChar('<');
    if (i >= 0)
      str = str.substring(0, i);
    return str + JUCE_T("Ptr");
  }

  static String replaceFirstLettersByLowerCase(const String& str)
  {
    if (str.isEmpty())
      return String::empty;
    int numUpper = 0;
    for (numUpper = 0; numUpper < str.length(); ++numUpper)
      if (!CharacterFunctions::isUpperCase(str[numUpper]) &&
          !CharacterFunctions::isDigit(str[numUpper]))
        break;

    if (numUpper == 0)
      return str;

    String res = str;
    if (numUpper == 1)
      res[0] = CharacterFunctions::toLowerCase(res[0]);
    else
      for (int i = 0; i < numUpper - 1; ++i)
        res[i] = CharacterFunctions::toLowerCase(res[i]);

    return res;
  }

  void generateCodeForChildren(XmlElement* xml)
  {
    for (XmlElement* elt = xml->getFirstChildElement(); elt; elt = elt->getNextElement())
    {
      String tag = elt->getTagName();
      if (tag == JUCE_T("class"))
        generateClassDeclaration(elt, NULL);
      else if (tag == JUCE_T("template"))
      {
        for (XmlElement* cl = elt->getFirstChildElement(); cl; cl = cl->getNextElement())
          if (cl->getTagName() == JUCE_T("class"))
          {
            generateClassDeclaration(cl, elt);
            newLine();
          }
        generateTemplateClassDeclaration(elt);
      }
      else if (tag == JUCE_T("enumeration"))
        generateEnumerationDeclaration(elt);
      else if (tag == JUCE_T("uicomponent"))
#ifdef LBCPP_USER_INTERFACE
        declarations.push_back(Declaration::makeUIComponent(currentNamespace, elt->getStringAttribute(JUCE_T("name")), xmlTypeToCppType(elt->getStringAttribute(JUCE_T("type")))));
#else
        {} // ignore
#endif // LBCPP_USER_INTERFACE
      else if (tag == JUCE_T("code"))
        generateCode(elt);
      else if (tag == JUCE_T("namespace"))
      {
        String name = elt->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
        String previousNamespace = currentNamespace;
        if (currentNamespace.isNotEmpty())
          currentNamespace += JUCE_T("::");
        currentNamespace += name;

        writeLine(JUCE_T("namespace ") + name + JUCE_T(" {"));
        generateCodeForChildren(elt);
        writeLine(JUCE_T("}; /* namespace ") + name + JUCE_T(" */"));

        currentNamespace = previousNamespace;
      }
      else if (tag == JUCE_T("import") || tag == JUCE_T("include"))
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
    writeLine(JUCE_T("#include \"precompiled.h\""));
    writeLine(JUCE_T("#include <oil/Core.h>"));
    writeLine(JUCE_T("#include <oil/Lua/Lua.h>"));
    writeLine(JUCE_T("#include <oil/library.h>"));

    OwnedArray<File> headerFiles;
    File directory = inputFile.getParentDirectory();
    directory.findChildFiles(headerFiles, File::findFiles, false, JUCE_T("*.h"));
    directory.findChildFiles(headerFiles, File::findFiles, false, JUCE_T("*.hpp"));
    std::set<String> sortedFiles;
    for (int i = 0; i < headerFiles.size(); ++i)
    {
      String path = directoryName;
      if (path.isNotEmpty())
        path += JUCE_T("/");
      path += headerFiles[i]->getRelativePathFrom(directory).replaceCharacter('\\', '/');
      sortedFiles.insert(path);
    }
    for (std::set<String>::const_iterator it = sortedFiles.begin(); it != sortedFiles.end(); ++it)
      writeLine(JUCE_T("#include ") + it->quoted());

    forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("include"))
      generateInclude(elt);
  }

  /*
  ** Include
  */
  void generateInclude(XmlElement* xml)
  {
    writeLine(JUCE_T("#include ") + xml->getStringAttribute(JUCE_T("file"), JUCE_T("???")).quoted());
  }

  /*
  ** Enumeration
  */
  void generateEnumValueInInitialize(XmlElement* xml)
  {
    String name = xml->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
    String oneLetterCode = xml->getStringAttribute(JUCE_T("oneLetterCode"), String::empty);
    String threeLettersCode = xml->getStringAttribute(JUCE_T("threeLettersCode"), String::empty);
    writeLine(JUCE_T("addElement(context, JUCE_T(") + name.quoted() + JUCE_T("), JUCE_T(") + oneLetterCode.quoted() + JUCE_T("), JUCE_T(") + threeLettersCode.quoted() + JUCE_T("));"));
  }

  void generateEnumerationDeclaration(XmlElement* xml)
  {
    String enumName = xml->getStringAttribute(JUCE_T("name"), JUCE_T("???"));

    Declaration declaration = Declaration::makeType(currentNamespace, enumName, JUCE_T("Enumeration"));
    declarations.push_back(declaration);
    
    openClass(declaration.implementationClassName, JUCE_T("DefaultEnumeration"));

    // constructor
    openScope(declaration.implementationClassName + JUCE_T("() : DefaultEnumeration(JUCE_T(") + makeFullName(enumName, true) + JUCE_T("))"));
    closeScope();

    newLine();

    openScope(JUCE_T("virtual bool initialize(ExecutionContext& context)"));
      forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("value"))
        generateEnumValueInInitialize(elt);
      writeLine(JUCE_T("return DefaultEnumeration::initialize(context);"));
    closeScope();
    newLine();

    
    // custom code
    forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("code"))
      {generateCode(elt); newLine();}

    closeClass();

    // enum declarator
    writeLine(JUCE_T("EnumerationPtr ") + declaration.cacheVariableName + JUCE_T(";"));
    //writeShortFunction(JUCE_T("EnumerationPtr ") + declaratorName + JUCE_T("()"),
    //  JUCE_T("static TypeCache cache(JUCE_T(") + enumName.quoted() + JUCE_T(")); return (EnumerationPtr)cache();"));
  }

  String makeFullName(const String& identifier, bool quote = false) const
  {
    String res = currentNamespace.isNotEmpty() ? currentNamespace + JUCE_T("::") + identifier : identifier;
    return quote ? res.quoted() : res;
  }

  /*
  ** Class
  */
  void generateClassDeclaration(XmlElement* xml, XmlElement* templateClassXml)
  {
    XmlElement* attributesXml = (templateClassXml ? templateClassXml : xml);

    String className = (templateClassXml && !xml->hasAttribute(JUCE_T("name")) ? templateClassXml : xml)->getStringAttribute(JUCE_T("name"), String::empty);
    bool isAbstract = attributesXml->getBoolAttribute(JUCE_T("abstract"), false);
    String classShortName = attributesXml->getStringAttribute(JUCE_T("shortName"), String::empty);
    String classBaseClass = attributesXml->getStringAttribute(JUCE_T("metaclass"), JUCE_T("DefaultClass"));
    String metaClass = getMetaClass(classBaseClass);
    String baseClassName = xmlTypeToCppType(attributesXml->getStringAttribute(JUCE_T("base"), getDefaultBaseType(metaClass)));

    bool isTemplate = (templateClassXml != NULL);
    Declaration declaration = isTemplate ? Declaration::makeTemplateClass(currentNamespace, className, metaClass) : Declaration::makeType(currentNamespace, className, metaClass);
    if (!isTemplate)
      declarations.push_back(declaration);

    openClass(declaration.implementationClassName, classBaseClass);

    // constructor
    std::vector<XmlElement* > variables;
    std::vector<XmlElement* > functions;
    if (isTemplate)
      openScope(declaration.implementationClassName + JUCE_T("(TemplateClassPtr templateType, const std::vector<ClassPtr>& templateArguments, ClassPtr baseClass)")
        + JUCE_T(" : ") + classBaseClass + JUCE_T("(templateType, templateArguments, baseClass)"));
    else
    {
      String arguments = JUCE_T("JUCE_T(") + makeFullName(className, true) + JUCE_T("), JUCE_T(") + baseClassName.quoted() + JUCE_T(")");
      openScope(declaration.implementationClassName + JUCE_T("() : ") + classBaseClass + JUCE_T("(") + arguments + JUCE_T(")"));
    }
    if (classShortName.isNotEmpty())
      writeLine(JUCE_T("shortName = JUCE_T(") + classShortName.quoted() + JUCE_T(");"));
    if (isAbstract)
      writeLine(JUCE_T("abstractClass = true;"));
    closeScope();
    newLine();

    openScope(JUCE_T("virtual bool initialize(ExecutionContext& context)"));
    
      forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("function"))
      {
        generateFunctionRegistrationCode(className, elt);
        functions.push_back(elt);
      }

      forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("variable"))
      {
        generateVariableRegistrationCode(className, elt);
        variables.push_back(elt);
      }
      writeLine(JUCE_T("return ") + classBaseClass + JUCE_T("::initialize(context);"));
    closeScope();
    newLine();

    // create() function
    if (classBaseClass == JUCE_T("DefaultClass"))
    {
      openScope(JUCE_T("virtual lbcpp::ObjectPtr createObject(ExecutionContext& context) const"));
      if (isAbstract)
      {
        writeLine(JUCE_T("context.errorCallback(\"Cannot instantiate abstract class ") + className + JUCE_T("\");"));
        writeLine(JUCE_T("return lbcpp::ObjectPtr();"));
      }
      else
      {
        writeLine(className + JUCE_T("* res = new ") + className + JUCE_T("();"));
        writeLine(JUCE_T("res->setThisClass(refCountedPointerFromThis(this));"));
        writeLine(JUCE_T("return lbcpp::ObjectPtr(res);"));
      }
      closeScope();
      newLine();
    }

    // getStaticVariableReference() function
    if (variables.size() && !xml->getBoolAttribute(JUCE_T("manualAccessors"), false) && classBaseClass == JUCE_T("DefaultClass"))
    {
      // getMemberVariableValue
      openScope(JUCE_T("virtual lbcpp::ObjectPtr getMemberVariableValue(const Object* __thisbase__, size_t __index__) const"));
        if (baseClassName != JUCE_T("Object"))
        {
          writeLine(JUCE_T("static size_t numBaseMemberVariables = baseType->getNumMemberVariables();"));
          writeLine(JUCE_T("if (__index__ < numBaseMemberVariables)"));
          writeLine(JUCE_T("return baseType->getMemberVariableValue(__thisbase__, __index__);"), 1);
          writeLine(JUCE_T("__index__ -= numBaseMemberVariables;"));
        }
        writeLine(JUCE_T("const ") + className + JUCE_T("* __this__ = static_cast<const ") + className + JUCE_T("* >(__thisbase__);"));
        //writeLine(JUCE_T("const ClassPtr& expectedType = variables[__index__]->getType();"));
        newLine();
        openScope(JUCE_T("switch (__index__)"));
          for (size_t i = 0; i < variables.size(); ++i)
          {
            String name = variables[i]->getStringAttribute(JUCE_T("var"), String::empty);
            if (name.isEmpty())
              name = variables[i]->getStringAttribute(JUCE_T("name"), JUCE_T("???"));

            String code = JUCE_T("case ") + String((int)i) + JUCE_T(": return lbcpp::nativeToObject(");
            bool isEnumeration = variables[i]->getBoolAttribute(JUCE_T("enumeration"), false);
            if (isEnumeration)
              code += JUCE_T("(int)(__this__->") + name + JUCE_T(")");
            else
              code += JUCE_T("__this__->") + name;

            code += JUCE_T(", variables[__index__]->getType());");
            writeLine(code, -1);
          }
          writeLine(JUCE_T("default: jassert(false); return lbcpp::ObjectPtr();"), -1);
        closeScope();
      closeScope();
      newLine();

      // setMemberVariableValue
      openScope(JUCE_T("virtual void setMemberVariableValue(Object* __thisbase__, size_t __index__, const lbcpp::ObjectPtr& __subValue__) const"));
        writeLine(JUCE_T("if (__index__ < baseType->getNumMemberVariables())"));
        writeLine(JUCE_T("{baseType->setMemberVariableValue(__thisbase__, __index__, __subValue__); return;}"), 1);
        writeLine(JUCE_T("__index__ -= baseType->getNumMemberVariables();"));
        writeLine(className + JUCE_T("* __this__ = static_cast<") + className + JUCE_T("* >(__thisbase__);"));
        newLine();
        openScope(JUCE_T("switch (__index__)"));
          for (size_t i = 0; i < variables.size(); ++i)
          {
            String name = variables[i]->getStringAttribute(JUCE_T("var"), String::empty);
            if (name.isEmpty())
              name = variables[i]->getStringAttribute(JUCE_T("name"), JUCE_T("???"));

            String code = JUCE_T("case ") + String((int)i) + JUCE_T(": lbcpp::objectToNative(defaultExecutionContext(), ");

            bool isEnumeration = variables[i]->getBoolAttribute(JUCE_T("enumeration"), false);
            if (isEnumeration)
              code += JUCE_T("(int& )(__this__->") + name + JUCE_T(")");
            else
              code += JUCE_T("__this__->") + name;
            code += JUCE_T(", __subValue__); break;");
            writeLine(code, -1);
          }
          writeLine(JUCE_T("default: jassert(false);"), -1);
        closeScope();
      closeScope();
    }

    // function forwarders
    for (size_t i = 0; i < functions.size(); ++i)
    {
      XmlElement* elt = functions[i];
      String functionName = elt->getStringAttribute(JUCE_T("name"));
      writeShortFunction(JUCE_T("static int ") + functionName + JUCE_T("Forwarder(lua_State* L)"),
                         JUCE_T("LuaState state(L); return ") + className + JUCE_T("::") + functionName + JUCE_T("(state);")); 
    }

    forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("code"))
      {generateCode(elt); newLine();}

    closeClass();

    // class declarator
    if (!isTemplate)
    {
      writeLine(metaClass + JUCE_T("Ptr ") + declaration.cacheVariableName + JUCE_T(";"));

      // class constructors
      forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("constructor"))
        generateClassConstructorMethod(elt, className, baseClassName);
    }
  }

  void generateVariableRegistrationCode(const String& className, XmlElement* xml)
  {
    String type = xmlTypeToCppType(xml->getStringAttribute(JUCE_T("type"), JUCE_T("???")));
    String name = xml->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
    String shortName = xml->getStringAttribute(JUCE_T("shortName"), String::empty);
    String description = xml->getStringAttribute(JUCE_T("description"), String::empty);
    String typeArgument = (type == className ? JUCE_T("this") : JUCE_T("JUCE_T(") + type.quoted() + JUCE_T(")"));
    
    String arguments = typeArgument + JUCE_T(", JUCE_T(") + name.quoted() + JUCE_T(")");
    arguments += JUCE_T(", ");
    arguments += shortName.isEmpty() ? JUCE_T("lbcpp::string::empty") : JUCE_T("JUCE_T(") + shortName.quoted() + JUCE_T(")");
    arguments += JUCE_T(", ");
    arguments += description.isEmpty() ? JUCE_T("lbcpp::string::empty") : JUCE_T("JUCE_T(") + description.quoted() + JUCE_T(")");
    
    if (xml->getBoolAttribute(JUCE_T("generated"), false))
      arguments += JUCE_T(", true");
    
    writeLine(JUCE_T("addMemberVariable(context, ") + arguments + JUCE_T(");"));
  }

  void generateFunctionRegistrationCode(const String& className, XmlElement* xml)
  {
    String lang = xml->getStringAttribute(JUCE_T("lang"), JUCE_T("???"));
    String name = xml->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
    String shortName = xml->getStringAttribute(JUCE_T("shortName"), String::empty);
    String description = xml->getStringAttribute(JUCE_T("description"), String::empty);

    if (lang != JUCE_T("lua"))
      std::cerr << "Unsupported language " << (const char* )lang << " for function " << (const char* )name << std::endl;

    String arguments = name + JUCE_T("Forwarder, JUCE_T(") + name.quoted() + JUCE_T(")");
    arguments += JUCE_T(", ");
    arguments += shortName.isEmpty() ? JUCE_T("lbcpp::string::empty") : JUCE_T("JUCE_T(") + shortName.quoted() + JUCE_T(")");
    arguments += JUCE_T(", ");
    arguments += description.isEmpty() ? JUCE_T("lbcpp::string::empty") : JUCE_T("JUCE_T(") + description.quoted() + JUCE_T(")");
    if (xml->getBoolAttribute(JUCE_T("static"), false))
      arguments += JUCE_T(", true");

    writeLine(JUCE_T("addMemberFunction(context, ") + arguments + JUCE_T(");"));
  }

  void generateClassConstructorMethod(XmlElement* xml, const String& className, const String& baseClassName)
  {
    String arguments = xml->getStringAttribute(JUCE_T("arguments"), String::empty);
    String parameters = xml->getStringAttribute(JUCE_T("parameters"), String::empty);
    String returnType = xml->getStringAttribute(JUCE_T("returnType"), String::empty);
    
    if (returnType.isEmpty())
      returnType = baseClassName;

    StringArray tokens;
    tokens.addTokens(arguments, JUCE_T(","), NULL);
    String argNames;
    for (int i = 0; i < tokens.size(); ++i)
    {
      String argName = tokens[i];
      int n = argName.lastIndexOfChar(' ');
      if (n >= 0)
        argName = argName.substring(n + 1);
      if (argNames.isNotEmpty())
        argNames += JUCE_T(", ");
      argNames += argName;
    }

    // class declarator
    String classNameWithFirstLowerCase = replaceFirstLettersByLowerCase(className);
    String returnTypePtr = typeToRefCountedPointerType(returnType);
    openScope(returnTypePtr + JUCE_T(" ") + classNameWithFirstLowerCase + JUCE_T("(") + arguments + JUCE_T(")"));
      writeLine(returnTypePtr + JUCE_T(" res = new ") + className + JUCE_T("(") + argNames + JUCE_T(");"));
      String code = JUCE_T("res->setThisClass(") + classNameWithFirstLowerCase + JUCE_T("Class");
      if (parameters.isNotEmpty())
        code += JUCE_T("(") + parameters + JUCE_T(")");
      writeLine(code + JUCE_T(");"));
      writeLine(JUCE_T("return res;"));
    closeScope();
    newLine();
  }

  static String getMetaClass(const String& classBaseClass)
  {
    if (classBaseClass == JUCE_T("Enumeration"))
      return JUCE_T("Enumeration");
    else
      return JUCE_T("Class");
  }

  static String getDefaultBaseType(const String& metaClass) 
    {if (metaClass == JUCE_T("Enumeration")) return JUCE_T("EnumValue"); else return JUCE_T("Object");}


  /*
  ** Template
  */
  void generateTemplateClassDeclaration(XmlElement* xml)
  {
    String className = xml->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
    String classBaseClass = xml->getStringAttribute(JUCE_T("metaclass"), JUCE_T("DefaultClass"));
    String metaClass = getMetaClass(classBaseClass);
    String baseClassName = xmlTypeToCppType(xml->getStringAttribute(JUCE_T("base"), getDefaultBaseType(metaClass)));

    Declaration declaration = Declaration::makeTemplateClass(currentNamespace, className, JUCE_T("Template") + metaClass);
    declarations.push_back(declaration);

    openClass(declaration.implementationClassName, JUCE_T("DefaultTemplateClass"));

    // constructor
    openScope(declaration.implementationClassName + JUCE_T("() : DefaultTemplateClass(JUCE_T(") + className.quoted() + JUCE_T("), JUCE_T(") + baseClassName.quoted() + JUCE_T("))"));
    closeScope();
    newLine();

    // initialize()
    openScope(JUCE_T("virtual bool initialize(ExecutionContext& context)"));
      std::vector<XmlElement* > parameters;
      forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("parameter"))
      {
        generateParameterDeclarationInConstructor(className, elt);
        parameters.push_back(elt);
      }
      writeLine(JUCE_T("return DefaultTemplateClass::initialize(context);"));
    closeScope();
    newLine();

    // instantiate
    openScope(JUCE_T("virtual ClassPtr instantiate(ExecutionContext& context, const std::vector<ClassPtr>& arguments, ClassPtr baseType) const"));
      generateTemplateInstantiationFunction(xml);
    closeScope();
    newLine();

    closeClass();

    // class declarator
    String classNameWithFirstLowerCase = replaceFirstLettersByLowerCase(className) + metaClass;
    if (parameters.size() == 0)
      std::cerr << "Error: No parameters in template. Type = " << (const char *)className << std::endl;
    else
    {
      String arguments;
      String initialization;
      for (size_t i = 0; i < parameters.size(); ++i)
      {
        arguments += JUCE_T("ClassPtr type") + String((int)i + 1);
        initialization += JUCE_T("types[") + String((int)i) + JUCE_T("] = type") + String((int)i + 1) + JUCE_T("; ");

        if (i < parameters.size() - 1)
          arguments += JUCE_T(", ");
      }
      writeShortFunction(metaClass + JUCE_T("Ptr ") + classNameWithFirstLowerCase + JUCE_T("(") + arguments + JUCE_T(")"),
        JUCE_T("std::vector<ClassPtr> types(") + String((int)parameters.size()) + JUCE_T("); ") + initialization + JUCE_T("return lbcpp::getType(JUCE_T(") + className.quoted() + JUCE_T("), types);")); 
    }

    // class constructors
    forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("constructor"))
      generateClassConstructorMethod(elt, className, baseClassName);
  }

  void generateParameterDeclarationInConstructor(const String& className, XmlElement* xml)
  {
    String type = xmlTypeToCppType(xml->getStringAttribute(JUCE_T("type"), JUCE_T("Object")));
    String name = xml->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
    writeLine(JUCE_T("addParameter(context, JUCE_T(") + name.quoted() + JUCE_T("), JUCE_T(") + type.quoted() + JUCE_T("));"));
  }

  void generateTemplateInstantiationFunction(XmlElement* xml)
  {
    String templateClassName = xml->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
    String classBaseClass = xml->getStringAttribute(JUCE_T("metaclass"), JUCE_T("DefaultClass"));
    String metaClass = getMetaClass(classBaseClass);

    forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("class"))
    {
      String className = elt->getStringAttribute(JUCE_T("name"));
      if (className.isEmpty())
        className = templateClassName;
      String condition = generateSpecializationCondition(elt);
      if (condition.isNotEmpty())
        writeLine(JUCE_T("if (") + condition + JUCE_T(")"));
      writeLine(JUCE_T("return new ") + className + metaClass + JUCE_T("(refCountedPointerFromThis(this), arguments, baseType);"), condition.isEmpty() ? 0 : 1);
    }
  }

  String generateSpecializationCondition(XmlElement* xml)
  {
    String res;
    forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("specialization"))
    {
      if (res.isNotEmpty())
        res += " && ";
      res += "inheritsFrom(context, arguments, " + elt->getStringAttribute(JUCE_T("name"), JUCE_T("???")).quoted() + JUCE_T(", ") + elt->getStringAttribute(JUCE_T("type"), JUCE_T("???")).quoted() + JUCE_T(")");
    }
    return res;
  }


  /*
  ** Code
  */
  void generateCode(XmlElement* elt)
  {
    StringArray lines;
    lines.addTokens(elt->getAllSubText(), JUCE_T("\n"), NULL);
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
  void generateLibraryClass()
  {
    String variableName = replaceFirstLettersByLowerCase(fileName) + JUCE_T("Library");

    forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("import"))
    {
      String ifdef = elt->getStringAttribute(JUCE_T("ifdef"));
      if (ifdef.isNotEmpty())
        writeLine(JUCE_T("#ifdef ") + ifdef);
      String name = elt->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
      name = replaceFirstLettersByLowerCase(name) + JUCE_T("Library");
      writeLine(JUCE_T("extern lbcpp::LibraryPtr ") + name + JUCE_T("();"));
      writeLine(JUCE_T("extern void ") + name + JUCE_T("CacheTypes(ExecutionContext& context);"));
      writeLine(JUCE_T("extern void ") + name + JUCE_T("UnCacheTypes();"));
      if (ifdef.isNotEmpty())
      {
        writeLine(JUCE_T("#else // ") + ifdef);
        writeLine(JUCE_T("inline lbcpp::LibraryPtr ") + name + JUCE_T("() {return lbcpp::LibraryPtr();}"));
        writeLine(JUCE_T("inline void ") + name + JUCE_T("CacheTypes(ExecutionContext& context) {}"));
        writeLine(JUCE_T("inline void ") + name + JUCE_T("UnCacheTypes() {}"));
        writeLine(JUCE_T("#endif // ") + ifdef);
      }
    }

    // cacheTypes function
    openScope(JUCE_T("void ") + variableName + JUCE_T("CacheTypes(ExecutionContext& context)"));
    forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("import"))
    {
      String name = elt->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
      writeLine(replaceFirstLettersByLowerCase(name) + JUCE_T("LibraryCacheTypes(context);"));
    }
    for (size_t i = 0; i < declarations.size(); ++i)
    {
      const Declaration& declaration = declarations[i];
      if (declaration.cacheVariableName.isNotEmpty())
        writeLine(declaration.getCacheVariableFullName() + JUCE_T(" = typeManager().getType(context, JUCE_T(") + declaration.getFullName().quoted() + JUCE_T("));"));
    }
    closeScope();
    
    // unCacheTypes function
    openScope(JUCE_T("void ") + variableName + JUCE_T("UnCacheTypes()"));
    forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("import"))
    {
      String name = elt->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
      writeLine(replaceFirstLettersByLowerCase(name) + JUCE_T("LibraryUnCacheTypes();"));
    }
    for (size_t i = 0; i < declarations.size(); ++i)
    {
      const Declaration& declaration = declarations[i];
      if (declaration.cacheVariableName.isNotEmpty())
        writeLine(declaration.getCacheVariableFullName() + JUCE_T(".clear();"));
    }
    closeScope();

    // Library class
    openClass(fileName + JUCE_T("Library"), JUCE_T("Library"));
    
    // constructor
    openScope(fileName + JUCE_T("Library() : Library(JUCE_T(") + fileName.quoted() + JUCE_T("))"));
    closeScope();
    newLine();

    // initialize function
    openScope(JUCE_T("virtual bool initialize(ExecutionContext& context)"));
    writeLine(JUCE_T("bool __ok__ = true;"));

    for (size_t i = 0; i < declarations.size(); ++i)
    {
      const Declaration& declaration = declarations[i];
      if (declaration.type == Declaration::uiComponentDeclaration)
      {
        writeLine(JUCE_T("__ok__ &= declareUIComponent(context, JUCE_T(") + declaration.name.quoted() +
          JUCE_T("), MakeUIComponentConstructor< ") + declaration.implementationClassName + JUCE_T(">::ctor);"));
      }
      else
      {
        String code = JUCE_T("__ok__ &= declare");
        if (declaration.type == Declaration::templateTypeDeclaration)
          code += JUCE_T("TemplateClass");
        else if (declaration.type == Declaration::typeDeclaration)
          code += JUCE_T("Type");
        code += JUCE_T("(context, new ") + declaration.getImplementationClassFullName() + JUCE_T(");");
        writeLine(code);
      }
    }

    forEachXmlChildElementWithTagName(*xml, elt, JUCE_T("import"))
    {
      String name = elt->getStringAttribute(JUCE_T("name"), JUCE_T("???"));
      writeLine(JUCE_T("__ok__ &= declareSubLibrary(context, ") + replaceFirstLettersByLowerCase(name) + JUCE_T("Library());"));
    }

    writeLine(JUCE_T("return __ok__;"));
    closeScope();
    
    newLine();
    writeShortFunction(JUCE_T("virtual void cacheTypes(ExecutionContext& context)"), variableName + JUCE_T("CacheTypes(context);"));
    writeShortFunction(JUCE_T("virtual void uncacheTypes()"), variableName + JUCE_T("UnCacheTypes();"));

    closeClass();
    
    writeShortFunction(JUCE_T("lbcpp::LibraryPtr ") + variableName + JUCE_T("()"), JUCE_T("return new ") + fileName + JUCE_T("Library();"));
  }

  void generateDynamicLibraryFunctions()
  {
    openScope(JUCE_T("extern \"C\""));

    newLine();
    writeLine(JUCE_T("# ifdef WIN32"));
    writeLine(JUCE_T("#  define OIL_EXPORT  __declspec( dllexport )"));
    writeLine(JUCE_T("# else"));
    writeLine(JUCE_T("#  define OIL_EXPORT"));
    writeLine(JUCE_T("# endif"));
    newLine();

    openScope(JUCE_T("OIL_EXPORT Library* lbcppInitializeLibrary(lbcpp::ApplicationContext& applicationContext)"));
    writeLine(JUCE_T("lbcpp::initializeDynamicLibrary(applicationContext);"));
    writeLine(JUCE_T("LibraryPtr res = ") + replaceFirstLettersByLowerCase(fileName) + JUCE_T("Library();"));
    writeLine(JUCE_T("res->incrementReferenceCounter();"));
    writeLine(JUCE_T("return res.get();"));
    closeScope();

    openScope(JUCE_T("OIL_EXPORT void lbcppDeinitializeLibrary()"));
    writeLine(replaceFirstLettersByLowerCase(fileName) + JUCE_T("LibraryUnCacheTypes();"));
    writeLine(JUCE_T("lbcpp::deinitializeDynamicLibrary();"));
    closeScope();

    closeScope(); // extern "C"
  }

private:
  XmlElement* xml;
  OutputStream& ostr;
  String fileName;
  String directoryName;
  int indentation;
  String currentNamespace;

  struct Declaration
  {
    enum Type
    {
      typeDeclaration,
      templateTypeDeclaration,
      uiComponentDeclaration,
    } type;

    static Declaration makeType(const String& namespaceName, const String& typeName, const String& kind)
    {
      Declaration res;
      res.namespaceName = namespaceName;
      res.type = typeDeclaration;
      res.name = typeName;
      res.implementationClassName = typeName + kind;
      res.cacheVariableName = replaceFirstLettersByLowerCase(typeName) + kind;
      return res;
    }

    static Declaration makeTemplateClass(const String& namespaceName, const String& typeName, const String& kind)
    {
      Declaration res;
      res.namespaceName = namespaceName;
      res.type = templateTypeDeclaration;
      res.name = typeName;
      res.implementationClassName = typeName + kind;
      return res;
    }

    static Declaration makeUIComponent(const String& namespaceName, const String& className, const String& typeName)
    {
      Declaration res;
      res.namespaceName = namespaceName;
      res.type = uiComponentDeclaration;
      res.implementationClassName = className;
      res.name = typeName;
      return res;
    }

    String name;
    String namespaceName;
    String implementationClassName;
    String cacheVariableName;

    String getFullName() const // namespace and name
      {return namespaceName.isEmpty() ? name : namespaceName + JUCE_T("::") + name;}

    String getCacheVariableFullName() const
      {return namespaceName.isEmpty() ? cacheVariableName : namespaceName + JUCE_T("::") + cacheVariableName;}

    String getImplementationClassFullName() const
      {return namespaceName.isEmpty() ? implementationClassName : namespaceName + JUCE_T("::") + implementationClassName;}
  };

  std::vector<Declaration> declarations;

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
    openScope(JUCE_T("class ") + className + JUCE_T(" : public ") + baseClass);
    writeLine(JUCE_T("public:"), -1);
  }

  void closeClass()
    {newLine(); writeLine(JUCE_T("lbcpp_UseDebuggingNewOperator")); closeScope(JUCE_T(";")); newLine();}

  void writeShortFunction(const String& declaration, const String& oneLineBody)
  {
    writeLine(declaration);
    writeLine(JUCE_T("{") + oneLineBody + JUCE_T("}"), 1);
    newLine();
  }
};

class XmlMacros
{
public:
  void registerMacrosRecursively(const XmlElement* xml)
  {
    if (xml->getTagName() == JUCE_T("defmacro"))
      m[xml->getStringAttribute(JUCE_T("name"))] = xml;
    else
      for (int i = 0; i < xml->getNumChildElements(); ++i)
        registerMacrosRecursively(xml->getChildElement(i));
  }

  typedef std::map<String, String> variables_t;

  String processText(const String& str, const variables_t& variables)
  {
    String res = str;
    for (std::map<String, String>::const_iterator it = variables.begin(); it != variables.end(); ++it)
      res = res.replace(JUCE_T("%{") +  it->first + JUCE_T("}"), it->second);
    return res;
  }

  // returns a new XmlElement
  XmlElement* process(const XmlElement* xml, const variables_t& variables = variables_t())
  {
    if (xml->getTagName() == JUCE_T("defmacro"))
      return NULL;

    jassert(xml->getTagName() != JUCE_T("macro"));

    if (xml->isTextElement())
      return XmlElement::createTextElement(processText(xml->getText(), variables));

    XmlElement* res = new XmlElement(xml->getTagName());
    for (int i = 0; i < xml->getNumAttributes(); ++i)
      res->setAttribute(xml->getAttributeName(i), processText(xml->getAttributeValue(i), variables));

    for (int i = 0; i < xml->getNumChildElements(); ++i)
    {
      XmlElement* child = xml->getChildElement(i);
      if (child->getTagName() == JUCE_T("macro"))
      {
        String name = processText(child->getStringAttribute(JUCE_T("name")), variables);
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
            if (child->getAttributeName(j) != JUCE_T("name"))
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
