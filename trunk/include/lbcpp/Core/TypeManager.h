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

  void declare(ExecutionContext& context, TypePtr type);
  void declare(ExecutionContext& context, TemplateTypePtr templateType);

  void finishDeclarations(ExecutionContext& context);

  TypePtr getType(ExecutionContext& context, const String& typeName, const std::vector<TypePtr>& arguments) const;
  TypePtr getType(ExecutionContext& context, const String& name) const;

  TypePtr findType(const String& name) const;
  bool doTypeExists(const String& type) const;

  void ensureIsInitialized(ExecutionContext& context);
  void shutdown();

private:
  typedef std::map<String, TypePtr> TypeMap;
  typedef std::map<String, TemplateTypeCache> TemplateTypeMap;

  CriticalSection typesLock;
  TypeMap types;
  TemplateTypeMap templateTypes;
 
  bool standardTypesAreDeclared;

  TemplateTypeCache* getTemplateType(ExecutionContext& context, const String& templateTypeName) const;

  static String removeAllSpaces(const String& str);
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_TYPE_MANAGER_H_
