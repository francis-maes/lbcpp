/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmGenerator.cpp       | CR-algorithm top-level generator|
| Author  : Francis Maes                   |                                 |
| Started : 01/02/2009 18:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "CRAlgorithmGenerator.h"

CRAlgorithmGenerator::CRAlgorithmGenerator(const FunctionPTreeAnalyser& input, SymbolLookup::FunctionScope* scope)
  : input(input)
{
  /*
  ** Call CRAlgorithmScopeGenerator
  */
  ParameterListPTreeAnalyser p = input.getParameters();
  for (size_t i = 0; i < p.size(); ++i)
    parameters.push_back(p[i]);
  std::string className = input.getIdentifierString() + "CRAlgorithm";
  CRAlgorithmVariableTranslator translator(scope, className, input.getReturnType().getPTree());
  CRAlgorithmScopeGenerator::prepare(translator, input.getBody(), scope, className);
  
  /*
  ** Generate CR-algorithm specific part
  */
  beginPart("CR-algorithm top-level functions", publicKeyword());
  
  // getName()
  body.add(atom("static const char* getName() {return " + PTreeGenerator::quote(input.getIdentifierString()) + ";}\n"));

  // return value
  if (hasReturnValue())
  {
    beginPart("Result variable", privateKeyword());
    body.addVariableDeclaration(input.getReturnType(), atom("*__return__"));
    body.addVariableDeclaration(atom("bool"), atom("__isReturnOwner__"));
    body.addAccessSpecifier(publicKeyword());
    body.add(atom("bool hasReturn() const {return __return__ != NULL;}\n"));
    body.add(list(atom("const"), input.getReturnType(), atom("* getReturn() const {return __return__;}\n")));
  }
  else
  {
    beginPart("Void result", publicKeyword());
    body.add(atom("bool hasReturn() const {return false;}\n"));
    body.add(atom("const void* getReturn() const {return NULL;}\n"));
  }
  body.add(atom("const char* getReturnType() const {return " + quote(input.getReturnTypeString()) + ";}\n"));
  body.add(list(atom("typedef "), input.getReturnType(), atom(" ReturnType;\n")));
}

PTree::Node* CRAlgorithmGenerator::createCode()
{
  ParameterListPTreeAnalyser parameters = input.getParameters();

  /*
  ** Class Declaration
  */
  BlockPTreeGenerator block;
  block.add(PTree::list(atom("// "), PTree::first(input.getPTree()), PTree::second(input.getPTree()), atom("\n")));
  block.add(CRAlgorithmScopeGenerator::createDeclaration()); // Class declaration

  /*
  ** Contruction function
  e.g.: 
CRAlgorithm* leftRightLabeling(const std::vector<FeatureGeneratorPtr>& x, const std::set<std::string>& labels, const std::vector<std::string>* ycorrect, size_t contextSize)
  {return lcpp::staticToDynamicCRAlgorithm(new leftRightLabelingCRAlgorithm(x, classes, &correct));}
  */  
  FunctionPTreeGenerator createFunction;
  createFunction.addModifier(inlineKeyword());
  createFunction.setReturnType(atom("lcpp::CRAlgorithmPtr"));
  createFunction.setIdentifier(input.getIdentifier());
  for (size_t i = 0; i < parameters.size(); ++i)
    createFunction.addParameter(parameters[i]);
  FuncallPTreeGenerator newCall;
  newCall.setName("new " + input.getIdentifierString() + "CRAlgorithm");
  for (size_t i = 0; i < parameters.size(); ++i)
    newCall.addArgument(parameters[i].getIdentifier());
  createFunction.body.add(returnStatement(funcallExpr(atom("lcpp::staticToDynamicCRAlgorithm"), newCall.createExpression())));
  block.add(createFunction.createDeclaration());

  /*
  ** Policy run function
  e.g.:
std::vector<std::string> leftRightLabeling(lcpp::PolicyPtr policy, const std::vector<FeatureGeneratorPtr>& x, const std::set<std::string>& labels, const std::vector<std::string>* ycorrect, size_t contextSize)
{
  lcpp::CRAlgorithmPtr __crAlgorithm__ = leftRightLabeling(x, labels, ycorrect, contextSize);
  __crAlgorithm__->run(policy);
  return *(const std::vector<std::string>* )__crAlgorithm__->getResult();
}
  */  
  FunctionPTreeGenerator runFunction;
  runFunction.addModifier(inlineKeyword());
  runFunction.setReturnType(input.getReturnType());
  runFunction.setIdentifier(input.getIdentifier());
  runFunction.addParameter(atom("lcpp::PolicyPtr"), atom("__policy__"));
  for (size_t i = 0; i < parameters.size(); ++i)
    runFunction.addParameter(parameters[i]);

  FuncallPTreeGenerator dynCall;
  dynCall.setIdentifier(input.getIdentifier());
  for (size_t i = 0; i < parameters.size(); ++i)
    dynCall.addArgument(parameters[i].getIdentifier());
  runFunction.body.addVariableDeclaration(atom("lcpp::CRAlgorithmPtr"), atom("__crAlgorithm__"), 
    dynCall.createExpression());
  runFunction.body.addExpressionStatement(funcallExpr(atom("__crAlgorithm__->run"), list(atom("__policy__"))));
  if (!input.getReturnType().isVoid())
    runFunction.body.add(returnStatement(atom("__crAlgorithm__->getReturn()->getReference< " + input.getReturnTypeString() + " >()")));

  block.add(runFunction.createDeclaration());

  return block.createContent();
}

void CRAlgorithmGenerator::customizeCopyConstructor(ConstructorPTreeGenerator& generator)
{
  if (hasReturnValue())
  {
    generator.addInitializer(identifier("__return__"), atom("other.__return__ ? new " + input.getReturnTypeString() + "(*other.__return__) : NULL"));
    generator.addInitializer(identifier("__isReturnOwner__"), atom("true"));
  }
}

void CRAlgorithmGenerator::customizeDefaultConstructor(ConstructorPTreeGenerator& generator)
{
  if (hasReturnValue())
  {
    generator.addInitializer(identifier("__return__"), atom("NULL"));
    generator.addInitializer(identifier("__isReturnOwner__"), atom("false"));
  }
}

void CRAlgorithmGenerator::customizeDestructor(FunctionPTreeGenerator& generator)
{
  if (hasReturnValue())
  {
    generator.body.addExpressionStatement(atom("if (__isReturnOwner__ && __return__) delete __return__"));
  }
}
