#include "Data/Protein.h"
#include "../Evaluator/ProteinEvaluator.h"

#include "Data/Formats/SPXFileParser.h"
#include <lbcpp/Data/Stream.h>

#ifdef LBCPP_NETWORKING

#endif

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
    juce::Thread::sleep(context.getRandomGenerator()->sampleSize(1000, 3000));
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

    std::vector<SamplerPtr> samplers(3);
    for (size_t i = 0; i < 3; ++i)
      samplers[i] = gaussianSampler(0.0, 2.0);
    
    OptimizerPtr optimizer = bestFirstSearchOptimizer(streams, context.getFile(T("osTest.xml")));
    //OptimizerPtr optimizer = edaOptimizer(objectCompositeSampler(bfsTestParameterClass, samplers), 20, 20, 5);
    
    OptimizationProblemPtr problem = new OptimizationProblem(new BFSTestObjectiveFunction(), new BFSTestParameter());
    
    return optimizer->compute(context, problem);
  }
};

};
