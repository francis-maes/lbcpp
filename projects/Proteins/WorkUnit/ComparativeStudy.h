
#include <lbcpp/Core/Function.h>
#include "../Predictor/LargeProteinPredictorParameters.h"
#include "../Evaluator/KolmogorovPerfectMatchingFunction.h"

namespace lbcpp
{


class GenerateLargeProteinExperimentsWorkUnit : public WorkUnit
{
public:
  GenerateLargeProteinExperimentsWorkUnit()
    : numParameters(0), numExperiments(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    std::map<String, LargeProteinParametersPtr> uniqueExperiments;
    for (size_t i = 0; i < numExperiments; ++i)
    {
      LargeProteinParametersPtr res = generateExperiment(context);
      if (uniqueExperiments.count(res->toString()) == 1)
      {
        --i;
        continue;
      }

      String fileName = T("Param-") + String((int)numParameters) + T("_Exp-") + String((int)i) + T(".xml");
      res->saveToFile(context, context.getFile(fileName));
      context.enterScope(fileName);
      context.leaveScope(res);

      uniqueExperiments[res->toString()] = res;
    }
    return true;
  }

protected:
  friend class GenerateLargeProteinExperimentsWorkUnitClass;

  size_t numParameters;
  size_t numExperiments;

  LargeProteinParametersPtr generateExperiment(ExecutionContext& context) const
  {
    LargeProteinParametersPtr res = new LargeProteinParameters();
    const size_t n = largeProteinParametersClass->getNumMemberVariables();
    std::vector<bool> notUsedParameters(n, true);
    restrictToResidueParameteres(notUsedParameters);
    for (size_t i = 0; i < numParameters; ++i)
    {
      size_t index;
      do
      {
        index = context.getRandomGenerator()->sampleSize(n);
      } while (!notUsedParameters[index]);
      
      Variable value = sampleValue(context, index);
      res->setVariable(index, value);
      notUsedParameters[index] = false;
    }
    return res;
  }

  Variable sampleValue(ExecutionContext& context, size_t index) const
  {
    const TypePtr varType = largeProteinParametersClass->getMemberVariableType(index);
    const String varName = largeProteinParametersClass->getMemberVariableName(index);
    if (varType->inheritsFrom(booleanType))
      return true;
    if (varName.endsWith(T("WindowSize")))
      return discretizeSampler(gaussianSampler(15, 15), 1, 40)->sample(context, context.getRandomGenerator());
    if (varName.endsWith(T("LocalHistogramSize")))
      return discretizeSampler(gaussianSampler(51, 50), 1, 100)->sample(context, context.getRandomGenerator());
    if (varName == T("separationProfilSize"))
      return discretizeSampler(gaussianSampler(7, 11), 1, 15)->sample(context, context.getRandomGenerator());

    jassertfalse;
    return Variable();
  }

  void restrictToGlobalParameteres(std::vector<bool>& notUsedParameters) const
  {
    for (int i = largeProteinParametersClass->findMemberVariable(T("useRelativePosition"));
         i < (int)largeProteinParametersClass->getNumMemberVariables();
         ++i)
    notUsedParameters[i] = false;
  }
  
  void restrictToResidueParameteres(std::vector<bool>& notUsedParameters) const
  {
    for (int i = largeProteinParametersClass->findMemberVariable(T("usePositionDifference"));
         i < (int)largeProteinParametersClass->getNumMemberVariables();
         ++i)
      notUsedParameters[i] = false;
  }

  void restrictToResiduePairParameteres(std::vector<bool>& notUsedParameters) const
  {
    for (int i = largeProteinParametersClass->findMemberVariable(T("cbsAbsoluteSize"));
         i < (int)largeProteinParametersClass->getNumMemberVariables();
         ++i)
      notUsedParameters[i] = false;
  }
};

class PlotDisulfideBondResultsWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    juce::OwnedArray<File> files;
    directory.findChildFiles(files, File::findFiles, false, T("Param-*_Exp-*.SVM_G-*_C-*.trace"));
    parseSVMFiles(context, files);
    
    files.clear();
    directory.findChildFiles(files, File::findFiles, false, T("Param-*_Exp-*.kNN.trace"));
    parseFiles(context, files, T("kNN"));

