/*-----------------------------------------.---------------------------------.
| Filename: ChooseClassGenerator.cpp       | Choose class generator          |
| Author  : Francis Maes                   |                                 |
| Started : 19/02/2009 21:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ChooseClassGenerator.h"
#include "../tools/PTreeAnalyser.h"

/*
  struct __Choose0Parameters__
  {
    typedef LabelDictionary ContainerType;
    typedef size_t ChoiceType;
    
    enum
    {
      numStateValues = 0,
      numActionValues = 2,
      // ...
      
    };
    
    const cralgo::ActionValueFunction& getActionValueFunction(size_t i)
    {
      switch (i) { ... }
    }
    
    ~__Choose0__()
    {
      delete fun1;
      delete fun2;
      delete fun3;
    }
    
    static __Choose0Parameters__& getInstance()
      {static __Choose0Parameters__ instance; return instance;}


  private:
    cralgo::ActionValueFunction* fun1;
    cralgo::ActionValueFunction* fun2;
    cralgo::StateFeaturesFunction* fun3;

    cralgo::StateDescriptionFunction* __stateDescriptionFunction__;
    cralgo::ActionDescriptionFunction* __actionDescriptionFunction__;
    // ...    
    
    __Choose0__()
    {
      fun1 = cralgo::staticToDynamicStateFunction<cralgo::ActionValueFunction, __aFunction__, CRAlgorithmType>();
      fun2 = cralgo::staticToDynamicStateFunction<cralgo::ActionValueFunction, __aFunction__, CRAlgorithmType>();
      // ...
    }
  };
*/

static const char* stateFunctionKinds[] = {
  "StateDescription", "ActionDescription",
  "StateValue", "ActionValue",
  "StateFeatures", "ActionFeatures",
  "FeatureGenerator",
  "Misc"
};
static const size_t numStateFunctionKinds
  = sizeof (stateFunctionKinds) / sizeof (const char* );
static const size_t numCompositeStateFunctionKinds = numStateFunctionKinds - 2;

std::string ChooseClassGenerator::StateFunctionInfo::findKind(CRAlgo::StateFundefStatement* stateFunction)
{
  // todo: finish and make more tests
  FunctionPTreeAnalyser input(stateFunction->getFunctionDefinition());
  
  std::string returnType = input.getReturnTypeString();
  size_t numParams = input.getParameters().size();
  if (returnType == "std :: string")
  {
    if (numParams == 0)
      return "StateDescription";
    else if (numParams == 1)
      return "ActionDescription";
  }
  if (returnType == "featureGenerator")
  {
    if (numParams == 0)
      return "StateFeatures";
    else if (numParams == 1)
      return "ActionFeatures";
    //else
    //  return "FeatureGenerator";
  }
  else if (returnType == "double")
  {
    if (numParams == 0)
      return "StateValue";
    else if (numParams == 1)
      return "ActionValue";
  }
  return "";//Misc"; 
}

ChooseClassGenerator::ChooseClassGenerator(CRAlgorithmChoose& choose, SymbolLookup::Scope* scope, const std::string& crAlgorithmClassName)
  : className(choose.getParametersClassName()), crAlgorithmClassName(crAlgorithmClassName)
{
  for (size_t i = 0; i < choose.getNumStateFunctions(); ++i)
  {
    CRAlgo::StateFundefStatement* stateFundef = choose.getStateFunctionDefinition(i);
    StateFunctionInfo info(PTree::reify(choose.getStateFunctionArgument(i)), stateFundef);
    if (info.kind.size())
      stateFunctions.push_back(info);
  }

  setKeyword(structKeyword());
  setName(className);
  body.add(createDestructor());
  body.add(atom("typedef " + PTree::reify(choose.getChooseType()) + " ChoiceType;\n"));
  body.add(atom("typedef " + choose.getContainerType() + " ContainerType;\n"));
  body.add(atom("static " + className + "& getInstance() {static " + className + " instance; return instance;}\n"));

  body.addNewLine();  
  body.add(createNumStateValuesEnum());
  body.addNewLine();
  for (size_t i = 0; i < numCompositeStateFunctionKinds; ++i)
    body.add(createStateFunctionGetter(stateFunctionKinds[i]));

  body.addNewLine();
  for (size_t i = 0; i < numCompositeStateFunctionKinds; ++i)
    body.add(createCompositeStateFunctionGetter(stateFunctionKinds[i]));    
  body.addNewLine();
  body.addAccessSpecifier(privateKeyword());
  addMemberVariables(body);
  body.add(createConstructor());
}

void ChooseClassGenerator::addMemberVariables(BlockPTreeGenerator& classBody)
{
  for (size_t i = 0; i < stateFunctions.size(); ++i)
  {
    StateFunctionInfo& sf = stateFunctions[i];
    classBody.addVariableDeclaration(atom(sf.getDynamicClassName() + "*"), identifier(sf.identifier));
  }
  classBody.addNewLine();
  for (size_t i = 0; i < numCompositeStateFunctionKinds; ++i)
    classBody.addVariableDeclaration(atom("const " + getDynamicClassName(stateFunctionKinds[i]) + "*"),
      identifier(std::string("__") + stateFunctionKinds[i] + "Function__"));
}

std::vector<ChooseClassGenerator::StateFunctionInfo> ChooseClassGenerator::getStateFunctionsOfKind(const std::string& kind) const
{
  std::vector<StateFunctionInfo> res;
  for (size_t i = 0; i < stateFunctions.size(); ++i)
    if (stateFunctions[i].kind == kind)
      res.push_back(stateFunctions[i]);
  return res;
}

