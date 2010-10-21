/*-----------------------------------------.---------------------------------.
| Filename: BoincEvaluator.cpp             | BOINC Evaluator                 |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 12:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Data/Protein.h"
#include "Perception/ProteinPerception.h"
#include "Inference/ProteinInferenceFactory.h"
#include "Inference/ProteinInference.h"
#include "Inference/ContactMapInference.h"
#include "Evaluator/ProteinEvaluator.h"
using namespace lbcpp;

class BoincWorker : public Object
{
public:
  bool initialize(const File& workingDirectory, const File& dataDirectory, MessageCallback& callback = MessageCallback::getInstance())
  {
    if (!workingDirectory.isDirectory() || !workingDirectory.exists())
    {
      callback.errorMessage(T("BoincEvaluator::initialize"), T("Working directory ") + workingDirectory.getFullPathName() + T(" does not exists"));
      return false;
    }
    if (!dataDirectory.isDirectory() || !dataDirectory.exists())
    {
      callback.errorMessage(T("BoincEvaluator::initialize"), T("Data directory ") + dataDirectory.getFullPathName() + T(" does not exists"));
      return false;
    }
    if (!parseInputFile(workingDirectory.getChildFile(T("input.xml")), callback))
      return false;

    this->workingDirectory = workingDirectory;
    this->dataDirectory = dataDirectory;
    return initializeWorker(callback);
  }

  bool run()
  {
    if (!runWorker(MessageCallback::getInstance()))
      return false;
    return generateOutputFile(workingDirectory.getChildFile(T("output.xml")), MessageCallback::getInstance());
  }
 
  void updateProgression(const String& message, double percentage)
  {
    std::cout << "[" << String(percentage * 100, 1) << "%] " << message << std::endl;
    File progressFile = workingDirectory.getChildFile("fractiondone");
    if (progressFile.existsAsFile())
      progressFile.deleteFile();
    OutputStream* ostr = progressFile.createOutputStream();
    (*ostr) << String(percentage) + T("\n");
    delete ostr;
  }

protected:
  std::map<String, String> parameters;
  File workingDirectory;
  File dataDirectory;

  virtual bool initializeWorker(MessageCallback& callback)
    {return true;}

  virtual bool runWorker(MessageCallback& callback) = 0;

  virtual void saveWorkerModel(XmlExporter& exporter)
    {}

  virtual void getWorkerScores(std::vector< std::pair<String, double> >& res) = 0;

private:
  bool parseInputFile(const File& file, MessageCallback& callback)
  {
    if (!file.existsAsFile())
    {
      callback.errorMessage(T("BoincEvaluator::parseInputFile"), T("File ") + file.getFullPathName() + T(" does not exists"));
      return false;
    }
    juce::XmlDocument document(file);
    
    XmlElement* elt = document.getDocumentElement();
    String lastParseError = document.getLastParseError();
    if (!elt)
    {
      callback.errorMessage(T("BoincEvaluator::parseInputFile"),
        lastParseError.isEmpty() ? T("Could not parse file ") + file.getFullPathName() : lastParseError);
      return false;
    }

    forEachXmlChildElementWithTagName(*elt, param, T("param"))
    {
      String name = param->getStringAttribute(T("name"));
      String value = param->getStringAttribute(T("value"));
      if (name.isEmpty() || value.isEmpty())
      {
        callback.errorMessage(T("BoincEvaluator::parseInputFile"), T("Missing name or value in parameter"));
        return false;
      }
      if (parameters.find(name) != parameters.end())
      {
        callback.errorMessage(T("BoincEvaluator::parseInputFile"), T("Parameter ") + name.quoted() + T(" multiply defined"));
        return false;
      }
      parameters[name] = value;
    }

    delete elt;
    return true;
  }

  bool generateOutputFile(const File& file, MessageCallback& callback)
  {
    XmlExporter exporter(T("result"), 0);

    // parameters
    exporter.enter(T("parameters"));
    for (std::map<String, String>::const_iterator it = parameters.begin(); it != parameters.end(); ++it)
    {
      exporter.enter(T("param"));
      exporter.setAttribute(T("name"), it->first);
      exporter.setAttribute(T("value"), it->second);
      exporter.leave();
    }
    exporter.leave();

    // model
    exporter.enter(T("model"));
    saveWorkerModel(exporter);
    exporter.leave();
    
    // scores
    exporter.enter(T("scores"));
    std::vector< std::pair<String, double> > scores;
    getWorkerScores(scores);
    for (size_t i = 0; i < scores.size(); ++i)
    {
      exporter.enter(T("score"));
      exporter.setAttribute(T("name"), scores[i].first);
      exporter.setAttribute(T("value"), scores[i].second);
      exporter.leave();
    }
    exporter.leave();

    return exporter.saveToFile(file, callback);
  }
};

class ExampleProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  ExampleProteinInferenceFactory(const std::map<String, String>& parameters)
    : parameters(parameters) {}

  virtual InferencePtr createTargetInference(const String& targetName) const
  {
    InferencePtr res = ProteinInferenceFactory::createTargetInference(targetName);
    res->setBatchLearner(onlineToBatchInferenceLearner());
    return res;
  }
  /* FIXME: support these paremeters:

      <param name="includeGlobalHistograms" type="Boolean"/>
    <param name="includeLocalHistograms" type="Boolean"/>
    <param name="includeLocalAAWindow" type="Boolean"/>
    <param name="includeBoundsProximity" type="Boolean"/>

  */

  virtual void getPerceptionRewriteRules(PerceptionRewriterPtr rewriter) const
  {
    rewriter->addRule(booleanType, booleanFeatures());
    rewriter->addRule(enumValueFeaturesPerceptionRewriteRule());
    rewriter->addRule(negativeLogProbabilityType, defaultPositiveDoubleFeatures(getIntegerParameter(T("numIntervalsPerEntropy")), -3, 3));
    rewriter->addRule(probabilityType, defaultProbabilityFeatures(getIntegerParameter(T("numIntervalsPerProbability"))));
    rewriter->addRule(positiveIntegerType, defaultPositiveIntegerFeatures(getIntegerParameter(T("numIntervalsPerPositiveInteger"))));
    rewriter->addRule(integerType, defaultIntegerFeatures());

    // all other features
    rewriter->addRule(doubleType, identityPerception());
  }

  virtual PerceptionPtr createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
    {return ProteinInferenceFactory::createPerception(targetName, is1DTarget, is2DTarget);}

