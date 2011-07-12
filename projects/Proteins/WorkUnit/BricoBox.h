#include "Data/Protein.h"
#include "../Evaluator/ProteinEvaluator.h"

#include "Data/Formats/SPXFileParser.h"
#include <lbcpp/Data/Stream.h>

#ifdef LBCPP_NETWORKING
# include <lbcpp/Network/NetworkInterface.h>
# include <lbcpp/Network/NetworkNotification.h>
#endif

#include <lbcpp/UserInterface/PieChartComponent.h>
#include <lbcpp/UserInterface/HistogramComponent.h>

#include <lbcpp/UserInterface/VariableSelector.h>

#include "../Predictor/NumericalCysteinPredictorParameters.h"
#include "../Predictor/Lin09PredictorParameters.h"

/*
** BricoBox - Some non-important test tools
*/

namespace lbcpp
{

class CheckDisulfideBondsWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectory(context, proteinDirectory);
    
    size_t numBridges = 0;
    for (size_t i = 0; i < proteins->getNumElements(); ++i)
      numBridges += getNumBridges(context, proteins->getElement(i).getObjectAndCast<Protein>());
    std::cout << numBridges << std::endl;
    return Variable();
  }
  
protected:
  friend class CheckDisulfideBondsWorkUnitClass;

  File proteinDirectory;

  static size_t getNumBridges(ExecutionContext& context, ProteinPtr protein);
  static bool checkConsistencyOfBridges(SymmetricMatrixPtr bridges);
};

class CheckARFFDataFileParserWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (!dataFile.exists())
    {
      context.errorCallback(T("CheckARFFDataFileParserWorkUnit::run"), T("Invalid input data file: ") + dataFile.getFullPathName());
      return Variable();
    }
    
    juce::OwnedArray<File> files;
    if (dataFile.isDirectory())
      dataFile.findChildFiles(files, 2, false, T("*.arff"));
    else
      files.add(new File(dataFile));

    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      VectorPtr data = classificationARFFDataParser(context, *files[i], new DynamicClass(T("features")), new DefaultEnumeration(T("output")))->load();
      context.resultCallback(files[i]->getFileName(), data);
    }

    return Variable();
  }

protected:
  friend class CheckARFFDataFileParserWorkUnitClass;
  
  File dataFile;
};

class SaveObjectProgram : public WorkUnit
{
  virtual String toString() const
    {return T("SaveObjectProgram is able to serialize a object.");}
  
  virtual Variable run(ExecutionContext& context)
  {
    if (className == String::empty)
    {
      context.warningCallback(T("SaveObjectProgram::run"), T("No class name specified"));
      return false;
    }

    if (outputFile == File::nonexistent)
      outputFile = File::getCurrentWorkingDirectory().getChildFile(className + T(".xml"));
    
    std::cout << "Loading class " << className.quoted() << " ... ";
    std::flush(std::cout);

    TypePtr type = typeManager().getType(context, className);
    if (!type)
    {
      std::cout << "Fail" << std::endl;
      return false;
    }

    ObjectPtr obj = Object::create(type);
    if (!obj)
    {
      std::cout << "Fail" << std::endl;
      return false;
    }

    std::cout << "OK" << std::endl;
    std::cout << "Saving class to " << outputFile.getFileName().quoted() << " ... ";
    std::flush(std::cout);

    obj->saveToFile(context, outputFile);

    std::cout << "OK" << std::endl;
    return true;
  }

protected:
  friend class SaveObjectProgramClass;
  
  String className;
  File outputFile;
};

class CheckXmlElementWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    WorkUnitPtr wu = new SaveObjectProgram();
    
    aaa = new XmlElement();
    aaa->saveObject(context, wu, T("workUnit"));
    
    saveToFile(context, context.getFile(T("testSerialisation")));

    ObjectPtr o = createFromFile(context, context.getFile(T("testSerialisation")));
    o->saveToFile(context, context.getFile(T("testReSerialisation")));
    
    
    ObjectPtr obj = aaa->createObject(context);
    obj.dynamicCast<WorkUnit>()->run(context);

    return true;
  }
  
  XmlElementPtr aaa;
};

class Save1DProteinTargets : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (!inputProtein.exists() || outputProtein == File::nonexistent)
      return false;

    ProteinPtr protein = Protein::createFromXml(context, inputProtein);
    if (!protein)
      return false;

    protein->getDisorderRegions();
    protein->getStructuralAlphabetSequence();
    protein->setTertiaryStructure(TertiaryStructurePtr());
    protein->saveToFile(context, outputProtein);

    return true;
  }
  
