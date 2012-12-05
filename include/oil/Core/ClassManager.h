/*-----------------------------------------.---------------------------------.
| Filename: ClassManager.h                 | Global Class Manager            |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_CORE_TYPE_MANAGER_H_
# define LBCPP_CORE_TYPE_MANAGER_H_

# include "predeclarations.h"

namespace lbcpp
{

struct TemplateClassCache;
class ClassManager
{
public:
  ClassManager();
  ~ClassManager();

  bool declare(ExecutionContext& context, ClassPtr type);
  bool declare(ExecutionContext& context, TemplateClassPtr templateType);

  void finishDeclarations(ExecutionContext& context);

  ClassPtr getType(ExecutionContext& context, const string& typeName, const std::vector<ClassPtr>& arguments) const;
  ClassPtr getType(ExecutionContext& context, const string& name) const;
  ClassPtr getTypeByShortName(ExecutionContext& context, const string& shortName) const;

  ClassPtr findType(const string& name) const;
  bool doTypeExists(const string& type) const;

  void shutdown();

private:
  typedef std::map<string, ClassPtr> TypeMap;
  typedef std::map<string, TemplateClassCache> TemplateClassMap;

  CriticalSection typesLock;
  TypeMap types;
  TypeMap typesByShortName;
  TemplateClassMap templateTypes;
 
  bool hasTemplateClass(const string& templateTypeName) const;
  TemplateClassCache* getTemplateClass(ExecutionContext& context, const string& templateTypeName) const;

  static string removeAllSpaces(const string& str);
};

extern ClassManager& typeManager();

extern ClassPtr getType(const string& typeName);
extern ClassPtr getType(const string& name, const std::vector<ClassPtr>& arguments);
extern bool doTypeExists(const string& typeName);
extern bool declareType(ClassPtr type);

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_TYPE_MANAGER_H_
