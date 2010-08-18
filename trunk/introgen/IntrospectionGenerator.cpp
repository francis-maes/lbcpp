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

class CppCodeGenerator
{
public:
  CppCodeGenerator(XmlElement* xml, OutputStream& ostr) : xml(xml), ostr(ostr)
  {
    fileName = xml->getStringAttribute(T("name"), T("???"));
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
    for (XmlElement* elt = xml->getFirstChildElement(); elt; elt = elt->getNextElement())
      if (elt->getTagName() == T("variable"))
      {
        generateVariableDeclarationInConstructor(className, elt);
        variables.push_back(elt);
      }
    closeScope();
    newLine();

    // create() function
    if (!isAbstract)
    {
      writeLine(T("virtual VariableValue create() const"));
      writeLine(T("{return new ") + className + T("();}"), 1);
      newLine();
    }

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

    if (!xml->getBoolAttribute(T("private"), false))
    {
      String declaratorName = className;
      if (declaratorName[0] >= 'A' && declaratorName[0] <= 'Z')
        declaratorName[0] += 'a' - 'A';
      declaratorName += T("Class");
      writeLine(T("ClassPtr ") + declaratorName + T("()"));
      writeLine(T("{static TypeCache cache(T(") + className.quoted() + T(")); return cache();}"), 1);
      newLine();
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

  File input(argv[1]);
  
  XmlDocument xmldoc(input);
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