protected:
  friend class Save1DProteinTargetsClass;
  
  File inputProtein;
  File outputProtein;
};

class LoadModelAndEvaluate : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    FunctionPtr predictor = Function::createFromFile(context, modelFile).dynamicCast<Function>();
    if (!predictor)
    {
      context.errorCallback(T("LoadModelAndEvaluate::run"), T("No predictor loaded !"));
      return false;
    }
    
    ContainerPtr testProteins = Protein::loadProteinsFromDirectoryPair(context, File::nonexistent, proteinDirectory.getChildFile(T("test")), 0, T("Loading test proteins"));
    if (!testProteins || !testProteins->getNumElements())
    {
      context.errorCallback(T("LoadModelAndEvaluate::run"), T("No test proteins"));
      return false;
    }
    
    EvaluatorPtr evaluator = new ProteinEvaluator();
    ScoreObjectPtr scores = predictor->evaluate(context, testProteins, evaluator, T("Evaluate on test proteins"));

    return scores;
  }

protected:
  friend class LoadModelAndEvaluateClass;
  
  File modelFile;
  File proteinDirectory;
};

class DebugExecutionTraceLoading : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (traceFile == File::nonexistent)
    {
      context.errorCallback(T("DebugExecutionTraceLoading::run"), T("Trace not found !"));
      return false;
    }

    ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, traceFile).dynamicCast<ExecutionTrace>();
    if (!trace)
    {
      context.errorCallback(T("DebugExecutionTraceLoading::run"), T("Trace not exists !"));
      return false;
    }
    
    context.informationCallback(trace->toString());
    
    context.informationCallback(T("Saving trace ..."));
    File f(traceFile.getFullPathName() + T(".saved"));
    trace->saveToFile(context, f);
    
    context.informationCallback(T("Loading saved trace ..."));
    trace = ExecutionTrace::createFromFile(context, f).dynamicCast<ExecutionTrace>();
    if (!trace)
    {
      context.errorCallback(T("DebugExecutionTraceLoading::run"), T("Saved trace not exists !"));
      return false;
    }

    return true;
  }
  
protected:
  friend class DebugExecutionTraceLoadingClass;
  
  File traceFile;
};

class CheckNetworkServerWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_NETWORKING
    NetworkServerPtr server = new NetworkServer(context);
    if (!server->startServer(1664))
    {
      context.errorCallback(T("WorkUnitManagerServer::run"), T("Not able to open port"));
      return false;
    }
    context.informationCallback(T("Server started !"));
    
    while (true)
    {
      /* Accept client */
      NetworkClientPtr client = server->acceptClient(INT_MAX);
      context.informationCallback(client->getConnectedHostName(), T("Connected"));
      
      String ping;
      if (!client->receiveString(5000, ping))
      {
        context.informationCallback(client->getConnectedHostName(), T("Error - Ping"));
        continue;
      }

      std::cout << ping << std::endl;
      client->sendVariable(T("PONG"));
    }
#endif
	return Variable();
  }
};

class CheckNetworkClientWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_NETWORKING
    for (size_t i = 0; i < 50; ++i)
    {
      NetworkClientPtr client = blockingNetworkClient(context, 3);
      if (!client->startClient(T("localhost"), 1664))
      {
        context.errorCallback(T("WorkUnitManagerServer::run"), T("Not connected !"));
        return false;
      }
      client->sendVariable(T("PING"));
      
      String pong;
      if (!client->receiveString(5000, pong))
      {
        context.informationCallback(client->getConnectedHostName(), T("Error - Pong"));
        continue;
      }

      std::cout << i << ": " << pong << std::endl;
      
      client->stopClient();
    }
    return true;
#endif
	return false;
  }
};

class ConvertSPXFileToProteins : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (!spxFile.exists())
      return false;

    ConsumerPtr consumer = saveToFileConsumer(outputDirectory);
    consumer->consumeStream(context, new SPXFileParser(context, spxFile));
    return true;
  }

protected:
  friend class ConvertSPXFileToProteinsClass;

  File spxFile;
  File outputDirectory;
};

class CheckManagerWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_NETWORKING
    NetworkClientPtr client = blockingNetworkClient(context);
    if (!client->startClient(T("localhost"), 1664))
    {
      context.errorCallback(T("CheckManagerWorkUnit::run"), T("Not connected !"));
      return false;
    }
    context.informationCallback(T("localhost"), T("Connected !"));
    
    ManagerNetworkInterfacePtr interface = forwarderManagerNetworkInterface(context, client, T("Alpha"));
    client->sendVariable(ReferenceCountedObjectPtr<NetworkInterface>(interface));
    
    WorkUnitNetworkRequestPtr request = new WorkUnitNetworkRequest(context, T("CheckManagerWorkUnit"), T("Alpha"), T("Omega"), WorkUnitPtr(), 1, 1, 1);
    for (size_t i = 0; i < 1500; ++i)
      context.informationCallback(String((int)i) + T(" : ") + interface->pushWorkUnit(request));
    client->sendVariable(new CloseCommunicationNotification());
    client->stopClient();
    
    return true;
#endif
    return false;
  }
};

class GenerateFoldsWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    const String prefix(T("fold_"));

    if (sourceDirectory == File::nonexistent)
      sourceDirectory = File::getCurrentWorkingDirectory();
    
    if (destinationDirectory == File::nonexistent)
      destinationDirectory = File::getCurrentWorkingDirectory();
    
    /* create train and test directories */
    for (size_t i = 0; i < numFolds; ++i)
    {
      const String dirName(prefix + String((int)i));
      destinationDirectory.getChildFile(dirName).createDirectory();
      destinationDirectory.getChildFile(dirName + T("/test")).createDirectory();
      destinationDirectory.getChildFile(dirName + T("/train")).createDirectory();
      destinationDirectory.getChildFile(dirName + T("/validation")).createDirectory();
    }

    /* List source folder */
    juce::OwnedArray< File > results;
    sourceDirectory.findChildFiles(results, File::findFiles, false, T("*.xml"));
    const size_t n = results.size();

    /* Build fold */
    size_t currentFold = 0;
    for (size_t i = 0; i < n; ++i)
    {
      const File* toCopy = results[i];
      // testing fold
      toCopy->copyFileTo(destinationDirectory.getChildFile(prefix + String((int)currentFold) + T("/test/") + toCopy->getFileName()));
      // training fold
      for (size_t j = 0; j < n; ++j)
      {
        if (j == currentFold)
          continue;
        toCopy->copyFileTo(destinationDirectory.getChildFile(prefix + String((int)j) + T("/train/") + toCopy->getFileName()));
      }

      ++currentFold;
      currentFold %= numFolds;
    }
    
    return true;
  }

protected:
  friend class GenerateFoldsWorkUnitClass;

  File sourceDirectory;
  File destinationDirectory;
  size_t numFolds;
};

class ProteinMaxLength : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputDirectory == File::nonexistent)
      inputDirectory = File::getCurrentWorkingDirectory();

    ContainerPtr trainingProteins = Protein::loadProteinsFromDirectory(context, File(inputDirectory.getFullPathName() + T("/train/")), 0, T("Loading training proteins"));
    ContainerPtr validationProteins = Protein::loadProteinsFromDirectory(context, File(inputDirectory.getFullPathName() + T("/validation/")), 0, T("Loading validation proteins"));
    ContainerPtr testingProteins = Protein::loadProteinsFromDirectory(context, File(inputDirectory.getFullPathName() + T("/test/")), 0, T("Loading testing proteins"));

    size_t trainingLength = getMaxLength(context, trainingProteins);
    context.resultCallback(T("trainingLength"), trainingLength);
    std::cout << "Training length: " << trainingLength << std::endl;

    size_t validationLength = getMaxLength(context, validationProteins);
    context.resultCallback(T("validationLength"), validationLength);
    std::cout << "Validation length: " << validationLength << std::endl;

    size_t testingLength = getMaxLength(context, testingProteins);
    context.resultCallback(T("testingLength"), testingLength);
    std::cout << "Test length: " << testingLength << std::endl;

    return juce::jmax((int)trainingLength, (int)validationLength, (int)testingLength);
  }
  
protected:
  friend class ProteinMaxLengthClass;
  
  File inputDirectory;
  
  size_t getMaxLength(ExecutionContext& context, const ContainerPtr& proteins) const
  {
    size_t maxLength = 0;
    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>(context);
      jassert(protein);
      if (protein->getLength() > maxLength)
        maxLength = protein->getLength();
    }
    return maxLength;
  }
};

class CheckGreedyDisulfideBondWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputFile == File::nonexistent || !inputFile.exists())
      return false;

    ProteinPtr protein = Protein::createFromXml(context, inputFile);
    jassert(protein);

    SymmetricMatrixPtr matrix = protein->getDisulfideBonds(context);
    jassert(matrix);

    FunctionPtr function = new GreedyDisulfidePatternBuilder(3);
    SymmetricMatrixPtr res = function->compute(context, matrix).getObjectAndCast<SymmetricMatrix>();
    jassert(res);

    return res;
  }

