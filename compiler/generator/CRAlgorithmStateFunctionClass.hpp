/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmStateFunctionC...hpp| State dependent functions       |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2009 00:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_STATE_ACTION_PAIR_FUNCTION_H_
# define CRALGORITHM_GENERATOR_STATE_ACTION_PAIR_FUNCTION_H_

# include "FeatureGeneratorClassGenerator.h"
# include "../tools/ErrorManager.h"
# include "../tools/PTreeAnalyser.h"

class CRAlgorithmStateFunctionClass : public ClassPTreeGenerator
{
public:
  CRAlgorithmStateFunctionClass(CRAlgorithmVariableTranslator& translator, CRAlgo::StateFundefStatement* stateFundef, SymbolLookup::UserScope* scope)
    : additionalCode(NULL)
  {
    assert(stateFundef);
    FunctionPTreeAnalyser input(translateFunctionDefinition(translator, stateFundef, scope));
    ParameterListPTreeAnalyser parameters(input.getParameters());
    std::string identifier = input.getIdentifierString();
    
    setKeyword(structKeyword());
    setName("__" + identifier + "Function__");
    
    // name, returnType, parameters
    body.add(atom("static const char* getName() {return " + quote(identifier) + ";}\n"));
    body.add(list(typedefKeyword(), input.getReturnType(), atom("__ReturnType__;\n")));
    
    for (size_t i = 0; i < parameters.size(); ++i)
    {
      ParameterPTreeAnalyser param = parameters[i];
      body.add(list(typedefKeyword(), param.getRecomposedType(false), atom("__Param" + size2str(i) + "Type__;\n")));
    }
    body.add(atom("enum {numParameters = " + size2str(parameters.size()) + "};\n"));

    // function body
    FunctionPTreeGenerator function;
    function.addModifiers(input.getModifiers());
    function.addModifier(staticKeyword());
    function.setReturnType(input.getReturnType());
    function.setName("function");
    function.addParameters(parameters.getPTreeVector());
    function.body.add(input.getBody());
    
    body.add(function.createDeclaration());
    
    if (input.getReturnType().getString() == "featureGenerator")
      body.add(atom("typedef " + identifier + "FeatureGenerator FeatureGenerator;\n"));
  }
  
  void createCode(BlockPTreeGenerator& block)
  {
    if (additionalCode)
      block.add(additionalCode);
    block.add(createDeclaration());
  }
  
private:
  PTree::Node* additionalCode;

  PTree::FunctionDefinition* translateFunctionDefinition(CRAlgorithmVariableTranslator& translator, CRAlgo::StateFundefStatement* stateFundef, SymbolLookup::UserScope* scope)
  {
    PTree::FunctionDefinition* fundef = stateFundef->getFunctionDefinition();
    FunctionPTreeAnalyser input(fundef);
    FunctionPTreeGenerator output;
    
    // add parameter __crAlgorithm__ and translate variables
    output.addModifiers(input.getModifiers());
    output.setReturnType(input.getReturnType());
    output.setIdentifier(input.getIdentifier());
    output.addParameter(atom(translator.getCRAlgorithmClassName()), list(atom("&"), identifier("__crAlgorithm__")));
    output.addParameters(input.getParameters());
    CRAlgorithmVariableTranslatorVisitor visitor(translator);
    output.body.add(visitor.translateVariables(input.getBody(), scope, false));
    fundef = output.createDeclaration();
    
    // feature generators
    if (CRAlgo::isFeatureGenerator(fundef))
    {
      // replace fundef by the staticToDynamic feature generator function
      FeatureGeneratorClassGenerator featureGeneratorGenerator(fundef, scope, true);
      additionalCode = featureGeneratorGenerator.createCode(&fundef);
    }
    return fundef;
  }
};

#endif // !CRALGORITHM_GENERATOR_STATE_ACTION_PAIR_FUNCTION_H_