public:
  virtual InferencePtr createContactMapInference(const String& targetName) const
    {jassert(false); return InferencePtr();}

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
    {jassert(false); return InferencePtr();}

  IterationFunctionPtr createLearningRate() const
  {
    double learningRate = pow(10.0, getNumericalParameter(T("learningRate")));
    double learningRateDecrease = pow(10.0, getNumericalParameter(T("learningRateDecrease")));
    if (learningRateDecrease < 0 || getNumericalParameter(T("learningRateDecrease")) == 10.0)
      return constantIterationFunction(learningRate);
    else
      return invLinearIterationFunction(learningRate, (size_t)learningRateDecrease);
  }

  ScalarObjectFunctionPtr createRegularizer() const
  {
    double l1Weight = getNumericalParameter(T("l1RegularizerWeight"));
    l1Weight = (l1Weight <= -10.0) ? 0.0 : pow(10.0, l1Weight);
    
    double l2Weight = getNumericalParameter(T("l2RegularizerWeight"));
    l2Weight = (l2Weight <= -10.0) ? 0.0 : pow(10.0, l2Weight);

    // FIXME: for the moment L1 regularization is not implemented
    return l2Weight ? l2RegularizerFunction(l2Weight) : ScalarObjectFunctionPtr();
  }

  InferenceOnlineLearnerPtr createOnlineLearner() const
  {
    StoppingCriterionPtr stoppingCriterion = maxIterationsStoppingCriterion(10);

    InferenceOnlineLearnerPtr res = gradientDescentOnlineLearner(
        getUpdateFrequencyParameter(T("randomizationFrequency")),                            // randomization
        getUpdateFrequencyParameter(T("learningStepsFrequency")), createLearningRate(), true,// learning steps
        getUpdateFrequencyParameter(T("regularizerFrequency")), createRegularizer());         // regularizer
    res->setNextLearner(stoppingCriterionOnlineLearner(
        InferenceOnlineLearner::perPass, stoppingCriterion, getBooleanParameter(T("restoreBestParametersWhenFinished")))); // stopping criterion
    return res;
  }

  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
  {
    String multiClassClassifier = getParameter(T("multiClassInference"));
    
    if (multiClassClassifier == T("multiClassMaxent"))
      return multiClassMaxentInference(perception, classes, createOnlineLearner(), targetName);
    else if (multiClassClassifier == T("multiClassLinearSVM"))
      return multiClassLinearSVMInference(perception, classes, createOnlineLearner(), false, targetName);
    else if (multiClassClassifier == T("multiClassLinearSVMMostViolated"))
      return multiClassLinearSVMInference(perception, classes, createOnlineLearner(), true, targetName);
    else if (multiClassClassifier == T("oneAgainstAllLinearSVM"))
      return oneAgainstAllClassificationInference(targetName, classes, binaryLinearSVMInference(perception, createOnlineLearner(), targetName));
    else if (multiClassClassifier == T("oneAgainstAllLogisticRegression"))
      return oneAgainstAllClassificationInference(targetName, classes, binaryLogisticRegressionInference(perception, createOnlineLearner(), targetName));
    else
    {
      MessageCallback::error(T("createMultiClassClassifier"), T("Unrecognized classifier name: ") + multiClassClassifier);
      return InferencePtr();
    }
  }

