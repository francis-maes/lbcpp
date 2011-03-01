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
#include <set>

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

    String namespaceName = xml->getStringAttribute(T("namespace"), T("lbcpp"));
    newLine();
    openScope(T("namespace ") + namespaceName);

    generateCodeForChildren(xml);
    newLine();
    generateLibraryClass();
    //generateFooter();
    newLine();

    if (xml->getBoolAttribute(T("dynamic")))
    {
      generateDynamicLibraryFunctions();
      newLine();
    }

    newLine();
    closeScope(T("; /* namespace ") + namespaceName + T(" */\n"));
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
      if (tag == T("type") || tag == T("templateType"))
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
      else if (tag == T("uicomponent"))
        declarations.push_back(Declaration::makeUIComponent(elt->getStringAttribute(T("name")), xmlTypeToCppType(elt->getStringAttribute(T("type")))));
      else if (tag == T("code"))
        generateCode(elt);
      else if (tag == T("import") || tag == T("include"))
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
    writeLine(T("#include <lbcpp/Core/Variable.h>"));
    writeLine(T("#include <lbcpp/Core/Library.h>"));
    writeLine(T("#include <lbcpp/library.h>"));

    OwnedArray<File> headerFiles;
    File directory = inputFile.getParentDirectory();
    directory.findChildFiles(headerFiles, File::findFiles, false, T("*.h"));
    std::set<String> sortedFiles;
    for (int i = 0; i < headerFiles.size(); ++i)
    {
      String path = directoryName;
      if (path.isNotEmpty())
        path += T("/");
      path += headerFiles[i]->getRelativePathFrom(directory).replaceCharacter('\\', '/');
      sortedFiles.insert(path);
    }
    for (std::set<String>::const_iterator it = sortedFiles.begin(); it != sortedFiles.end(); ++it)
      writeLine(T("#include ") + it->quoted());

    forEachXmlChildElementWithTagName(*xml, elt, T("include"))
      generateInclude(elt);
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

    String fullName = implementation + T("(T(") + typeName.quoted() + T(")");
    if (baseTypeName.isNotEmpty())
    {
      fullName += T(", ");
      if (xml->getTagName() == T("type"))
        fullName += T("lbcpp::getType(T(") + baseTypeName.quoted() + T("))");
      else
        fullName += T("T(") + baseTypeName.quoted() + T(")");
    }
    fullName += T(")");

    Declaration declaration = Declaration::makeType(typeName, T("Type"));
    declaration.implementationClassName = fullName;
    declarations.push_back(declaration);
    // String singletonVariableName = replaceFirstLettersByLowerCase(typeName) + T("Type");
    //  std::make_pair(currentScope + singletonVariableName, fullName));

    // Type declarator
    if (xml->getTagName() == T("type"))
      writeLine(T("TypePtr ") + declaration.cacheVariableName + T(";"));
  }

  /*
  ** Enumeration
  */
  void generateEnumValueInInitialize(XmlElement* xml)
  {
    String name = xml->getStringAttribute(T("name"), T("???"));
    String oneLetterCode = xml->getStringAttribute(T("oneLetterCode"), String::empty);
    String threeLettersCode = xml->getStringAttribute(T("threeLettersCode"), String::empty);
    writeLine(T("addElement(context, T(") + name.quoted() + T("), T(") + oneLetterCode.quoted() + T("), T(") + threeLettersCode.quoted() + T("));"));
  }

  void generateEnumerationDeclaration(XmlElement* xml)
  {
    String enumName = xml->getStringAttribute(T("name"), T("???"));

    Declaration declaration = Declaration::makeType(enumName, T("Enumeration"));
    declarations.push_back(declaration);
    
    openClass(declaration.implementationClassName, T("DefaultEnumeration"));

    // constructor
    openScope(declaration.implementationClassName + T("() : DefaultEnumeration(T(") + enumName.quoted() + T("))"));
    closeScope();

    newLine();

    openScope(T("virtual bool initialize(ExecutionContext& context)"));
      forEachXmlChildElementWithTagName(*xml, elt, T("value"))
        generateEnumValueInInitialize(elt);
      writeLine(T("return DefaultEnumeration::initialize(context);"));
    closeScope();
    newLine();

    
    // custom code
    forEachXmlChildElementWithTagName(*xml, elt, T("code"))
      {generateCode(elt); newLine();}

    closeClass();

    // enum declarator
    writeLine(T("EnumerationPtr ") + declaration.cacheVariableName + T(";"));
    //writeShortFunction(T("EnumerationPtr ") + declaratorName + T("()"),
    //  T("static TypeCache cache(T(") + enumName.quoted() + T(")); return (EnumerationPtr)cache();"));
  }

  /*
  ** Class
  */
  void generateClassDeclaration(XmlElement* xml, bool isTemplate)
  {
    String className = xml->getStringAttribute(T("name"), T("???"));
    bool isAbstract = xml->getBoolAttribute(T("abstract"), false);
    String classShortName = xml->getStringAttribute(T("shortName"), String::empty);
    String classBaseClass = xml->getStringAttribute(T("metaclass"), T("DefaultClass"));
    String metaClass = getMetaClass(classBaseClass);
    String baseClassName = xmlTypeToCppType(xml->getStringAttribute(T("base"), getDefaultBaseType(metaClass)));

    Declaration declaration = isTemplate ? Declaration::makeTemplateType(className, metaClass) : Declaration::makeType(className, metaClass);
    if (!isTemplate)
      declarations.push_back(declaration);

    openClass(declaration.implementationClassName, classBaseClass);

    // constructor
    std::vector<XmlElement* > variables;
    if (isTemplate)
      openScope(declaration.implementationClassName + T("(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)")
        + T(" : ") + classBaseClass + T("(templateType, templateArguments, baseClass)"));
    else
    {
      String arguments = T("T(") + className.quoted() + T("), T(") + baseClassName.quoted() + T(")");
      openScope(declaration.implementationClassName + T("() : ") + classBaseClass + T("(") + arguments + T(")"));
    }
    if (classShortName.isNotEmpty())
      writeLine(T("shortName = T(") + classShortName.quoted() + T(");"));
    if (isAbstract)
      writeLine(T("abstractClass = true;"));
    closeScope();
    newLine();

    openScope(T("virtual bool initialize(ExecutionContext& context)"));
      forEachXmlChildElementWithTagName(*xml, elt, T("variable"))
      {
        generateVariableDeclarationInConstructor(className, elt);
        variables.push_back(elt);
      }
      writeLine(T("return ") + classBaseClass + T("::initialize(context);"));
    closeScope();
    newLine();

    // create() function
    if (!isAbstract && classBaseClass == T("DefaultClass"))
    {
      openScope(T("virtual Variable create(ExecutionContext& context) const"));
        writeLine(className + T("* res = new ") + className + T("();"));
        writeLine(T("res->setThisClass(refCountedPointerFromThis(this));"));
        writeLine(T("return res;"));
      closeScope();
      newLine();
    }

    // getStaticVariableReference() function
    if (variables.size() && !xml->getBoolAttribute(T("manualAccessors"), false) && classBaseClass == T("DefaultClass"))
    {
      // getMemberVariableValue
      openScope(T("virtual Variable getMemberVariableValue(const Object* __thisbase__, size_t __index__) const"));
        writeLine(T("TypePtr expectedType = getMemberVariableType(__index__);"));
        writeLine(T("if (__index__ < baseType->getNumMemberVariables())"));
        writeLine(T("return baseType->getMemberVariableValue(__thisbase__, __index__);"), 1);
        writeLine(T("__index__ -= baseType->getNumMemberVariables();"));
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
              code += T("(int)(__this__->") + name + T(")");
            else
              code += T("__this__->") + name;

            code += T(", expectedType); break;");
            writeLine(code, -1);
          }
          writeLine(T("default: jassert(false); return Variable();"), -1);
        closeScope();
        writeLine(T("jassert(__res__.getType()->inheritsFrom(expectedType));"));
        writeLine(T("return __res__;"));
      closeScope();
      newLine();

      // setMemberVariableValue
      openScope(T("virtual void setMemberVariableValue(Object* __thisbase__, size_t __index__, const Variable& __subValue__) const"));
        writeLine(T("if (__index__ < baseType->getNumMemberVariables())"));
        writeLine(T("{baseType->setMemberVariableValue(__thisbase__, __index__, __subValue__); return;}"), 1);
        writeLine(T("__index__ -= baseType->getNumMemberVariables();"));
        writeLine(className + T("* __this__ = static_cast<") + className + T("* >(__thisbase__);"));
        newLine();
        openScope(T("switch (__index__)"));
          for (size_t i = 0; i < variables.size(); ++i)
          {
            String name = variables[i]->getStringAttribute(T("var"), String::empty);
            if (name.isEmpty())
              name = variables[i]->getStringAttribute(T("name"), T("???"));

            String code = T("case ") + String((int)i) + T(": lbcpp::variableToNative(defaultExecutionContext(), ");

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

    // class declarator
    if (!isTemplate)
    {
      writeLine(metaClass + T("Ptr ") + declaration.cacheVariableName + T(";"));

      // class constructors
      forEachXmlChildElementWithTagName(*xml, elt, T("constructor"))
        generateClassConstructorMethod(elt, className, baseClassName);
    }
  }

  void generateVariableDeclarationInConstructor(const String& className, XmlElement* xml)
  {
    String type = xmlTypeToCppType(xml->getStringAttribute(T("type"), T("???")));
    String name = xml->getStringAttribute(T("name"), T("???"));
    String shortName = xml->getStringAttribute(T("shortName"), String::empty);
    String description = xml->getStringAttribute(T("description"), String::empty);
    String typeArgument = (type == className ? T("this") : T("T(") + type.quoted() + T(")"));
    
    String arguments = typeArgument + T(", T(") + name.quoted() + T(")");
    arguments += T(", ");
    arguments += shortName.isEmpty() ? T("String::empty") : T("T(") + shortName.quoted() + T(")");
    arguments += T(", ");
    arguments += description.isEmpty() ? T("String::empty") : T("T(") + description.quoted() + T(")");
    
    writeLine(T("addMemberVariable(context, ") + arguments + T(");"));
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

  static String getMetaClass(const String& classBaseClass)
  {
    if (classBaseClass == T("Enumeration"))
      return T("Enumeration");
    else
      return T("Class");
  }

  static String getDefaultBaseType(const String& metaClass) 
    {if (metaClass == T("Enumeration")) return T("EnumValue"); else return T("Object");}


  /*
  ** Template
  */

  void generateTemplateClassDeclaration(XmlElement* xml)
  {
    String className = xml->getStringAttribute(T("name"), T("???"));
    String classBaseClass = xml->getStringAttribute(T("metaclass"), T("DefaultClass"));
    String metaClass = getMetaClass(classBaseClass);
    String baseClassName = xmlTypeToCppType(xml->getStringAttribute(T("base"), getDefaultBaseType(metaClass)));

    Declaration declaration = Declaration::makeTemplateType(className, T("Template") + metaClass);
    declarations.push_back(declaration);

    openClass(declaration.implementationClassName, T("DefaultTemplateType"));

    // constructor
    openScope(declaration.implementationClassName + T("() : DefaultTemplateType(T(") + className.quoted() + T("), T(") + baseClassName.quoted() + T("))"));
    closeScope();
    newLine();

    // initialize()
    openScope(T("virtual bool initialize(ExecutionContext& context)"));
      std::vector<XmlElement* > parameters;
      forEachXmlChildElementWithTagName(*xml, elt, T("parameter"))
      {
        generateParameterDeclarationInConstructor(className, elt);
        parameters.push_back(elt);
      }
      writeLine(T("return DefaultTemplateType::initialize(context);"));
    closeScope();
    newLine();

    // instantiate
    openScope(T("virtual TypePtr instantiate(ExecutionContext& context, const std::vector<TypePtr>& arguments, TypePtr baseType) const"));
      writeLine(T("return new ") + className + metaClass + T("(refCountedPointerFromThis(this), arguments, baseType);"));
    closeScope();
    newLine();

    closeClass();

    // class declarator
    String classNameWithFirstLowerCase = replaceFirstLettersByLowerCase(className) + metaClass;
    if (parameters.size() == 0)
      std::cerr << "Error: No parameters in template. Type = " << (const char *)className << std::endl;
    else if (parameters.size() == 1)
      writeShortFunction(metaClass + T("Ptr ") + classNameWithFirstLowerCase + T("(TypePtr type)"),
      T("return lbcpp::getType(T(") + className.quoted() + T("), std::vector<TypePtr>(1, type));"));
          //T("static UnaryTemplateTypeCache cache(T(") + className.quoted() + T(")); return cache(type);"));
    else if (parameters.size() == 2)
      writeShortFunction(metaClass + T("Ptr ") + classNameWithFirstLowerCase + T("(TypePtr type1, TypePtr type2)"),
        T("std::vector<TypePtr> types(2); types[0] = type1; types[1] = type2; return lbcpp::getType(T(") + className.quoted() + T("), types);"));
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
    writeLine(T("addParameter(context, T(") + name.quoted() + T("), T(") + type.quoted() + T("));"));
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
  void generateLibraryClass()
  {
    String variableName = replaceFirstLettersByLowerCase(fileName) + T("Library");

    forEachXmlChildElementWithTagName(*xml, elt, T("import"))
    {
      String ifdef = elt->getStringAttribute(T("ifdef"));
      if (ifdef.isNotEmpty())
        writeLine(T("#ifdef ") + ifdef);
      String name = elt->getStringAttribute(T("name"), T("???"));
      name = replaceFirstLettersByLowerCase(name) + T("Library");
      writeLine(T("extern lbcpp::LibraryPtr ") + name + T("();"));
      writeLine(T("extern void ") + name + T("CacheTypes(ExecutionContext& context);"));
      writeLine(T("extern void ") + name + T("UnCacheTypes();"));
      if (ifdef.isNotEmpty())
      {
        writeLine(T("#else // ") + ifdef);
        writeLine(T("inline lbcpp::LibraryPtr ") + name + T("() {return lbcpp::LibraryPtr();}"));
        writeLine(T("inline void ") + name + T("CacheTypes(ExecutionContext& context) {}"));
        writeLine(T("inline void ") + name + T("UnCacheTypes() {}"));
        writeLine(T("#endif // ") + ifdef);
      }
    }

    // cacheTypes function
    openScope(T("void ") + variableName + T("CacheTypes(ExecutionContext& context)"));
    forEachXmlChildElementWithTagName(*xml, elt, T("import"))
    {
      String name = elt->getStringAttribute(T("name"), T("???"));
      writeLine(replaceFirstLettersByLowerCase(name) + T("LibraryCacheTypes(context);"));
    }
    for (size_t i = 0; i < declarations.size(); ++i)
    {
      const Declaration& declaration = declarations[i];
      if (declaration.cacheVariableName.isNotEmpty())
        writeLine(declaration.cacheVariableName + T(" = typeManager().getType(context, T(") + declaration.name.quoted() + T("));"));
    }
    closeScope();
    
    // unCacheTypes function
    openScope(T("void ") + variableName + T("UnCacheTypes()"));
    forEachXmlChildElementWithTagName(*xml, elt, T("import"))
    {
      String name = elt->getStringAttribute(T("name"), T("???"));
      writeLine(replaceFirstLettersByLowerCase(name) + T("LibraryUnCacheTypes();"));
    }
    for (size_t i = 0; i < declarations.size(); ++i)
    {
      const Declaration& declaration = declarations[i];
      if (declaration.cacheVariableName.isNotEmpty())
        writeLine(declaration.cacheVariableName + T(".clear();"));
    }
    closeScope();

    // Library class
    openClass(fileName + T("Library"), T("Library"));
    
    // constructor
    openScope(fileName + T("Library() : Library(T(") + fileName.quoted() + T("))"));
    closeScope();
    newLine();

    // initialize function
    openScope(T("virtual bool initialize(ExecutionContext& context)"));
    writeLine(T("bool __ok__ = true;"));

    forEachXmlChildElementWithTagName(*xml, elt, T("import"))
      if (elt->getBoolAttribute(T("pre"), false))
      {
        String name = elt->getStringAttribute(T("name"), T("???"));
        writeLine(T("__ok__ &= declareSubLibrary(context, ") + replaceFirstLettersByLowerCase(name) + T("Library());"));
      }
 
    for (size_t i = 0; i < declarations.size(); ++i)
    {
      const Declaration& declaration = declarations[i];
      if (declaration.type == Declaration::uiComponentDeclaration)
      {
        writeLine(T("__ok__ &= declareUIComponent(context, T(") + declaration.name.quoted() +
          T("), MakeUIComponentConstructor< ") + declaration.implementationClassName + T(">::ctor);"));
      }
      else
      {
        String code = T("__ok__ &= declare");
        if (declaration.type == Declaration::templateTypeDeclaration)
          code += T("TemplateType");
        else if (declaration.type == Declaration::typeDeclaration)
          code += T("Type");
        code += T("(context, new ") + declaration.implementationClassName + T(");");
        writeLine(code);
      }
    }

    forEachXmlChildElementWithTagName(*xml, elt, T("import"))
      if (!elt->getBoolAttribute(T("pre"), false))
      {
        String name = elt->getStringAttribute(T("name"), T("???"));
        writeLine(T("__ok__ &= declareSubLibrary(context, ") + replaceFirstLettersByLowerCase(name) + T("Library());"));
      }

    writeLine(T("return __ok__;"));
    closeScope();
    
    newLine();
    writeShortFunction(T("virtual void cacheTypes(ExecutionContext& context)"), variableName + T("CacheTypes(context);"));
    writeShortFunction(T("virtual void uncacheTypes()"), variableName + T("UnCacheTypes();"));

    closeClass();
    
    writeShortFunction(T("lbcpp::LibraryPtr ") + variableName + T("()"), T("return new ") + fileName + T("Library();"));
  }

  void generateDynamicLibraryFunctions()
  {
    openScope(T("extern \"C\""));

    newLine();
    writeLine(T("# ifdef WIN32"));
    writeLine(T("#  define LBCPP_EXPORT  __declspec( dllexport )"));
    writeLine(T("# else"));
    writeLine(T("#  define LBCPP_EXPORT"));
    writeLine(T("# endif"));
    newLine();

    openScope(T("LBCPP_EXPORT Library* lbcppInitializeLibrary(lbcpp::ApplicationContext& applicationContext)"));
    writeLine(T("lbcpp::initializeDynamicLibrary(applicationContext);"));
    writeLine(T("LibraryPtr res = ") + replaceFirstLettersByLowerCase(fileName) + T("Library();"));
    writeLine(T("res->incrementReferenceCounter();"));
    writeLine(T("return res.get();"));
    closeScope();

    openScope(T("LBCPP_EXPORT void lbcppDeinitializeLibrary()"));
    writeLine(replaceFirstLettersByLowerCase(fileName) + T("LibraryUnCacheTypes();"));
    writeLine(T("lbcpp::deinitializeDynamicLibrary();"));
    closeScope();

    closeScope(); // extern "C"
  }

private:
  XmlElement* xml;
  OutputStream& ostr;
  String fileName;
  String directoryName;
  int indentation;

  struct Declaration
  {
    enum Type
    {
      typeDeclaration,
      templateTypeDeclaration,
      uiComponentDeclaration,
    } type;

    static Declaration makeType(const String& typeName, const String& kind)
    {
      Declaration res;
      res.type = typeDeclaration;
      res.name = typeName;
      res.implementationClassName = typeName + kind;
      res.cacheVariableName = replaceFirstLettersByLowerCase(typeName) + kind;
      return res;
    }

    static Declaration makeTemplateType(const String& typeName, const String& kind)
    {
      Declaration res;
      res.type = templateTypeDeclaration;
      res.name = typeName;
      res.implementationClassName = typeName + kind;
      return res;
    }

    static Declaration makeUIComponent(const String& className, const String& typeName)
    {
      Declaration res;
      res.type = uiComponentDeclaration;
      res.implementationClassName = className;
      res.name = typeName;
      return res;
    }

    String name;
    String implementationClassName;
    String cacheVariableName;
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
    openScope(T("class ") + className + T(" : public ") + baseClass);
    writeLine(T("public:"), -1);
  }

  void closeClass()
    {newLine(); writeLine(T("lbcpp_UseDebuggingNewOperator")); closeScope(T(";")); newLine();}

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
