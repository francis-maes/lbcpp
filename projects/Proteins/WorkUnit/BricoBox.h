#include <lbcpp/lbcpp.h>
#include "Data/Protein.h"
#include "../Evaluator/ProteinEvaluator.h"

#include "Data/Formats/SPXFileParser.h"

#ifdef LBCPP_NETWORKING
# include <lbcpp/Network/NetworkInterface.h>
# include <lbcpp/Network/NetworkNotification.h>
#endif

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

#if 1
class UnknownFromManagerWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    std::cout << "Mouahaha, I'm Fantomas" << std::endl;
    return true;
  }
};
#endif

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

class CheckSymmetricMatrixWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    SymmetricMatrixPtr mat = new DoubleSymmetricMatrix(doubleType, 5, 0.0);
    
    size_t n = mat->getNumElements();
    if (n != 15)
      context.errorCallback(T("CheckSymmetricMatrixWorkUnit::run"), T("Wrong number of elements: ") + String((int)n));

    for (size_t i = 0; i < n; ++i)
      mat->setElement(i, (double)i);
    
    context.informationCallback(mat->toString());
    return true;
  }
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
    
    WorkUnitNetworkRequestPtr request = new WorkUnitNetworkRequest(context, T("CheckManagerWorkUnit"), T("Alpha"), T("Omega"), new UnknownFromManagerWorkUnit(), 1, 1, 1);
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

class CysteinBondingPropertyStatisticsWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputDirectory == File::nonexistent)
      inputDirectory = File::getCurrentWorkingDirectory();
    
    ContainerPtr proteins = Protein::loadProteinsFromDirectory(context, inputDirectory, 0, T("Loading proteins"));
    std::vector<size_t> sums(4, 0);
    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>();
      Variable value = protein->getCysteinBondingProperty(context);
      if (!value.exists())
        sums[3]++;
      else
        sums[value.getObjectAndCast<DoubleVector>()->getIndexOfMaximumValue()]++;
    }
    
    std::cout << "All  : " << (sums[0] / (double)n) << std::endl;
    std::cout << "None : " << (sums[1] / (double)n) << std::endl;
    std::cout << "Mix  : " << (sums[2] / (double)n) << std::endl;
    std::cout << "Miss : " << (sums[3] / (double)n) << std::endl;
    
    return true;
  }
  
protected:
  friend class CysteinBondingPropertyStatisticsWorkUnitClass;
  
  File inputDirectory;
};

};
