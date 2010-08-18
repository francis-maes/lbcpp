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

  void generateCodeForChildren(XmlElement* xml)
  {
    for (XmlElement* elt = xml->getFirstChildElement(); elt; elt = elt->getNextElement())
    {
      String tag = elt->getTagName();
      if (tag == T("include"))
        generateInclude(elt);
      else if (tag == T("class"))
        generateClassDeclaration(elt);
      else if (tag == T("namespace"))
        generateNamespaceDeclaration(elt);
      else if (tag == T("code"))
        generateCode(elt);
      else
        std::cerr << "Warning: unrecognized tag: " << (const char* )tag << std::endl;
    }
  }

  void generateInclude(XmlElement* xml)
  {
    writeLine(T("#include ") + xml->getStringAttribute(T("file"), T("???")).quoted());
  }

  void generateVariableDeclarationInConstructor(const String& className, XmlElement* xml)
  {
    String type = xmlTypeToCppType(xml->getStringAttribute(T("type"), T("???")));
    String name = xml->getStringAttribute(T("name"), T("???"));
    
    String typeArgument = (type == className ? T("ClassPtr(this)") : T("T(") + type.quoted() + T(")"));
    writeLine(T("addVariable(") + typeArgument + T(", T(") + name.quoted() + T("));"));
  }

  void generateClassDeclaration(XmlElement* xml)
  {
    String className = xml->getStringAttribute(T("name"), T("???"));
    String baseClassName = xml->getStringAttribute(T("base"), T("Object"));
    bool isAbstract = xml->getBoolAttribute(T("abstract"), false);

    currentScopes.push_back(className);
    classes.push_back(getCurrentScopeFullName());

    openScope(T("class ") + className + T("Class : public DynamicClass"));
    writeLine(T("public:"), -1);

    // constructor
    std::vector<XmlElement* > variables;
    openScope(className + T("Class() : DynamicClass(T(") + className.quoted() + T("), T(") + baseClassName.quoted() + T("))"));
    forEachXmlChildElementWithTagName(*xml, elt, T("variable"))
    {
      generateVariableDeclarationInConstructor(className, elt);
      variables.push_back(elt);
    }
    closeScope();
    newLine();

    // create() function
    if (!isAbstract)
      writeShortFunction(T("virtual VariableValue create() const"),
        T("return new ") + className + T("();"));

    // getStaticVariableReference() function
    if (variables.size())
    {
      openScope(T("virtual VariableReference getStaticVariableReference(const VariableValue& __value__, size_t __index__) const"));
        writeLine(T("if (__index__ < baseType->getNumStaticVariables())"));
        writeLine(T("return baseType->getStaticVariableReference(__value__, __index__);"), 1);
        writeLine(T("__index__ -= baseType->getNumStaticVariables();"));
        writeLine(className + T("* __this__ = static_cast<") + className + T("* >(__value__.getObjectPointer());"));
        newLine();
        openScope(T("switch (__index__)"));
          for (size_t i = 0; i < variables.size(); ++i)
          {
            String name = variables[i]->getStringAttribute(T("name"), T("???"));
            String cast = variables[i]->getStringAttribute(T("cast"), String::empty);
            String code = T("case ") + String((int)i) + T(": return ");
            if (cast.isNotEmpty())
              code += T("(") + cast + T("& )(");
            code += T("__this__->") + name;
            if (cast.isNotEmpty())
              code += T(")");
            code += T(";");
            writeLine(code, -1);
          }
          writeLine(T("default: jassert(false); return VariableReference();"), -1);
        closeScope();
      closeScope();
    }

    closeScope(T(";"));
    newLine();

    currentScopes.pop_back();

    String classNameWithFirstLowerCase = className;
    if (classNameWithFirstLowerCase[0] >= 'A' && classNameWithFirstLowerCase[0] <= 'Z')
      classNameWithFirstLowerCase[0] += 'a' - 'A';

    // class declarator
    writeShortFunction(T("ClassPtr ") + classNameWithFirstLowerCase + T("Class()"),
      T("static TypeCache cache(T(") + className.quoted() + T(")); return cache();"));

    // class constructors
    forEachXmlChildElementWithTagName(*xml, elt, T("constructor"))
    {
      String arguments = elt->getStringAttribute(T("arguments"), String::empty);
      String returnType = elt->getStringAttribute(T("returnType"), String::empty);
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

      writeShortFunction(returnType + T("Ptr lbcpp::") + classNameWithFirstLowerCase + T("(") + arguments + T(")"),
                         T("return new ") + className + T("(") + argNames + T(");"));
    }
  }

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

  void generateFooter()
  {
    openScope(T("void declare") + fileName + T("Classes()"));
    for (size_t i = 0; i < classes.size(); ++i)
      writeLine(T("lbcpp::Class::declare(new ") + classes[i] + T("Class());"));
    closeScope();
  }

private:
  XmlElement* xml;
  OutputStream& ostr;
  String fileName;
  String directoryName;
  int indentation;
  
  std::vector<String> currentScopes;
  std::vector<String> classes;
  
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