protected:
  friend class CheckGreedyDisulfideBondWorkUnitClass;

  File inputFile;
};

class EDAResultFileWriter : public Object
{
public:
  EDAResultFileWriter(const File& outputFile, const File& logFile)
    : outputFile(outputFile), logFile(logFile) {}
  EDAResultFileWriter() {}

  void write(String features, double trainScore, double testScore) const
  {    
    if (!outputFile.exists())
      createFile(numericalCysteinFeaturesParametersClass);

    OutputStream *o = outputFile.createOutputStream();
    *o << trainScore << ";" << testScore;

    StringArray tokens;
    tokens.addTokens(features.substring(1, features.length() - 1), T(","), NULL);
    for (size_t i = 0; i < (size_t)tokens.size(); ++i)
      *o << ";" << tokens[i];
    *o << "\n";

    delete o;
  }
  
  void writeLog(String features) const
  {
    OutputStream *o = logFile.createOutputStream();
    *o << features << "\"\n";
    delete o;
  }

  void createFile(TypePtr type) const
  {
    OutputStream* o = outputFile.createOutputStream();
    *o << "Train Score; Test Score";
    for (size_t i = 0; i < type->getNumMemberVariables(); ++i)
      *o << ";" << type->getMemberVariableName(i);
    *o << "\n";
    delete o;
  }

protected:
  friend class EDAResultFileWriterClass;

  File outputFile;
  File logFile;
};

typedef ReferenceCountedObjectPtr<EDAResultFileWriter> EDAResultFileWriterPtr;

class CParameterOptimizer : public Optimizer
{
public:
  CParameterOptimizer(Lin09ParametersPtr parameters)
    : parameters(parameters) {}
  CParameterOptimizer() {}

  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  {
    std::vector<double> values;
    for (size_t i = 0; i < 5; ++i)
      values.push_back(5.4 + (double)i);
    
    for (size_t i = 0; i < values.size(); ++i)
    {
      Lin09PredictorParametersPtr candidate = new Lin09PredictorParameters(parameters);
      candidate->useLibSVM = true;
      candidate->C = values[i];
      
      size_t numAttempt = 0;
      while (!optimizerContext->evaluate(candidate))
      {
        context.informationCallback(T("Evaluation - Attempt ") + String((int)numAttempt));
        Thread::sleep(optimizerContext->getTimeToSleep());
      }

      optimizerState->incTotalNumberOfRequests();
    }
    
    while (!optimizerContext->areAllRequestsProcessed())
      Thread::sleep(optimizerContext->getTimeToSleep());
    
    jassert(optimizerState->getNumberOfProcessedRequests() == values.size());
    double bestScore = DBL_MAX;
    const std::vector<std::pair<double, Variable> >& results = optimizerState->getProcessedRequests();
    for (size_t i = 0; i < results.size(); ++i)
      if (results[i].first < bestScore)
        bestScore = results[i].first;

    if (bestScore == DBL_MAX)
      context.errorCallback(T("CParameterOptimizer"), T("Jobs Failed ! Parameter: ") + parameters->toString());

    optimizerState->setBestScore(bestScore);
    return bestScore;
  }

protected:
  friend class CParameterOptimizerClass;
  Lin09ParametersPtr parameters;
};

class CysteinLearnerFunction : public SimpleUnaryFunction
{
public:
  CysteinLearnerFunction(String inputDirectory = String::empty)
    : SimpleUnaryFunction(lin09PredictorParametersClass, doubleType, T("CysteinLearner"))
    , inputDirectory(inputDirectory) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
//    ExecutionContextPtr subContext = multiThreadedExecutionContext(8, ctx.getProjectDirectory());
//    ExecutionContext& context = *subContext;