protected:
  std::map<String, String> parameters;

  String getParameter(const String& name) const
  {
    std::map<String, String>::const_iterator it = parameters.find(name);
    if (it == parameters.end())
    {
      MessageCallback::error(T("getParameterValue"), T("Missing parameter ") + name);
      return String::empty;
    }
    return it->second;
  }

  double getNumericalParameter(const String& name) const
  {
    String res = getParameter(name);
    return res.isNotEmpty() ? res.getDoubleValue() : 0.0;
  }

  int getIntegerParameter(const String& name) const
  {
    String res = getParameter(name);
    return res.isNotEmpty() ? res.getIntValue() : 0;
  }

  bool getBooleanParameter(const String& name) const
    {return getParameter(name) == T("yes");}

  InferenceOnlineLearner::UpdateFrequency getUpdateFrequencyParameter(const String& parameterName) const
  {
    String value = getParameter(parameterName);
    if (value.isEmpty() || value == T("never"))
      return InferenceOnlineLearner::never;
    else if (value == T("perStep"))
      return InferenceOnlineLearner::perStep;
    else if (value.startsWith(T("perMiniBatch")))
      return (InferenceOnlineLearner::UpdateFrequency)(InferenceOnlineLearner::perStepMiniBatch + value.substring(strlen("perMiniBatch")).getIntValue());
    else if (value == T("perEpisode"))
      return InferenceOnlineLearner::perEpisode;
    else if (value == T("perPass"))
      return InferenceOnlineLearner::perPass;
    else
    {
      MessageCallback::error(T("getUpdateFrequencyParameter"), T("Unrecognized value: ") + value);
      return InferenceOnlineLearner::never;
    }
  }
};

class ExampleBoincWorker : public BoincWorker
{
protected:
  ContainerPtr loadProteins(const File& directory)
  {
    return directoryFileStream(directory)->load()->apply(loadFromFileFunction(proteinClass))
      ->apply(proteinToInputOutputPairFunction(), false)->randomize();
  }

  virtual bool initializeWorker(MessageCallback& callback)
  {
    trainingProteins = loadProteins(dataDirectory.getChildFile(T("train")));
    testingProteins = loadProteins(dataDirectory.getChildFile(T("test")));
    if (!trainingProteins || !trainingProteins->getNumElements() || !testingProteins || !testingProteins->getNumElements())
    {
      callback.error(T("initializeWorker"), T("Could not load training or testing proteins"));
      return false;
    }
    std::cout << trainingProteins->getNumElements() << " training proteins, " << testingProteins->getNumElements() << " testing proteins" << std::endl;

    ExampleProteinInferenceFactory factory(parameters);

    inference = new ProteinSequentialInference();
    inference->appendInference(factory.createInferenceStep(T("secondaryStructure")));
    return true;
  }

  struct Callback : public InferenceCallback
  {
    Callback(ExampleBoincWorker* worker)
      : worker(worker), counter(0)
    {
      worker->updateProgression(T("Learning Iteration 1"), 0.0);
    }

    ExampleBoincWorker* worker;
    size_t counter;

    virtual void postInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
    {
      String inferenceName = stack->getCurrentInference()->getName();
      if (inferenceName == T("LearningPass"))
      {
        ++counter;
        if (counter < 10)
          worker->updateProgression(T("Learning Iteration ") + String((int)counter + 1), counter / 10.0);
      }
    }
  };

  virtual bool runWorker(MessageCallback& callback)
  {
    InferenceContextPtr context = singleThreadedInferenceContext();
    context->appendCallback(new Callback(this));
    context->train(inference, trainingProteins);

    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    context->evaluate(inference, testingProteins, evaluator);
    std::cout << "Evaluation: " << evaluator->toString() << std::endl;

    testAccuracy = evaluator->getEvaluatorForTarget(T("secondaryStructure"))->getDefaultScore();
    return true;
  }

  virtual void getWorkerScores(std::vector< std::pair<String, double> >& res)
    {res.push_back(std::make_pair(T("testAccuracy"), testAccuracy));}

  virtual void saveWorkerModel(XmlExporter& exporter)
    {inference->saveToXml(exporter);}

protected:
  VectorSequentialInferencePtr inference;
  ContainerPtr trainingProteins;
  ContainerPtr testingProteins;
  double testAccuracy;
};

extern void declareProteinClasses();

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    std::cerr << "Usage: " << argv[0] << " dataDirectory " << std::endl;
    return 1;
  }

  lbcpp::initialize();
  declareProteinClasses();

  File workingDirectory = File::getCurrentWorkingDirectory();
  File dataDirectory = workingDirectory.getChildFile(argv[1]);

  ExampleBoincWorker worker;
  if (!worker.initialize(workingDirectory, dataDirectory))
    return 1;

  return worker.run();
}
