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
    
    const lcpp::ActionValueFunction& getActionValueFunction(size_t i)
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
    lcpp::ActionValueFunctionPtr fun1;
    lcpp::ActionValueFunctionPtr fun2;
    lcpp::StateFeaturesFunctionPtr fun3;

    lcpp::StateDescriptionFunctionPtr __stateDescriptionFunction__;
    lcpp::ActionDescriptionFunctionPtr __actionDescriptionFunction__;
    // ...    
    
    __Choose0__()
    {
      fun1 = lcpp::staticToDynamicStateFunction<lcpp::ActionValueFunction, __aFunction__, CRAlgorithmType>();
      fun2 = lcpp::staticToDynamicStateFunction<lcpp::ActionValueFunction, __aFunction__, CRAlgorithmType>();
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
  std::string chooseType = PTree::reify(choose.getChooseType());
  if (chooseType.size() && chooseType[chooseType.size() - 1] == ' ')
    chooseType = chooseType.substr(0, chooseType.size() - 1);
    
  body.add(atom("typedef " + chooseType + " ChoiceType;\n"));
  body.add(atom("typedef " + choose.getContainerType() + " ContainerType;\n"));
  body.add(atom("static " + className + "& getInstance() {static " + 
    className + " instance; return instance;}\n"));
  body.add(atom("static std::string getChoiceType() {return " + quote(chooseType) + ";}\n"));

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
    classBody.addVariableDeclaration(atom(sf.getDynamicClassName() + "Ptr"), identifier(sf.identifier));
  }
  classBody.addNewLine();
  for (size_t i = 0; i < numCompositeStateFunctionKinds; ++i)
    classBody.addVariableDeclaration(atom(getDynamicClassName(stateFunctionKinds[i]) + "Ptr"),
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
  std::string returnType = getDynamicClassName(stateFunctionKind) + "Ptr";
  output.setReturnType(atom(returnType));
  output.setName("get" + stateFunctionKind);
  output.addParameter(atom("size_t"), identifier("i"));
  output.setConst(true);
  
  SwitchPTreeGenerator switchOutput;
  switchOutput.setCondition(identifier("i"));
  size_t index = 0;
  for (size_t i = 0; i < stateFunctions.size(); ++i)
    if (stateFunctions[i].kind == stateFunctionKind)
      switchOutput.addCase(literal(index++), returnStatement(atom(stateFunctions[i].identifier)));
  switchOutput.setDefault(atom("assert(false); return " + returnType + "();\n"));
  output.body.add(switchOutput.createStatement());
  
  return output.createDeclaration();
}

PTree::Node* ChooseClassGenerator::createCompositeStateFunctionGetter(const std::string& stateFunctionKind)
{
  std::string id = std::string("__") + stateFunctionKind + "Function__";

  FunctionPTreeGenerator output;
  std::string returnType = getDynamicClassName(stateFunctionKind) + "Ptr";
  output.setReturnType(atom(returnType));
  output.setName("get" + stateFunctionKind + "Function");
  output.setConst(true);

  std::vector<StateFunctionInfo> stateFunctions = getStateFunctionsOfKind(stateFunctionKind);
  
  if (stateFunctions.size() == 0)
    output.body.add(returnStatement(atom(returnType + "()")));
  else
  {
    PTree::Node* createCode;
    if (stateFunctions.size() == 1)
    {
      StateFunctionInfo& sf = stateFunctions[0];
      createCode = atom("const_cast<" + className + "* >(this)->" + id + " = " + sf.identifier + ";");
      // lcpp::StateFunctionTraits<" + sf.getDynamicClassName() + ">::create<" + sf.className + ", " + crAlgorithmClassName + ">();");
    }
    else
    {
      BlockPTreeGenerator block;
      block.add(atom("lcpp::impl::Composite" + stateFunctionKind + "Function __tmp__;\n"));
//      block.add(atom("typedef lcpp::StateFunctionTraits<" + getDynamicClassName(stateFunctionKind) + "> StateFunTraits;\n"));
//      block.add(atom("StateFunTraits::CompositeFunction* __tmp__ = StateFunTraits::createComposite();\n"));
      for (size_t i = 0; i < stateFunctions.size(); ++i)
        block.add(atom("__tmp__.add(" + stateFunctions[i].identifier + ");\n"));
      block.addExpressionStatement(assignExpr(atom("const_cast<" + className + "* >(this)->" + id), atom("lcpp::impl::staticToDynamic(__tmp__)")));
      createCode = block.createBlock();
    }
      
    output.body.add(ifStatement(atom("!" + id), createCode));
    output.body.add(returnStatement(atom(id)));
  }
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
//      atom("lcpp::StateFunctionTraits<" + sf.getDynamicClassName() + ">::create<" + sf.className + ", " + crAlgorithmClassName + ">()"));
        atom("lcpp::impl::staticToDynamic(" + sf.className + "())"));
    output.body.addExpressionStatement(expr);
  }
//  for (size_t i = 0; i < numCompositeStateFunctionKinds; ++i)
//    output.addInitializer(identifier(std::string("__") + stateFunctionKinds[i] + "Function__"), atom("NULL"));
  if (!output.body.size())
    output.body.add(NULL);
  return output.createDeclaration();
}

PTree::Node* ChooseClassGenerator::createDestructor()
{
  FunctionPTreeGenerator output;
  output.setReturnType(atom("~"));
  output.setName(className);
  output.body.add(NULL);
/*  for (size_t i = 0; i < stateFunctions.size(); ++i)
  {
    StateFunctionInfo& sf = stateFunctions[i];
    output.body.addExpressionStatement(atom("delete " + sf.identifier));
  }
  for (size_t i = 0; i < numCompositeStateFunctionKinds; ++i)
  {
    std::string id = std::string("__") + stateFunctionKinds[i] + "Function__";
    output.body.add(atom("if (" + id + ") delete " + id + ";\n"));
  }*/
  return output.createDeclaration();
}