    files.clear();
    directory.findChildFiles(files, File::findFiles, false, T("Param-*_Exp-*.Dumb.trace"));
    parseFiles(context, files, T("Dumb"));

    files.clear();
    directory.findChildFiles(files, File::findFiles, false, T("Param-*_Exp-*.x3.trace"));
    parseFiles(context, files, T("x3"));

    return true;
  }

protected:
  friend class PlotDisulfideBondResultsWorkUnitClass;

  File directory;

  size_t parseNumParameters(ExecutionContext& context, const String& str) const
  {
    static const size_t prefixLength = String(T("Param-")).length();
    const int end = str.indexOf(T("_Exp-"));
    jassert(end != -1);
    return Variable::createFromString(context, positiveIntegerType, str.substring(prefixLength, end)).getInteger();
  }

  size_t parseExperimentId(ExecutionContext& context, const String& str) const
  {
    static const size_t prefixLength = String(T("_Exp-")).length();
    const int start = str.indexOf(T("_Exp-")) + prefixLength;
    const int end = str.indexOfChar(start, T('.'));
    jassert(start != -1 && end != -1);
    return Variable::createFromString(context, positiveIntegerType, str.substring(start, end)).getInteger();
  }

  double getReturnValueOfTraceFile(ExecutionContext& context, File f) const
  {
    ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, f).staticCast<ExecutionTrace>();
    jassert(trace);
    const Variable res = trace->getRootNode()->findFirstNode()->getReturnValue();
    if (res.isConvertibleToDouble())
      return res.toDouble();
    else      
    {
      context.warningCallback(T("Error in Trace: ") + f.getFileName());
      return DBL_MAX;
    }
  }

  void parseFiles(ExecutionContext& context, const juce::OwnedArray<File>& files, const String& name) const
  {
    if (files.size() == 0)
      return;
    // Read files
    typedef std::map<size_t, std::vector<std::pair<size_t, double> > > ScoresMap;
    ScoresMap results;
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      const String fileName = files[i]->getFileName();
      const size_t numParameters = parseNumParameters(context, fileName);      
      const size_t experimentId = parseExperimentId(context, fileName);
      const double result = getReturnValueOfTraceFile(context, *files[i]);
      if (result == DBL_MAX)
        continue;

      results[numParameters].push_back(std::make_pair(experimentId, result));
    }

    // Print results
    File outputFile = context.getFile(T("result.") + name + (".plot"));
    outputFile.deleteFile();
    OutputStream* o = outputFile.createOutputStream();
    for (ScoresMap::iterator it = results.begin(); it != results.end(); ++it)
    {
      context.enterScope(T("Num. Parameters: ") + String((int)it->first));
      double sum = 0;
      std::vector<std::pair<size_t, double> > subResults = it->second;
      context.resultCallback(T("Num. Parameters"), it->first);
      context.resultCallback(T("Num. Experiments"), subResults.size());
      for (size_t i = 0; i < subResults.size(); ++i)
      {
        context.enterScope(T("Experiment: ") + String((int)subResults[i].first));
        context.resultCallback(T("Experiment"), subResults[i].first);
        context.resultCallback(T("Result"), subResults[i].second);
        sum += subResults[i].second;
        context.leaveScope(subResults[i].second);
        *o << (int)it->first << "\t" << (int)subResults[i].first << "\t" << subResults[i].second << "\n";
      }
      context.leaveScope(sum / ((double)subResults.size()));
    }
    delete o;
  }

  double parseGamma(ExecutionContext& context, const String& str) const
  {
    static const size_t prefixLength = String(T(".SVM_G-")).length();
    const int start = str.indexOf(T(".SVM_G-")) + prefixLength;
    const int end = str.indexOfChar(start, T('_'));
    jassert(start != -1 && end != -1);
    return Variable::createFromString(context, doubleType, str.substring(start, end)).getDouble();
  }

  double parseRegularizer(ExecutionContext& context, const String& str) const
  {
    static const size_t prefixLength = String(T("_C-")).length();
    const int start = str.indexOf(T("_C-")) + prefixLength;
    const int end = str.indexOfChar(start, T('.'));
    return Variable::createFromString(context, doubleType, str.substring(start, end)).getDouble();
  }

  void parseSVMFiles(ExecutionContext& context, const juce::OwnedArray<File>& files) const
  {
    if (files.size() == 0)
      return;
    // Param -> Exp -> Gamma -> Reg -> Result
    typedef std::map<size_t, std::map<size_t, std::map<double, std::map<double, double> > > > ScoresMap;
    ScoresMap results;
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      const String fileName = files[i]->getFileName();
      const size_t numParameters = parseNumParameters(context, fileName);      
      const size_t experimentId = parseExperimentId(context, fileName);
      const double gamma = parseGamma(context, fileName);
      const double regularizer = parseRegularizer(context, fileName);
      const double result = getReturnValueOfTraceFile(context, *files[i]);
      if (result == DBL_MAX)
        continue;

      results[numParameters][experimentId][gamma][regularizer] = result;
    }

    // Print results
    OutputStream* o = context.getFile(T("result.SVM.plot")).createOutputStream();

    typedef std::map<size_t, std::map<double, std::map<double, double> > > SubScoresMap;
    typedef std::map<double, std::map<double, double> > GammaScoresMap;
    typedef std::map<double, double> RegularizerScoresMap;
    for (ScoresMap::iterator it = results.begin(); it != results.end(); ++it)
    {
      for (SubScoresMap::iterator sit = it->second.begin(); sit != it->second.end(); ++sit)
      {
        context.enterScope(T("Param: ") + String((int)it->first) + T(" & Exp: ") + String((int)sit->first));
        double bestGamma = DBL_MAX;
        double bestGammaScore = DBL_MAX;
        for (GammaScoresMap::iterator git = sit->second.begin(); git != sit->second.end(); ++git)
        {
          context.enterScope(T("Gamma: ") + String(git->first));
          context.resultCallback(T("Gamma"), git->first);
          double bestReg = DBL_MAX;
          double bestRegScore = DBL_MAX;
          for (RegularizerScoresMap::iterator rit = git->second.begin(); rit != git->second.end(); ++rit)
          {
            context.enterScope(T("Regularizer: ") + String(rit->first));
            context.resultCallback(T("Regularizer"), rit->first);
            context.resultCallback(T("Result"), rit->second);
            if (rit->second < bestRegScore)
            {
              bestReg = rit->first;
              bestRegScore = rit->second;
            }
            context.leaveScope(rit->first);
          }
          context.resultCallback(T("Best Regularizer Score"), bestRegScore);
          context.resultCallback(T("Best Regularizer"), bestReg);
          if (bestRegScore < bestGammaScore)
          {
            bestGammaScore = bestRegScore;
            bestGamma = git->first;
          }
          context.leaveScope(bestRegScore);
        }
        context.resultCallback(T("Best Gamma"), bestGamma);
        context.resultCallback(T("Best Gamma Score"), bestGammaScore);
        context.leaveScope(bestGammaScore);
        *o << (int)it->first << "\t" << (int)sit->first << "\t" << bestGammaScore << "\n";
      }
    }
    delete o;
  }
};

