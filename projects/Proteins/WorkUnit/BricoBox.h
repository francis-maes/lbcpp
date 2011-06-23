#include <lbcpp/lbcpp.h>
#include "Data/Protein.h"
#include "../Evaluator/ProteinEvaluator.h"

#include "Data/Formats/SPXFileParser.h"

#ifdef LBCPP_NETWORKING
# include <lbcpp/Network/NetworkInterface.h>
# include <lbcpp/Network/NetworkNotification.h>
#endif

#include <lbcpp/UserInterface/PieChartComponent.h>
#include <lbcpp/UserInterface/HistogramComponent.h>

#include <lbcpp/UserInterface/VariableSelector.h>

#include "../Predictor/NumericalCysteinPredictorParameters.h"

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

class ProteinStatisticsWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputDirectory == File::nonexistent)
      inputDirectory = File::getCurrentWorkingDirectory();

    ContainerPtr trainingProteins = Protein::loadProteinsFromDirectory(context, File(inputDirectory.getFullPathName() + T("/train/")), 0, T("Loading training proteins"));
    ContainerPtr validationProteins = Protein::loadProteinsFromDirectory(context, File(inputDirectory.getFullPathName() + T("/validation/")), 0, T("Loading validation proteins"));
    ContainerPtr testingProteins = Protein::loadProteinsFromDirectory(context, File(inputDirectory.getFullPathName() + T("/test/")), 0, T("Loading testing proteins"));

    if (!trainingProteins->getNumElements() && !validationProteins->getNumElements() && !testingProteins->getNumElements())
    {
      context.informationCallback(T("No proteins"));
      return false;
    }

    VectorPtr allProteins = vector(proteinClass, trainingProteins->getNumElements() + validationProteins->getNumElements() + testingProteins->getNumElements());
    size_t index = 0;
    for (size_t i = 0; i < trainingProteins->getNumElements(); ++i, ++index)
      allProteins->setElement(index, trainingProteins->getElement(i));
    for (size_t i = 0; i < validationProteins->getNumElements(); ++i, ++index)
      allProteins->setElement(index, validationProteins->getElement(i));
    for (size_t i = 0; i < testingProteins->getNumElements(); ++i, ++index)
      allProteins->setElement(index, testingProteins->getElement(i));

    computeProteinStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, ss3Target);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, ss8Target);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, sa20Target);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, drTarget);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, stalTarget);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, cbpTarget);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, cbsTarget);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, dsbTarget);
    //computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, cma8Target);
    computeDisulfideStatistics(context, allProteins);
    
    return true;
  }

protected:
  friend class ProteinStatisticsWorkUnitClass;

  File inputDirectory;

