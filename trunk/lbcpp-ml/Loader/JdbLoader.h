/*-----------------------------------------.---------------------------------.
| Filename: JdbLoader.h                    | JDB Loader                      |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 16:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_LOADER_JDB_H_
# define LBCPP_ML_LOADER_JDB_H_

# include <lbcpp/Core/Loader.h>
# include <lbcpp/Data/Table.h>
# include <lbcpp-ml/Expression.h>

namespace lbcpp
{
 
# ifdef JUCE_WIN32
#  pragma warning(disable:4996) // microsoft visual does not like fopen()/fclose()
# endif // JUCE_WIN32

class JdbLoader : public TextLoader
{
public:
  virtual string getFileExtensions() const
    {return "jdb";}

  virtual ClassPtr getTargetClass() const
    {return tableClass;}

  virtual void parseBegin(ExecutionContext& context)
  {
    hasReadDatasetName = false;
    hasReadAttributes = false;
    table = TablePtr();
  }

  virtual bool parseLine(ExecutionContext& context, const string& l)
  {
    char* line = const_cast<char* >((const char* )l);
    while (*line == ' ' || *line == '\t')
      ++line;
    if (*line == 0 || *line == ';')
      return true; // skip empty lines and comment lines
    if (!hasReadDatasetName)
    {
      hasReadDatasetName = true;
      return true;
    }
    if (!hasReadAttributes)
    {
      table = parseAttributes(context, line);
      hasReadAttributes = true;
      return table != TablePtr();
    }
    return parseExample(context, line, table);
  }

  virtual ObjectPtr parseEnd(ExecutionContext& context)
    {return table;}

protected:
  TablePtr table;
  bool hasReadDatasetName;
  bool hasReadAttributes;

  TablePtr parseAttributes(ExecutionContext& context, char* line)
  {
    TablePtr res = new Table();

    bool isFirst = true;
    while (true)
    {
      char* name = strtok(isFirst ? line : NULL, " \t\n\r");
      char* kind = strtok(NULL, " \t\n\r");
      if (!name || !kind)
        break;
      isFirst = false;
      ClassPtr attributeClass;
      if (!strcmp(kind, "numerical") || !strcmp(kind, "NUMERICAL"))
        attributeClass = doubleClass;
      else if (!strcmp(kind, "symbolic") || !strcmp(kind, "SYMBOLIC"))
        attributeClass = new DefaultEnumeration();
      else if (!strcmp(kind, "name") || !strcmp(kind, "NAME"))
        attributeClass = stringClass;
      else
      {
        context.errorCallback(T("Could not recognize attribute type ") + string(kind).quoted());
        return TablePtr();
      }
      res->addColumn(new VariableExpression(attributeClass, name, res->getNumColumns()), attributeClass);
    }
    return res;
  }

  bool parseExample(ExecutionContext& context, char* line, TablePtr table)
  {
    std::vector<ObjectPtr> row;
    for (size_t i = 0; true; ++i)
    {
      char* token = strtok((i == 0 ? line : NULL), " \t\n");
      if (!token)
        break;

      ObjectPtr value;

      ClassPtr attributeClass = table->getType(i);
      DefaultEnumerationPtr enumeration = attributeClass.dynamicCast<DefaultEnumeration>();
      if (enumeration)
        value = new EnumValue(enumeration, enumeration->findOrAddElement(context, token));
      else if (attributeClass == doubleClass)
        value = new Double(strtod(token, NULL));
      else if (attributeClass == stringClass)
        value = new String(token);
      else
        jassertfalse;

      row.push_back(value);
    }
    table->addRow(row);
    return true;
  }
};

}; /* namespace lbcpp */

#endif // LBCPP_ML_LOADER_JDB_H_