    ContainerPtr trainingData = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(inputDirectory).getChildFile(T("train/")), 0, T("Loading training proteins"));
    if (!trainingData || !trainingData->getNumElements())
    {
      context.errorCallback(T("No training proteins !"));
      return 101;
    }

    Lin09PredictorParametersPtr parameters = input.getObjectAndCast<Lin09PredictorParameters>(context);

    size_t numStacks = 1;
    ProteinSequentialPredictorPtr predictor = new ProteinSequentialPredictor();
    for (size_t i = 0; i < numStacks; ++i)
    {
      ProteinPredictorPtr stack = new ProteinPredictor(parameters);
      stack->addTarget(dsbTarget);
      predictor->addPredictor(stack);
    }

    if (!predictor->train(context, trainingData, ContainerPtr(), T("Training")))
      return 102;

    ProteinEvaluatorPtr trainEvaluator = new ProteinEvaluator();
    CompositeScoreObjectPtr trainScores = predictor->evaluate(context, trainingData, trainEvaluator, T("Evaluate on training proteins"));

    ContainerPtr testingData = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(inputDirectory).getChildFile(T("test/")), 0, T("Loading testing proteins"));
    if (!testingData || !testingData->getNumElements())
    {
      context.warningCallback(T("No testing proteins ! Training score is returned !"));
      return trainEvaluator->getScoreObjectOfTarget(trainScores, dsbTarget)->getScoreToMinimize();
    }
    
    ProteinEvaluatorPtr testEvaluator = new ProteinEvaluator();
    CompositeScoreObjectPtr testScores = predictor->evaluate(context, testingData, testEvaluator, T("Evaluate on test proteins"));

    return testEvaluator->getScoreObjectOfTarget(testScores, dsbTarget)->getScoreToMinimize();
  }

protected:
  friend class CysteinLearnerFunctionClass;

  String inputDirectory;
};

class COptimizerFunction : public SimpleUnaryFunction
{
public:
  COptimizerFunction(String inputDirectory = String::empty)
    : SimpleUnaryFunction(lin09ParametersClass, doubleType, T("COptimizer")), inputDirectory(inputDirectory) {}

  virtual Variable computeFunction(ExecutionContext& ctx, const Variable& input) const
  {
    ExecutionContextPtr subContext = singleThreadedExecutionContext(ctx.getProjectDirectory());
    ExecutionContext& context = *subContext;
    context.appendCallback(consoleExecutionCallback());

    Lin09ParametersPtr parameters = input.getObjectAndCast<Lin09Parameters>(context);

    std::vector<String> destination;
    destination.push_back(T("jbecker@nic3"));
    destination.push_back(T("fmaes@nic3"));

    FunctionPtr f = new CysteinLearnerFunction(inputDirectory);
    OptimizerPtr optimizer = new CParameterOptimizer(parameters);
    OptimizerContextPtr optimizerContext = distributedOptimizerContext(context, f, T("bfsCysBondsLibSVM"), T("jbecker@monster24"), destination, T("localhost"), 1664, 8, 5, 48, 6000);
    OptimizerStatePtr optimizerState = new OptimizerState();

    return optimizer->compute(context, optimizerContext, optimizerState);
  }

protected:
  friend class COptimizerFunctionClass;

  String inputDirectory;
};

class BFSCysteinProteinLearner : public WorkUnit
{
public:  
  Variable run(ExecutionContext& context)
  {
    FunctionPtr f = new COptimizerFunction(inputDirectory);

    OptimizerPtr optimizer = bestFirstSearchOptimizer();
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, f, FunctionPtr(), 30000);
    OptimizerStatePtr optimizerState = streamBasedOptimizerState(context, Lin09Parameters::createInitialObject(), Lin09Parameters::createStreams());

    return optimizer->compute(context, optimizerContext, optimizerState);
  }

protected:
  friend class BFSCysteinProteinLearnerClass;

  String inputDirectory;
};

class BFSTestParameter : public Object
{
public:
  double a;
  double b;
  double c;
  
  BFSTestParameter() : a(10.f), b(10.f), c(10.f) {}

protected:
  friend class BFSTestParameterClass;
};

extern ClassPtr bfsTestParameterClass;

class BFSTestObjectiveFunction : public SimpleUnaryFunction
{
public:
  BFSTestObjectiveFunction()
    : SimpleUnaryFunction(bfsTestParameterClass, doubleType, T("BFSTest")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ReferenceCountedObjectPtr<BFSTestParameter> param = input.getObjectAndCast<BFSTestParameter>(context);
    return Variable(fabs(param->a * param->a + param->b * param->c - 10), doubleType);
  }
};

class BFSTestWorkUnit : public WorkUnit
{
public:
  Variable run(ExecutionContext& context)
  {
    std::vector<double> values(5);
    for (size_t i = 0; i < 5; ++i)
      values[i] = (double)i - 2;
    StreamPtr dStream = doubleStream(doubleType, values);
    
    std::vector<StreamPtr> streams(3);
    for (size_t i = 0; i < 3; ++i)
      streams[i] = dStream;
    
    OptimizerPtr optimizer = bestFirstSearchOptimizer();
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, new BFSTestObjectiveFunction(), FunctionPtr(), 3000);
    OptimizerStatePtr optimizerState = streamBasedOptimizerState(context, new BFSTestParameter(), streams);
    
    return optimizer->compute(context, optimizerContext, optimizerState);
  }
};

};