protected:
  void computeProteinStatistics(ExecutionContext& context, ContainerPtr trainingProteins, ContainerPtr validationProteins, ContainerPtr testingProteins, ContainerPtr allProteins) const
  {
    context.enterScope(T("Computing gobal statistics"), WorkUnitPtr());
    
    HistogramConfigurationPtr res;
    res = computeLengthStatistics(context, trainingProteins, T("Training"));
    context.resultCallback(T("Train"), res);
    
    res = computeLengthStatistics(context, validationProteins, T("Validation"));
    context.resultCallback(T("Validation"), res);
    
    res = computeLengthStatistics(context, testingProteins, T("Testing"));
    context.resultCallback(T("Test"), res);
    
    res = computeLengthStatistics(context, allProteins, T("Global"));
    context.resultCallback(T("All"), res);

    context.leaveScope();
  }
  
  HistogramConfigurationPtr computeLengthStatistics(ExecutionContext& context, const ContainerPtr& proteins, const String& name) const
  {
    const size_t n = proteins->getNumElements();
    HistogramConfigurationPtr config = new HistogramConfiguration(36, 0, 1400, true, name);
    for (size_t i = 0; i < n; i++)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>(context);
      jassert(protein);
      config->addData(protein->getLength());
    }
    return config;
  }
  
  void computeStatistics(ExecutionContext& context, ContainerPtr trainingProteins, ContainerPtr validationProteins, ContainerPtr testingProteins, ContainerPtr allProteins, ProteinTarget target) const
  {
    context.enterScope(T("Computing statistics of ") + proteinClass->getMemberVariableName(target), WorkUnitPtr());

    const TypePtr targetType = proteinClass->getMemberVariableType(target);
    if (targetType->inheritsFrom(objectVectorClass(doubleVectorClass(enumValueType, probabilityType))))
    {
      PieChartConfigurationPtr res;
      res = computeMultiClassStatistics(context, trainingProteins, target, T("Training"));
      context.resultCallback(T("Train"), res);

      res = computeMultiClassStatistics(context, validationProteins, target, T("Validation"));
      context.resultCallback(T("Validation"), res);

      res = computeMultiClassStatistics(context, testingProteins, target, T("Testing"));
      context.resultCallback(T("Test"), res);

      res = computeMultiClassStatistics(context, allProteins, target, T("Global"));
      context.resultCallback(T("All"), res);
    }
    else if (targetType->inheritsFrom(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType)))
    {
      PieChartConfigurationPtr res;
      res = computeBinaryStatistics(context, trainingProteins, target, T("Training"));
      context.resultCallback(T("Train"), res);
      
      res = computeBinaryStatistics(context, validationProteins, target, T("Validation"));
      context.resultCallback(T("Validation"), res);
      
      res = computeBinaryStatistics(context, testingProteins, target, T("Testing"));
      context.resultCallback(T("Test"), res);
      
      res = computeBinaryStatistics(context, allProteins, target, T("Global"));
      context.resultCallback(T("All"), res);
    }
    else if (target == cbpTarget)
    {
      PieChartConfigurationPtr res;
      res = computeMultiClassPropertyStatistics(context, trainingProteins, target, T("Training"));
      context.resultCallback(T("Train"), res);
      
      res = computeMultiClassPropertyStatistics(context, validationProteins, target, T("Validation"));
      context.resultCallback(T("Validation"), res);
      
      res = computeMultiClassPropertyStatistics(context, testingProteins, target, T("Testing"));
      context.resultCallback(T("Test"), res);
      
      res = computeMultiClassPropertyStatistics(context, allProteins, target, T("Global"));
      context.resultCallback(T("All"), res);
    }
    else if (targetType->inheritsFrom(doubleSymmetricMatrixClass(probabilityType)))
    {
      PieChartConfigurationPtr res;
      res = computeSymmetricMatrixStatistics(context, trainingProteins, target, T("Training"));
      context.resultCallback(T("Train"), res);
      
      res = computeSymmetricMatrixStatistics(context, validationProteins, target, T("Validation"));
      context.resultCallback(T("Validation"), res);
      
      res = computeSymmetricMatrixStatistics(context, testingProteins, target, T("Testing"));
      context.resultCallback(T("Test"), res);
      
      res = computeSymmetricMatrixStatistics(context, allProteins, target, T("Global"));
      context.resultCallback(T("All"), res);
    }
    else
      context.errorCallback(T("computeStatistics"), T("The target is not (yet) supported !"));

    context.leaveScope();
  }
  
  void computeDisulfideStatistics(ExecutionContext& context, const ContainerPtr& proteins) const
  {
    const size_t n = proteins->getNumElements();
    context.enterScope(T("Computing disulfide histograms"), WorkUnitPtr());
    
    HistogramConfigurationPtr configNumBridges = new HistogramConfiguration(1, 0, 30, false, T("#Seqs vs #Bridges"));
    HistogramConfigurationPtr configBridgeDensity = new HistogramConfiguration(0.00555555, 0, 0.18, false, T("#Seqs vs #Briges/Length"));
    for (size_t i = 0; i < n; i++)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>(context);
      jassert(protein);
      const DoubleVectorPtr& states = protein->getCysteinBondingStates(context);
      const size_t numStates = states->getNumElements();
      size_t numBondedCysteins = 0;
      for (size_t i = 0; i < numStates; ++i)
        if (states->getElement(i).getDouble() > 0.5)
          ++numBondedCysteins;
      jassert(numBondedCysteins % 2 == 0);
      
      configNumBridges->addData(numBondedCysteins / 2);
      configBridgeDensity->addData((numBondedCysteins / 2) / (double)protein->getLength());
    }
    context.resultCallback(T("#Seqs vs #Bridges"), configNumBridges);
    context.resultCallback(T("#Seqs vs #Briges/Length"), configBridgeDensity);

    context.leaveScope();
  }

  PieChartConfigurationPtr computeMultiClassStatistics(ExecutionContext& context, const ContainerPtr& proteins, ProteinTarget target, const String& name) const
  {
    EnumerationPtr enumeration = proteinClass->getMemberVariableType(target)->getTemplateArgument(0)->getTemplateArgument(0).dynamicCast<Enumeration>();
    jassert(enumeration);
    std::vector<size_t> counts(enumeration->getNumElements() + 1, 0);
    size_t totalCount = 0;
    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>();
      jassert(protein);
      ObjectVectorPtr vector = protein->getTargetOrComputeIfMissing(context, target).getObjectAndCast<ObjectVector>(context);
      if (!vector)
        continue;

      size_t numElements = vector->getNumElements();
      for (size_t j = 0; j < numElements; ++j)
      {
        size_t index = (size_t)-1;
        DoubleVectorPtr values = vector->getElement(j).getObjectAndCast<DoubleVector>();
        if (values)
          index = (size_t)values->getIndexOfMaximumValue();
        counts[index == (size_t)-1 ? counts.size() - 1 : index]++;
      }
      totalCount += numElements;
    }

    PieChartConfigurationPtr config = new PieChartConfiguration(name + T(" ") + proteinClass->getMemberVariableShortName(target),
                                                                T("Number of examples: ") + String((int)totalCount));
    for (size_t i = 0; i < enumeration->getNumElements(); ++i)
      config->appendElement(enumeration->getElementName(i) + T(" (") + String((int)counts[i]) + T(" examples)"), (double)counts[i]/(double)totalCount);
    config->appendElement(T("Missing (") + String((int)counts[counts.size() - 1]) + T(" examples)"), (double)counts[counts.size()-1]/(double)totalCount);

    return config;
  }
  
  PieChartConfigurationPtr computeBinaryStatistics(ExecutionContext& context, const ContainerPtr& proteins, ProteinTarget target, const String& name) const
  {
    enum {numBinaryElement = 3};
    std::vector<size_t> counts(numBinaryElement, 0);
    size_t totalCount = 0;

    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>();
      jassert(protein);
      DoubleVectorPtr vector = protein->getTargetOrComputeIfMissing(context, target).getObjectAndCast<DoubleVector>(context);
      if (!vector)
        continue;

      size_t numElements = vector->getNumElements();
      for (size_t j = 0; j < numElements; ++j)
      {
        size_t index = numBinaryElement - 1; // Missing
        Variable value = vector->getElement(j);
        if (value.exists())
          index = value.getDouble() > 0.5 ? 0 : 1;
        counts[index]++;
      }
      totalCount += numElements;
    }

    PieChartConfigurationPtr config = new PieChartConfiguration(name + T(" ") + proteinClass->getMemberVariableShortName(target),
                                                                T("Number of examples: ") + String((int)totalCount));
                                                                
    config->appendElement(T("True (") + String((int)counts[0]) + T(" examples)"), (double)counts[0]/(double)totalCount);
    config->appendElement(T("False (") + String((int)counts[1]) + T(" examples)"), (double)counts[1]/(double)totalCount);
    config->appendElement(T("Missing (") + String((int)counts[2]) + T(" examples)"), (double)counts[2]/(double)totalCount);

    return config;
  }
  
  PieChartConfigurationPtr computeMultiClassPropertyStatistics(ExecutionContext& context, const ContainerPtr& proteins, ProteinTarget target, const String& name) const
  {
    EnumerationPtr enumeration = proteinClass->getMemberVariableType(target)->getTemplateArgument(0).dynamicCast<Enumeration>();
    jassert(enumeration);
    std::vector<size_t> counts(enumeration->getNumElements() + 1, 0);
    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>();
      jassert(protein);

      size_t index = (size_t)-1;
      DoubleVectorPtr values = protein->getTargetOrComputeIfMissing(context, target).getObjectAndCast<DoubleVector>();
      if (values)
        index = (size_t)values->getIndexOfMaximumValue();
      counts[index == (size_t)-1 ? counts.size() - 1 : index]++;
    }
    
    PieChartConfigurationPtr config = new PieChartConfiguration(name + T(" ") + proteinClass->getMemberVariableShortName(target),
                                                                T("Number of examples: ") + String((int)n));
    for (size_t i = 0; i < enumeration->getNumElements(); ++i)
      config->appendElement(enumeration->getElementName(i) + T(" (")+ String((int)counts[i]) + T(" examples)"), (double)counts[i]/(double)n);
    config->appendElement(T("Missing (") + String((int)counts[counts.size() - 1]) + T(" examples)"), (double)counts[counts.size()-1]/(double)n);
    
    return config;
  }
  
  PieChartConfigurationPtr computeSymmetricMatrixStatistics(ExecutionContext& context, const ContainerPtr& proteins, ProteinTarget target, const String& name) const
  {
    enum {numBinaryElement = 3};
    std::vector<size_t> counts(numBinaryElement, 0);
    size_t totalCount = 0;

    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>();
      jassert(protein);
      SymmetricMatrixPtr matrix = protein->getTargetOrComputeIfMissing(context, target).getObjectAndCast<SymmetricMatrix>(context);
      if (!matrix)
        continue;

      size_t numElements = matrix->getNumElements();
      for (size_t j = 0; j < numElements; ++j)
      {
        size_t index = numBinaryElement - 1; // Missing
        Variable value = matrix->getElement(j);
        if (value.exists())
          index = value.getDouble() > 0.5 ? 0 : 1;
        counts[index]++;
      }
      totalCount += numElements;
    }

    PieChartConfigurationPtr config = new PieChartConfiguration(name + T(" ") + proteinClass->getMemberVariableShortName(target),
                                                                T("Number of examples: ") + String((int)totalCount));
    config->appendElement(T("True (") + String((int)counts[0]) + T(" examples)"), (double)counts[0]/(double)totalCount);
    config->appendElement(T("False (") + String((int)counts[1]) + T(" examples)"), (double)counts[1]/(double)totalCount);
    config->appendElement(T("Missing (") + String((int)counts[2]) + T(" examples)"), (double)counts[2]/(double)totalCount);

    return config;
  }
};