class ProteinLearnerForComparativeStudy : public WorkUnit
{
public:
  ProteinLearnerForComparativeStudy()
    : numFolds(10) {}

  virtual Variable run(ExecutionContext& context)
  {
    LargeProteinParametersPtr parameters = LargeProteinParameters::createFromFile(context, parametersFile).dynamicCast<LargeProteinParameters>();
    if (!parameters)
    {
      context.errorCallback(T("Trouble with the parameters file !"));
      return false;
    }

    if (inputDirectory.getChildFile(T("train/")).exists() && supervisionDirectory.getChildFile(T("test/")).exists()
        && supervisionDirectory.getChildFile(T("train/")).exists() && supervisionDirectory.getChildFile(T("test/")).exists())
    {
      context.informationCallback(T("Train/Test split detected."));
      ContainerPtr trainingProteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory.getChildFile(T("train/")), supervisionDirectory.getChildFile(T("train/")), 0, T("Loading training proteins"));
      ContainerPtr testingProteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory.getChildFile(T("test/")), supervisionDirectory.getChildFile(T("test/")), 0, T("Loading testing proteins"));

      if (!trainingProteins && !testingProteins)
      {
        context.errorCallback(T("Touble with train/test proteins !"));
        return DBL_MAX;
      }

      return computeFold(context, parameters, trainingProteins, testingProteins);
    }
    
    context.informationCallback(T("No train/test split detected ! Application of Cross-Validation."));
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, 0, T("Loading proteins"));
    if (!proteins)
    {
      context.errorCallback(T("Trouble with proteins !"));
      return DBL_MAX;
    }
    
    ScalarVariableMeanAndVariancePtr res = new ScalarVariableMeanAndVariance();

    for (size_t i = 0; i < numFolds; ++i)
      res->push(computeFold(context, parameters, proteins->invFold(i, numFolds), proteins->fold(i, numFolds)));

    return res->getMean();
  }

  double computeFold(ExecutionContext& context, const LargeProteinParametersPtr& parameters, const ContainerPtr& train, const ContainerPtr& test) const
  {
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameters);
    // Extra-Trees - Default settings
    if (learningMachineName.startsWith(T("ExtraTrees")))
    {
      size_t x3Trees = 1000;
      size_t x3Attributes = 0;
      size_t x3Splits = 1;      

      StringArray tokens;
      tokens.addTokens(learningMachineName, T(":"), NULL);
      if (tokens.size() > 1)
      {
        x3Trees = tokens[1].getIntValue();
        x3Attributes = tokens[2].getIntValue();
        x3Splits = tokens[3].getIntValue();
      }
      bool x3LowMemory = true;
      predictor->learner = extraTreeLearningMachine(x3Trees, x3Attributes, x3Splits, false, x3LowMemory);
    } 
    // k-Nearest Neighbors - Default setting
    if (learningMachineName == T("kNN"))
    {
      size_t knnNeighbors = 5;
      predictor->learner = new PreProcessInputCompositeFunction(composeFunction(
                               doubleVectorNormalizeFunction(true, true), concatenatedDoubleVectorNormalizeFunction()), 
                               nearestNeighborLearningMachine(knnNeighbors, true));
    }
    // Support vector machines - Settings
    if (learningMachineName.startsWith(T("SVM")))
    {
      StringArray tokens;
      tokens.addTokens(learningMachineName, T(":"), NULL);
      int svmC = tokens[1].getIntValue();
      int svmGamma = tokens[2].getIntValue();
      predictor->learner = new PreProcessInputCompositeFunction(
                               doubleVectorNormalizeFunction(true, true),
                               libSVMLearningMachine(pow(2.0, svmC), rbfKernel, 0, pow(2.0, svmGamma), 0.0));
    }

    if (!predictor->learner)
    {
      context.errorCallback(T("No learner !"));
      return DBL_MAX;
    }
    
    ProteinPredictorPtr iteration = new ProteinPredictor(predictor);
    iteration->addTarget(target);
    
    if (!iteration->train(context, train, test, T("Training")))
      return DBL_MAX;

    ProteinEvaluatorPtr evaluator = createProteinEvaluator();
    CompositeScoreObjectPtr scores = iteration->evaluate(context, test, evaluator, T("EvaluateTest"));
    return evaluator->getScoreToMinimize(scores);
  }

protected:
  friend class ProteinLearnerForComparativeStudyClass;

  File inputDirectory;
  File supervisionDirectory;
  File parametersFile;
  
  String learningMachineName;
  size_t numFolds;
  
  ProteinTarget target;

  ProteinEvaluatorPtr createProteinEvaluator() const
  {    
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();

    if (target == ss3Target || target == ss8Target || target == stalTarget)
      evaluator->addEvaluator(target, containerSupervisedEvaluator(classificationEvaluator()), T("SS3-SS8-StAl"), true);
    else if (target == sa20Target || target == cbsTarget)
      evaluator->addEvaluator(target, containerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore)), T("SA20"), true);
    else if (target == drTarget)
      evaluator->addEvaluator(target, containerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationMCCScore)), T("DR"), true);
    else if (target == dsbTarget)
      evaluator->addEvaluator(target, new DisulfidePatternEvaluator(new KolmogorovPerfectMatchingFunction(0.f), 0.f), T("DSB QP Perfect"), true);
    else if (target == cbpTarget)
      evaluator->addEvaluator(target, binaryClassificationEvaluator(binaryClassificationAccuracyScore), T("CBP"), true);

    return evaluator;
  }
};

};
