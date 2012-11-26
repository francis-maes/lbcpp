/*-----------------------------------------.---------------------------------.
| Filename: ClassificationSandBox.h        | Classification SandBox          |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 10:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_CLASSIFICATION_SANDBOX_H_
# define LBCPP_MCGP_CLASSIFICATION_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/ExpressionSampler.h>
# include <lbcpp-ml/SplittingCriterion.h>

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class ClassificationSandBox : public WorkUnit
{
public:
  ClassificationSandBox() : maxExamples(0), verbosity(0) {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);

    // load dataset and display information
    std::vector<VariableExpressionPtr> inputs;
    VariableExpressionPtr supervision;
    TablePtr dataset = loadDataFile(context, dataFile, inputs, supervision);
    if (!dataset || !inputs.size() || !supervision)
      return new Boolean(false);

    size_t numVariables = inputs.size();
    size_t numExamples = dataset->getNumRows();
    size_t numLabels = supervision->getType().staticCast<Enumeration>()->getNumElements();
    context.informationCallback(string((int)numExamples) + T(" examples, ") +
                                string((int)numVariables) + T(" variables, ") +
                                string((int)numLabels) + T(" labels"));
    context.resultCallback("dataset", dataset);

    // make train/test split
    size_t numTrainingSamples = getNumTrainingSamples(dataFile);
    dataset = dataset->randomize(context);
    TablePtr trainingData = dataset->range(0, numTrainingSamples);
    TablePtr testingData = dataset->invRange(0, numTrainingSamples);
    context.informationCallback(string((int)trainingData->getNumRows()) + " training examples, " + string((int)testingData->getNumRows()) + " testing examples");

    // make problem
    ProblemPtr problem = makeProblem(context, inputs, supervision, trainingData, testingData);
    context.resultCallback("problem", problem);

    // make learner
    SamplerPtr expressionVectorSampler = subsetVectorSampler(scalarExpressionVectorSampler(), (size_t)(sqrt((double)numVariables) + 0.5));
    SolverPtr conditionLearner = exhaustiveConditionLearner(expressionVectorSampler);
    //conditionLearner->setVerbosity((SolverVerbosity)verbosity);
    SolverPtr learner = treeLearner(informationGainSplittingCriterion(true), conditionLearner);
    //learner->setVerbosity((SolverVerbosity)verbosity);
    learner = simpleEnsembleLearner(learner, 100);
    learner->setVerbosity((SolverVerbosity)verbosity);

    // learn
    ExpressionPtr model;
    FitnessPtr fitness;
    context.enterScope("Learning");
    learner->solve(context, problem, storeBestSolverCallback(*(ObjectPtr* )&model, fitness));
    context.leaveScope();
    context.resultCallback("model", model);
    context.resultCallback("fitness", fitness);

    // evaluate
    double testingScore = problem->getValidationObjective(0)->evaluate(context, model);
    context.resultCallback("testingScore", testingScore);

    return new Boolean(true);
  }
  
private:
  friend class ClassificationSandBoxClass;

  juce::File dataFile;
  size_t maxExamples;
  size_t verbosity;

  TablePtr loadDataFile(ExecutionContext& context, const juce::File& file, std::vector<VariableExpressionPtr>& inputs, VariableExpressionPtr& supervision)
  {
    context.enterScope(T("Loading ") + file.getFileName());
    TablePtr res = Object::createFromFile(context, file).staticCast<Table>();
    context.leaveScope(res ? res->getNumRows() : 0);
    if (!res)
      return TablePtr();

    // we take all numerical attributes as input and take the latest categorical attribute as supervision
    for (size_t i = 0; i < res->getNumColumns(); ++i)
    {
      if (res->getType(i)->inheritsFrom(doubleClass))
        inputs.push_back(res->getKey(i));
      else if (res->getType(i)->inheritsFrom(enumValueClass))
        supervision = res->getKey(i);
    }
    return res;
  }

  size_t getNumTrainingSamples(const juce::File& dataFile) const
  {
    if (dataFile.getFileName() == T("waveform.jdb"))
      return 300;
    jassertfalse;
    return 0;
  }

  ProblemPtr makeProblem(ExecutionContext& context, const std::vector<VariableExpressionPtr>& inputs, const VariableExpressionPtr& supervision, const TablePtr& trainingData, const TablePtr& testingData)
  {
    ProblemPtr res = new Problem();
    ExpressionDomainPtr domain = new ExpressionDomain();
    domain->addInputs(inputs);
    domain->setSupervision(supervision);

    res->setDomain(domain);
    res->addObjective(multiClassAccuracyObjective(trainingData, supervision));
    res->addValidationObjective(multiClassAccuracyObjective(testingData, supervision));
    return res;
  }
};

}; /* namespace lbcpp */

#endif // LBCPP_MCGP_CLASSIFICATION_SANDBOX_H_