size_t ChooseClassGenerator::getNumStateFunctionsOfKind(const std::string& kind) const
{
  size_t count = 0;
  for (size_t i = 0; i < stateFunctions.size(); ++i)
    if (stateFunctions[i].kind == kind)
      ++count;
  return count;
}

PTree::Node* ChooseClassGenerator::createNumStateValuesEnum()
{
  ClassPTreeGenerator output;
  output.setKeyword(enumKeyword());
  for (size_t i = 0; i < numCompositeStateFunctionKinds; ++i)
  {
    size_t count = getNumStateFunctionsOfKind(stateFunctionKinds[i]);
    output.body.add(atom(std::string("num") + stateFunctionKinds[i] + "Functions = " + size2str(count) + ",\n"));
  }
  return output.createDeclaration();
}

PTree::Node* ChooseClassGenerator::createStateFunctionGetter(const std::string& stateFunctionKind)
{
  FunctionPTreeGenerator output;
  std::string returnType = "const " + getDynamicClassName(stateFunctionKind);
  output.setReturnType(atom(returnType + "&"));
  output.setName("get" + stateFunctionKind);
  output.addParameter(atom("size_t"), identifier("i"));
  output.setConst(true);
  
  SwitchPTreeGenerator switchOutput;
  switchOutput.setCondition(identifier("i"));
  size_t index = 0;
  for (size_t i = 0; i < stateFunctions.size(); ++i)
    if (stateFunctions[i].kind == stateFunctionKind)
      switchOutput.addCase(literal(index++), returnStatement(atom("*" + stateFunctions[i].identifier)));
  switchOutput.setDefault(atom("assert(false); return *(" + returnType + "* )0;\n"));
  output.body.add(switchOutput.createStatement());
  
  return output.createDeclaration();
}

PTree::Node* ChooseClassGenerator::createCompositeStateFunctionGetter(const std::string& stateFunctionKind)
{
  std::string id = std::string("__") + stateFunctionKind + "Function__";

  FunctionPTreeGenerator output;
  std::string returnType = "const " + getDynamicClassName(stateFunctionKind);
  output.setReturnType(atom(returnType + "&"));
  output.setName("get" + stateFunctionKind + "Function");
  output.setConst(true);
  
  
  PTree::Node* createCode;
  std::vector<StateFunctionInfo> stateFunctions = getStateFunctionsOfKind(stateFunctionKind);

  if (stateFunctions.size() == 0)
    createCode = atom("const_cast<" + className + "* >(this)->" + id + " = cralgo::StateFunctionTraits<" + getDynamicClassName(stateFunctionKind) + ">::createComposite();");
  else if (stateFunctions.size() == 1)
  {
    StateFunctionInfo& sf = stateFunctions[0];
    createCode = 
      atom("const_cast<" + className + "* >(this)->" + id + " = cralgo::StateFunctionTraits<" + sf.getDynamicClassName() + ">::create<" + sf.className + ", " + crAlgorithmClassName + ">();");
  }
  else
  {
    BlockPTreeGenerator block;
    block.add(atom("typedef cralgo::StateFunctionTraits<" + getDynamicClassName(stateFunctionKind) + "> StateFunTraits;\n"));
    block.add(atom("StateFunTraits::CompositeFunction* __tmp__ = StateFunTraits::createComposite();\n"));
    for (size_t i = 0; i < stateFunctions.size(); ++i)
      block.add(atom("__tmp__->add(*" + stateFunctions[i].identifier + ");\n"));
    block.addExpressionStatement(assignExpr(atom("const_cast<" + className + "* >(this)->" + id), atom("__tmp__")));
    createCode = block.createBlock();
  }
    
  output.body.add(ifStatement(atom("!" + id), createCode));
  output.body.add(returnStatement(atom("*" + id)));

  return output.createDeclaration();
}

PTree::Node* ChooseClassGenerator::createConstructor()
{
  ConstructorPTreeGenerator output;
  output.setName(className);
  for (size_t i = 0; i < stateFunctions.size(); ++i)
  {
    StateFunctionInfo& sf = stateFunctions[i];
    PTree::Node* expr = assignExpr(identifier(sf.identifier),
      atom("cralgo::StateFunctionTraits<" + sf.getDynamicClassName() + ">::create<" + sf.className + ", " + crAlgorithmClassName + ">()"));
    output.body.addExpressionStatement(expr);
  }
  for (size_t i = 0; i < numCompositeStateFunctionKinds; ++i)
    output.addInitializer(identifier(std::string("__") + stateFunctionKinds[i] + "Function__"), atom("NULL"));
  if (!output.body.size())
    output.body.add(NULL);
  return output.createDeclaration();
}

PTree::Node* ChooseClassGenerator::createDestructor()
{
  FunctionPTreeGenerator output;
  output.setReturnType(atom("~"));
  output.setName(className);
  for (size_t i = 0; i < stateFunctions.size(); ++i)
  {
    StateFunctionInfo& sf = stateFunctions[i];
    output.body.addExpressionStatement(atom("delete " + sf.identifier));
  }
  for (size_t i = 0; i < numCompositeStateFunctionKinds; ++i)
  {
    std::string id = std::string("__") + stateFunctionKinds[i] + "Function__";
    output.body.add(atom("if (" + id + ") delete " + id + ";\n"));
  }
  return output.createDeclaration();
}
