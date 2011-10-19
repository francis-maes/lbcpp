/*-----------------------------------------.---------------------------------.
| Filename: LuapeProblem.h                 | Lua Program Evolution Problem   |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_PROBLEM_H_
# define LBCPP_LUAPE_PROBLEM_H_

namespace lbcpp
{

class LuapeOperation : public Object
{
public:
  LuapeOperation(const String& name, int functionReference, const std::vector<TypePtr>& inputTypes, TypePtr outputType)
    : name(name), functionReference(functionReference), inputTypes(inputTypes), outputType(outputType) {}
  LuapeOperation() {}

protected:
  friend class LuapeOperationClass;

  String name;
  int functionReference;
  std::vector<TypePtr> inputTypes;
  TypePtr outputType; // one day: output type that can refer to input types
};

typedef ReferenceCountedObjectPtr<LuapeOperation> LuapeOperationPtr;

class LuapeProblem : public Object
{
public:
  LuapeProblem() : failed(false) {}

  static int input(LuaState& state);
  static int operation(LuaState& state);
  //static int objective(LuaState& state);
  
protected:
  friend class LuapeProblemClass;

  std::vector<VariableSignaturePtr> inputs;
  std::vector<LuapeOperationPtr> operations;
  bool failed;
};

extern ClassPtr luapeProblemClass;
typedef ReferenceCountedObjectPtr<LuapeProblem> LuapeProblemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_PROBLEM_H_
