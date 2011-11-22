/*-----------------------------------------.---------------------------------.
| Filename: LuapeSandBox.h                 | Luape Sand Box                  |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 11:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_SAND_BOX_H_
# define LBCPP_LUAPE_SAND_BOX_H_

# include <lbcpp/Data/Stream.h>
# include "LuapeProblem.h"
# include "LuapeLearner.h"
# include "PolicyBasedWeakLearner.h"
# include "../Core/SinglePlayerMCTSOptimizer.h"

namespace lbcpp
{

class LuapeSandBox : public WorkUnit
{
public:
  LuapeSandBox() : maxExamples(0), maxSteps(3), budgetPerIteration(1000), maxIterations(10) {}

  virtual Variable run(ExecutionContext& context)
  {
    DynamicClassPtr inputClass = new DynamicClass("inputs");
    DefaultEnumerationPtr labels = new DefaultEnumeration("labels");
    ContainerPtr trainData = loadData(context, trainFile, inputClass, labels);
    ContainerPtr testData = loadData(context, testFile, inputClass, labels);
    if (!trainData || !testData)
      return false;

    //context.resultCallback("train", trainData);
    //context.resultCallback("test", testData);

    context.informationCallback(
      String((int)trainData->getNumElements()) + T(" training examples, ") +
      String((int)testData->getNumElements()) + T(" testing examples, ") + 
      String((int)inputClass->getNumMemberVariables()) + T(" input variables,") +
      String((int)labels->getNumElements()) + T(" labels"));

    LuapeProblemPtr problem = createProblem(inputClass);

    LuapeInferencePtr classifier = new LuapeClassifier();
    if (!classifier->initialize(context, inputClass, labels))
      return false;

    //BoostingWeakLearnerPtr weakLearner = singleStumpWeakLearner();
    // FIXME: reset TreeBasedRandomPolicy
    BoostingWeakLearnerPtr weakLearner = policyBasedWeakLearner(new RandomPolicy(), budgetPerIteration, maxSteps);
    classifier->setBatchLearner(new LuapeBatchLearner(adaBoostMHLearner(weakLearner), problem, maxIterations));
    classifier->setEvaluator(defaultSupervisedEvaluator());

    classifier->train(context, trainData, testData, T("Training"), true);
    //classifier->evaluate(context, trainData, EvaluatorPtr(), T("Evaluating on training data"));
    //classifier->evaluate(context, testData, EvaluatorPtr(), T("Evaluating on testing data"));*/
    return true;
  }

protected:
  friend class LuapeSandBoxClass;

  File trainFile;
  File testFile;
  size_t maxExamples;
  size_t maxSteps;
  size_t budgetPerIteration;
  size_t maxIterations;

  ContainerPtr loadData(ExecutionContext& context, const File& file, DynamicClassPtr inputClass, DefaultEnumerationPtr labels) const
  { 
    context.enterScope(T("Loading ") + file.getFileName());
    ContainerPtr res = classificationARFFDataParser(context, file, inputClass, labels)->load(maxExamples);
    if (res && !res->getNumElements())
      res = ContainerPtr();
    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
  }

  LuapeProblemPtr createProblem(DynamicClassPtr inputClass)
  {
    LuapeProblemPtr res = new LuapeProblem();
    size_t n = inputClass->getNumMemberVariables();
    for (size_t i = 0; i < n; ++i)
    {
      VariableSignaturePtr variable = inputClass->getMemberVariable(i);
      res->addInput(variable->getType(), variable->getName());
    }

    res->addFunction(addDoubleLuapeFunction());
    res->addFunction(subDoubleLuapeFunction());
    res->addFunction(mulDoubleLuapeFunction());
    res->addFunction(divDoubleLuapeFunction());
    res->addFunction(andBooleanLuapeFunction());
    res->addFunction(equalBooleanLuapeFunction());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_SAND_BOX_H_
