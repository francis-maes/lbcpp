/*-----------------------------------------.---------------------------------.
| Filename: TypeManager.h                  | Global Type Manager             |
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

struct TemplateTypeCache;
class TypeManager
{
public:
  TypeManager();
  ~TypeManager();

  bool declare(ExecutionContext& context, ClassPtr type);
  bool declare(ExecutionContext& context, TemplateTypePtr templateType);

  void finishDeclarations(ExecutionContext& context);

  ClassPtr getType(ExecutionContext& context, const String& typeName, const std::vector<ClassPtr>& arguments) const;
  ClassPtr getType(ExecutionContext& context, const String& name) const;
  ClassPtr getTypeByShortName(ExecutionContext& context, const String& shortName) const;

  ClassPtr findType(const String& name) const;
  bool doTypeExists(const String& type) const;

  void shutdown();

private:
  typedef std::map<String, ClassPtr> TypeMap;
  typedef std::map<String, TemplateTypeCache> TemplateTypeMap;

  CriticalSection typesLock;
  TypeMap types;
  TypeMap typesByShortName;
  TemplateTypeMap templateTypes;
 
  bool hasTemplateType(const String& templateTypeName) const;
  TemplateTypeCache* getTemplateType(ExecutionContext& context, const String& templateTypeName) const;

  static String removeAllSpaces(const String& str);
};

extern TypeManager& typeManager();

extern ClassPtr getType(const String& typeName);
extern ClassPtr getType(const String& name, const std::vector<ClassPtr>& arguments);
extern bool doTypeExists(const String& typeName);
extern bool declareType(ClassPtr type);

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_TYPE_MANAGER_H_