class EDAResultFileWriter : public Object
{
public:
  EDAResultFileWriter(const File& outputFile, const File& logFile)
    : outputFile(outputFile), logFile(logFile) {}
  EDAResultFileWriter() {}

  void write(String features, double trainScore, double testScore)
  {
    ScopedLock _(lock);

    OutputStream *o = outputFile.createOutputStream();
    *o 
    << trainScore << ";"
    << testScore << ";"
    << "\"" << features << "\"\n";
    delete o;
  }
  
  void writeLog(String features)
  {
    ScopedLock _(lock);
    OutputStream *o = logFile.createOutputStream();
    *o << features << "\"\n";
    delete o;
  }

protected:
  friend class EDAResultFileWriterClass;

  CriticalSection lock;
  File outputFile;
  File logFile;
};

typedef ReferenceCountedObjectPtr<EDAResultFileWriter> EDAResultFileWriterPtr;

class CysteinLearnerFunction : public Function
{
public:
  CysteinLearnerFunction(const File& inputDirectory, EDAResultFileWriterPtr fileWriter)
    : inputDirectory(inputDirectory), fileWriter(fileWriter) {}
  CysteinLearnerFunction() {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return numericalCysteinFeaturesParametersClass;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    fileWriter->writeLog(input.toString());
    
    size_t numStacks = 5;

    ContainerPtr trainingData = Protein::loadProteinsFromDirectoryPair(context, File(), inputDirectory.getChildFile(T("train/")), 0, T("Loading training proteins"));

    NumericalCysteinPredictorParametersPtr parameters = new NumericalCysteinPredictorParameters(input.getObjectAndCast<NumericalCysteinFeaturesParameters>(context));
    ProteinSequentialPredictorPtr predictor = new ProteinSequentialPredictor();
    for (size_t i = 0; i < numStacks; ++i)
    {
      ProteinPredictorPtr stack = new ProteinPredictor(parameters);
      stack->addTarget(cbpTarget);
      predictor->addPredictor(stack);
      
      stack = new ProteinPredictor(parameters);
      stack->addTarget(cbsTarget);
      predictor->addPredictor(stack);
      
      stack = new ProteinPredictor(parameters);
      stack->addTarget(dsbTarget);
      predictor->addPredictor(stack);
    }
    
    if (!predictor->train(context, trainingData, ContainerPtr(), T("Training")))
      return false;

    ProteinEvaluatorPtr trainEvaluator = new ProteinEvaluator();
    CompositeScoreObjectPtr trainScores = predictor->evaluate(context, trainingData, trainEvaluator, T("Evaluate on training proteins"));
    
    ContainerPtr testingData = Protein::loadProteinsFromDirectoryPair(context, File(), inputDirectory.getChildFile(T("test/")), 0, T("Loading testing proteins"));

    ProteinEvaluatorPtr testEvaluator = new ProteinEvaluator();
    CompositeScoreObjectPtr testScores = predictor->evaluate(context, testingData, testEvaluator, T("Evaluate on test proteins"));

    fileWriter->write(input.toString(), trainEvaluator->getScoreObjectOfTarget(trainScores, dsbTarget)->getScoreToMinimize()
                                    , testEvaluator->getScoreObjectOfTarget(testScores, dsbTarget)->getScoreToMinimize());

    return testEvaluator->getScoreObjectOfTarget(testScores, dsbTarget)->getScoreToMinimize();
  }

protected:
  friend class CysteinLearnerFunctionClass;

  File inputDirectory;
  EDAResultFileWriterPtr fileWriter;
};

class EDACysteinProteinLearner : public WorkUnit
{
public:
  Variable run(ExecutionContext& context)
  {
    size_t numIterations = 100;
    size_t populationSize = 5;
    size_t numBests = 1;

    EDAResultFileWriterPtr fileWriter = new EDAResultFileWriter(File::getCurrentWorkingDirectory().getChildFile(T("out.txt")), File::getCurrentWorkingDirectory().getChildFile(T("log.txt")));

    FunctionPtr f = new CysteinLearnerFunction(inputDirectory, fileWriter);
    SamplerPtr sampler = objectCompositeSampler(numericalCysteinFeaturesParametersClass, NumericalCysteinFeaturesParameters::createSamplers());

    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests, StoppingCriterionPtr(), 0.0);
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, f, FunctionPtr(), 10);
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler);

    return optimizer->compute(context, optimizerContext, optimizerState);
  }

protected:
  friend class EDACysteinProteinLearnerClass;

  File inputDirectory;
};

};
