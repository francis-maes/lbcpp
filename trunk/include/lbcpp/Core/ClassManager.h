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

  ClassPtr getType(ExecutionContext& context, const String& typeName, const std::vector<ClassPtr>& arguments) const;
  ClassPtr getType(ExecutionContext& context, const String& name) const;
  ClassPtr getTypeByShortName(ExecutionContext& context, const String& shortName) const;

  ClassPtr findType(const String& name) const;
  bool doTypeExists(const String& type) const;

  void shutdown();

private:
  typedef std::map<String, ClassPtr> TypeMap;
  typedef std::map<String, TemplateClassCache> TemplateClassMap;

  CriticalSection typesLock;
  TypeMap types;
  TypeMap typesByShortName;
  TemplateClassMap templateTypes;
 
  bool hasTemplateClass(const String& templateTypeName) const;
  TemplateClassCache* getTemplateClass(ExecutionContext& context, const String& templateTypeName) const;

  static String removeAllSpaces(const String& str);
};

extern ClassManager& typeManager();

extern ClassPtr getType(const String& typeName);
extern ClassPtr getType(const String& name, const std::vector<ClassPtr>& arguments);
extern bool doTypeExists(const String& typeName);
extern bool declareType(ClassPtr type);

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_TYPE_MANAGER_H_
