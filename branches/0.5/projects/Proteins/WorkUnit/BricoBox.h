
#include "Data/Protein.h"
#include "../Evaluator/ProteinEvaluator.h"

#include "Data/Formats/SPXFileParser.h"
#include <lbcpp/Data/Stream.h>
#include <lbcpp/Data/Consumer.h>
#include <lbcpp/Execution/WorkUnit.h>
#include <lbcpp/Execution/ExecutionTrace.h>
#include <lbcpp/Optimizer/Optimizer.h>

#ifdef LBCPP_NETWORKING

#endif

#include <lbcpp/UserInterface/VariableSelector.h>

#include "../Predictor/NumericalCysteinPredictorParameters.h"
#include "../Predictor/Lin09PredictorParameters.h"
# include "../Model/SeparationProfileFunction.h"


#ifndef _PROTEINS_BRICO_BOX_
#define _PROTEINS_BRICO_BOX_
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
  
class SS3CompositionWithRespectToDRWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, ss3ProteinDirectory, drProteinDirectory);

    size_t numDisordered = 0;
    size_t numOrdered = 0;

    size_t numDisorderedSheets = 0;
    size_t numDisorderedHelices = 0;
    size_t numDisorderedCoils = 0;

    size_t numOrderedSheets = 0;
    size_t numOrderedHelices = 0;
    size_t numOrderedCoils = 0;

    for (size_t i = 0; i < proteins->getNumElements(); ++i)
    {
      ProteinPtr ss3Protein = proteins->getElement(i).getObjectAndCast<Pair>()->getFirst().getObjectAndCast<Protein>();
      ProteinPtr drProtein = proteins->getElement(i).getObjectAndCast<Pair>()->getSecond().getObjectAndCast<Protein>();
      jassert(ss3Protein);
      jassert(drProtein);
      jassert(ss3Protein->getLength() == drProtein->getLength());
      DenseDoubleVectorPtr dr = drProtein->getDisorderRegions();
      jassert(dr);
      ContainerPtr ss3 = ss3Protein->getSecondaryStructure();
      jassert(ss3);
      for (size_t j = 0; j < drProtein->getLength(); ++j)
      {
        jassert(dr->getElement(j).exists());
        jassert(ss3->getElement(j).exists());
        DoubleVectorPtr ss3Element = ss3->getElement(j).getObjectAndCast<DoubleVector>();
        if (dr->getValue(j) > 0.5)
        {
          ++numDisordered;
          switch (ss3Element->getIndexOfMaximumValue())
          {
            case helix: ++numDisorderedHelices; break;
            case sheet: ++numDisorderedSheets; break;
            case coil: ++numDisorderedCoils; break;
            default: jassertfalse;
          }
        }
        else
        {
          ++numOrdered;
          switch (ss3Element->getIndexOfMaximumValue())
          {
            case helix: ++numOrderedHelices; break;
            case sheet: ++numOrderedSheets; break;
            case other: ++numOrderedCoils; break;
            default: jassertfalse;
          }
        }
      }
    }

    std::cout << "      Ordered   Disordered   Total" << std::endl;
    std::cout << "Helix " << numOrderedHelices << "   " << numDisorderedHelices << "   " << (numOrderedHelices + numDisorderedHelices) << std::endl;
    std::cout << "Sheet " << numOrderedSheets << "   " << numDisorderedSheets << "   " << (numOrderedSheets + numDisorderedSheets) << std::endl;
    std::cout << "Coil  " << numOrderedCoils << "   " << numDisorderedCoils << "   " << (numOrderedCoils + numDisorderedCoils) << std::endl;
    std::cout << "      " << numOrdered << "   " << numDisordered << std::endl;

    return Variable();
  }
  
protected:
  friend class SS3CompositionWithRespectToDRWorkUnitClass;

  File ss3ProteinDirectory;
  File drProteinDirectory;
};

class SACompositionWithRespectToDRWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, saProteinDirectory, drProteinDirectory);

    size_t numDisordered = 0;
    size_t numOrdered = 0;

    size_t numDisorderedExposed = 0;
    size_t numDisorderedBuried = 0;

    size_t numOrderedExposed = 0;
    size_t numOrderedBuried = 0;

    for (size_t i = 0; i < proteins->getNumElements(); ++i)
    {
      ProteinPtr saProtein = proteins->getElement(i).getObjectAndCast<Pair>()->getFirst().getObjectAndCast<Protein>();
      ProteinPtr drProtein = proteins->getElement(i).getObjectAndCast<Pair>()->getSecond().getObjectAndCast<Protein>();
      jassert(saProtein);
      jassert(drProtein);
      jassert(saProtein->getLength() == drProtein->getLength());
      DenseDoubleVectorPtr dr = drProtein->getDisorderRegions();
      jassert(dr);
      DenseDoubleVectorPtr sa = saProtein->getSolventAccessibilityAt20p();
      jassert(sa);
      for (size_t j = 0; j < drProtein->getLength(); ++j)
      {
        jassert(dr->getElement(j).exists());
        jassert(sa->getElement(j).exists());
        if (dr->getValue(j) > 0.5)
        {
          ++numDisordered;
          if (sa->getValue(j) > 0.5)
            ++numDisorderedExposed;
          else
            ++numDisorderedBuried;
        }
        else
        {
          ++numOrdered;
          if (sa->getValue(j) > 0.5)
            ++numOrderedExposed;
          else
            ++numOrderedBuried;
        }
      }
    }

    std::cout << "          Ordered   Disordered   Total" << std::endl;
    std::cout << "Exposed " << numOrderedExposed << "   " << numDisorderedExposed << "   " << (numOrderedExposed + numDisorderedExposed) << std::endl;
    std::cout << "Buried  " << numOrderedBuried << "   " << numDisorderedBuried << "   " << (numOrderedBuried + numDisorderedBuried) << std::endl;
    std::cout << "        " << numOrdered << "   " << numDisordered << std::endl;

    return Variable();
  }
  
protected:
  friend class SACompositionWithRespectToDRWorkUnitClass;

  File saProteinDirectory;
  File drProteinDirectory;
};

class SASeparationProfileCompositionWithRespectToDRWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, saProteinDirectory, drProteinDirectory);

    FunctionPtr createProfile = new ProbabilityCreateSeparationProfileFunction();

    for (size_t i = 0; i < proteins->getNumElements(); ++i)
    {
      ProteinPtr saProtein = proteins->getElement(i).getObjectAndCast<Pair>()->getFirst().getObjectAndCast<Protein>();
      ProteinPtr drProtein = proteins->getElement(i).getObjectAndCast<Pair>()->getSecond().getObjectAndCast<Protein>();
      jassert(saProtein);
      jassert(drProtein);
      jassert(saProtein->getLength() == drProtein->getLength());
      DenseDoubleVectorPtr dr = drProtein->getDisorderRegions();
      jassert(dr);
      DenseDoubleVectorPtr sa = saProtein->getSolventAccessibilityAt20p();
      jassert(sa);

      SeparationProfilePtr profile = createProfile->compute(context, sa).getObjectAndCast<SeparationProfile>();

      
      std::vector<size_t> buriedOrdered(1000, 0), buriedDisordered(1000, 0), buriedMix(1000, 0), buried(1000, 0);
      computeComposition(context, profile->getProfile(0), dr, buriedOrdered, buriedDisordered, buriedMix, buried);
      
      std::vector<size_t> exposedOrdered(1000, 0), exposedDisordered(1000, 0), exposedMix(1000, 0), exposed(1000, 0);
      computeComposition(context, profile->getProfile(1), dr, exposedOrdered, exposedDisordered, exposedMix, exposed);

      for (size_t i = 0; i < 1000; ++i)
      {
        std::cout << ((int)i - 500) << "\t"
                  << buriedOrdered[i] << "\t" << buriedDisordered[i] << "\t"
                  << buriedMix[i] << "\t" << buried[i] << "\t"
                  << exposedOrdered[i] << "\t" << exposedDisordered[i] << "\t"
                  << exposedMix[i] << "\t" << exposed[i] << std::endl;
      }
    }

    return Variable();
  }
  
protected:
  friend class SASeparationProfileCompositionWithRespectToDRWorkUnitClass;

  File saProteinDirectory;
  File drProteinDirectory;

  void computeComposition(ExecutionContext& context, std::vector<size_t>& profile, const DenseDoubleVectorPtr& dr,
                          std::vector<size_t>& ordered, std::vector<size_t>& disordered,
                          std::vector<size_t>& mix, std::vector<size_t>& all) const
  {
    const size_t n = profile.size();
    for (size_t i = 0; i < n; ++i)
      std::cout << profile[i] << " ";
    std::cout << std::endl;
    
    for (size_t i = 0; i < n; ++i)
    {
      for (size_t j = 0; j < n; ++j)
      {
        if (i == j)
          continue;

        jassert(dr->getElement(i).exists());
        jassert(dr->getElement(j).exists());

        int sep = 500 + profile[j] - profile[i];
        if (sep < 0 || sep >= 1000 || sep == 500)
          continue;

        ++all[sep];
        if (dr->getValue(i) > 0.5 && dr->getValue(j) > 0.5)
          ++disordered[sep];
        else if (dr->getValue(i) <= 0.5 && dr->getValue(j) <= 0.5)
          ++ordered[sep];
        else
          ++mix[sep];
      }
    }
  }
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

class CountCysteinesWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectory(context, proteinDirectory, 0, T("Loading proteins"));

    size_t numCys = 0;
    size_t numBonded = 0;
    ScalarVariableMeanAndVariance stat;
    for (size_t i = 0; i < proteins->getNumElements(); ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>(context);
      const size_t n = protein->getCysteinIndices().size();
      numCys += n;

      size_t bonds = 0;
      SymmetricMatrixPtr matrix = protein->getDisulfideBonds(context);
      for (size_t j = 0; j < n; ++j)
        for (size_t jj = j + 1; jj < n; ++jj)
          bonds += matrix->getElement(j, jj).getDouble() * 2;
      stat.push(bonds);
      numBonded += bonds;
    }
    
    std::cout << "Num. Cysteines: " << numCys << std::endl;
    std::cout << "Num. Bonded:    " << numBonded << std::endl;
    std::cout << "Avg. Bonds:     " << stat.toString() << std::endl;
    return true;
  }

protected:
  friend class CountCysteinesWorkUnitClass;

  File proteinDirectory;
};

class GenerateFoldsWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    const String prefix(T(""));

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
      for (size_t j = 0; j < numFolds; ++j)
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
    : SimpleUnaryFunction(bfsTestParameterClass, scalarVariableMeanAndVarianceClass, T("BFSTest")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ReferenceCountedObjectPtr<BFSTestParameter> param = input.getObjectAndCast<BFSTestParameter>(context);
    juce::Thread::sleep(context.getRandomGenerator()->sampleSize(1000, 3000));

    ScalarVariableMeanAndVariancePtr res = new ScalarVariableMeanAndVariance();
    for (size_t i = 0; i < 10; ++i)
      res->push(fabs(param->a * param->a + param->b * param->c - 10) + context.getRandomGenerator()->sampleDouble(-5.f, 5.f));

    return res;
  }
};

class BFSTestWorkUnit : public WorkUnit
{
public:
  Variable run(ExecutionContext& context)
  {
    ExecutionContextPtr remoteContext = distributedExecutionContext(context, T("monster24.montefiore.ulg.ac.be"), 1664,
                                                                    T("testProject"), T("jbecker@mac"), T("jbecker@giga"),
                                                                    fixedResourceEstimator(1, 1024, 1), false);
    
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

    OptimizationProblemPtr problem = new OptimizationProblem(new BFSTestObjectiveFunction(), new BFSTestParameter(), SamplerPtr(), new BFSTestObjectiveFunction());
    
    return optimizer->compute(*remoteContext.get(), problem);
    //return optimizer->compute(context, problem);
  }
};

class SpaceSeparateDataParser : public TextParser
{
public:
  SpaceSeparateDataParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features)
    : TextParser(context, file), features(features),
      elementsType(pairClass(sparseDoubleVectorClass(features, doubleType), booleanType)) {}

  SpaceSeparateDataParser() {}
  
  virtual bool parseLine(const String& line)
  {
    String timmedline = line.trim();
    if (timmedline == String::empty)
      return true;
    std::vector<String> tokens;
    tokenize(timmedline, tokens);

    if (features->getNumElements() == 0)
    {
      for (size_t i = 0; i < tokens.size(); ++i)
        features->addElement(context, T("feature_") + String((int)i));
    }

    SparseDoubleVectorPtr ddv = new SparseDoubleVector(features, doubleType);
    for (size_t i = 0; i < tokens.size(); ++i)
    {
      const double value = tokens[i].getDoubleValue();
      if (value != 0)
         ddv->appendValue(i, value);
    }
     
    setResult(new Pair(elementsType, ddv, false));
    return true;
  }

  virtual TypePtr getElementsType() const
    {return elementsType;}

protected:
  DefaultEnumerationPtr features;
  TypePtr elementsType;
};

class NormalizeExampleCompositeFunction : public CompositeFunction
{
public:
  NormalizeExampleCompositeFunction(const FunctionPtr& decorated)
    : decorated(decorated) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(decorated->getRequiredInputType(0, 2));
    size_t supervision = builder.addInput(decorated->getRequiredInputType(1, 2));

    input = builder.addFunction(doubleVectorNormalizeFunction(true, true), input);
    builder.addFunction(decorated, input, supervision);
  }

protected:
  friend class NormalizeExampleCompositeFunctionClass;

  FunctionPtr decorated;

  NormalizeExampleCompositeFunction() {}
};

class PrintFeaturesFunction : public Function
{
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)doubleVectorClass() : anyType;}
  
  virtual String getOutputPostFix() const
    {return T("Print");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return booleanType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    DenseDoubleVectorPtr input = inputs[0].getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
    for (size_t i = 0; i < input->getNumValues(); ++i)
      std::cout << input->getValue(i) << " ";
    std::cout << std::endl;
    return true;
  }
};

class ExportFeaturesWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinDirectory), 0, T("Loading proteins"));

    std::vector<DoubleVectorPtr> features;
    buildFeatures(context, proteins, features);
    
    std::vector<size_t> order;
    context.getRandomGenerator()->sampleOrder(features.size(), order);

#if 1
    VectorPtr data = vector(pairClass(features[0]->getClass(), booleanType), features.size());
    for (size_t i = 0; i < features.size(); ++i)
      data->setElement(i, new Pair(features[i], false));

    FunctionPtr printer = new PrintFeaturesFunction();
    for (size_t i = 0; i < 1; ++i)
      printer->compute(context, features[order[i]], false);

    std::cout << "Normalization" << std::endl;
    FunctionPtr norm = new NormalizeExampleCompositeFunction(new PrintFeaturesFunction());
    if (!norm->train(context, data))
      return false;
    for (size_t i = 0; i < 1; ++i)
      norm->compute(context, features[order[i]], false);
    return true;
#endif // !0

    std::vector<double> zFactor;
    computeVariances(features, zFactor);

    OutputStream* o = context.getFile(T("dsb.features")).createOutputStream();
    for (size_t i = 0; i < features.size(); ++i)
    {
      DenseDoubleVectorPtr v = features[order[i]]->toDenseDoubleVector();
      const size_t n = v->getNumElements();
      for (size_t j = 0; j < n; ++j)
      {
        Variable value = v->getElement(j);
        *o << (value.exists() && zFactor[j] != 0.f ? value.getDouble() / sqrt(zFactor[j]) : 0.0) << " ";
      }
      *o << "\n";
    }
    delete o;
    return true;
  }

protected:
  friend class ExportFeaturesWorkUnitClass;

  String proteinDirectory;

  void buildFeatures(ExecutionContext& context, const ContainerPtr& proteins, std::vector<DoubleVectorPtr>& features) const
  {
    Lin09ParametersPtr fp = new Lin09Parameters();
    fp->useProteinLength= true;
    fp->pssmWindowSize = 20;
    fp->separationProfilSize = 9;
    fp->useCysteinDistance = true;
    fp->pssmLocalHistogramSize = 70;
    fp->aminoAcidLocalHistogramSize = 30;
    Lin09PredictorParametersPtr predictorParameters = new Lin09PredictorParameters(fp);
    
    FunctionPtr proteinPerception = predictorParameters->createProteinPerception();
    FunctionPtr disulfideFunction = predictorParameters->createDisulfideSymmetricResiduePairVectorPerception();

    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable perception = proteinPerception->compute(context, proteins->getElement(i).getObjectAndCast<Pair>()->getFirst());
      SymmetricMatrixPtr featuresVector = disulfideFunction->compute(context, perception).getObjectAndCast<SymmetricMatrix>();
      SymmetricMatrixPtr supervision = proteins->getElement(i).getObjectAndCast<Pair>()->getSecond().getObjectAndCast<Protein>()->getDisulfideBonds(context);
      jassert(featuresVector && supervision && featuresVector->getDimension() == supervision->getDimension());
      const size_t dimension = featuresVector->getDimension();
      for (size_t j = 0; j < dimension; ++j)
        for (size_t k = j + 1; k < dimension; ++k)
          features.push_back(featuresVector->getElement(j,k).getObjectAndCast<DoubleVector>());
    }
  }

  void computeVariances(const std::vector<DoubleVectorPtr>& examples, std::vector<double>& zFactor) const
  {
    if (examples.size() == 0)
      return;

    const EnumerationPtr enumeration = examples[0]->getElementsEnumeration();
    const size_t numFeatures = enumeration->getNumElements();

    std::vector<ScalarVariableMeanAndVariancePtr> variances(numFeatures);
    for (size_t i = 0; i < numFeatures; ++i)
      variances[i] = new ScalarVariableMeanAndVariance();

    for (size_t i = 0; i < examples.size(); ++i)
    {
      SparseDoubleVectorPtr s = examples[i]->toSparseVector();
      for (size_t j = 0; j < s->getNumValues(); ++j)
      {
        const std::pair<size_t, double>& value = s->getValue(j);
        variances[value.first]->push(value.second);
      }
    }

    zFactor.resize(numFeatures);
    for (size_t i = 0; i < numFeatures; ++i)
      zFactor[i] = variances[i]->getVariance();
  }
};

class LSHTestWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    DefaultEnumerationPtr features = new DefaultEnumeration(T("a1aEnumeration"));
    ContainerPtr trainingData;
    //trainingData = (new SpaceSeparateDataParser(context, context.getFile(T("/Users/jbecker/Desktop/E2LSH-0.1/mnist1k.dts")), features))->load()->randomize();
    trainingData = binaryClassificationLibSVMDataParser(context, File::getCurrentWorkingDirectory().getChildFile(T("../../projects/Examples/Data/BinaryClassification/a1a.train")), features)->load()->randomize();
    FunctionPtr lsh = new NormalizeExampleCompositeFunction(binaryLocalitySensitiveHashing(5));
    if (!lsh->train(context, trainingData, ContainerPtr(), T("Training - Examples: ") + String((int)trainingData->getNumElements())))
      return false;
    //return true;
    ContainerPtr testingData;
    //testingData = (new SpaceSeparateDataParser(context, context.getFile(T("/Users/jbecker/Desktop/E2LSH-0.1/mnist1k.q")), features))->load()->randomize();
    testingData = binaryClassificationLibSVMDataParser(context, File::getCurrentWorkingDirectory().getChildFile(T("../../projects/Examples/Data/BinaryClassification/a1a.test")), features)->load()->randomize();
    ScoreObjectPtr score = lsh->evaluate(context, testingData, binaryClassificationCurveEvaluator(binaryClassificationAccuracyScore, true), T("Evaluation - Examples: ") + String((int)testingData->getNumElements()));

#if 0
    double sum = 0.f;
    size_t numTrues = 0;
    for (size_t i = 0; i < testingData->getNumElements(); ++i)
    {
      numTrues += testingData->getElement(i).getObject()->getVariable(1).getBoolean() ? 1 : 0;
      Variable v = lsh->compute(context, testingData->getElement(i).getObject()->getVariable(0), Variable());
      //std::cout << v.toString() << std::endl;
      sum += v.getDouble();
    }

    std::cout << "Coverage: " << (sum / (double)testingData->getNumElements()) << std::endl;
    
    context.informationCallback(T("% of true: ") + String(numTrues / (double)testingData->getNumElements()));
    context.informationCallback(T("Coverage: ") + String(sum / (double)testingData->getNumElements()));
#endif //!0
    return true;
  }
};

class CopyCysteinBondingPropertyWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ProteinPtr cbpProtein = Protein::createFromXml(context, cbpProteinFile);
    ProteinPtr targetProtein = Protein::createFromXml(context, targetProteinFile);

    if (!cbpProtein || !targetProtein)
      return false;

    if (cbpProtein->toString() != targetProtein->toString())
    {
      jassertfalse;
      return false;
    }

    targetProtein->setCysteinBondingProperty(cbpProtein->getCysteinBondingProperty(context).getDouble());
    targetProtein->saveToFile(context, targetProteinFile);
    return true;
  }

protected:
  friend class CopyCysteinBondingPropertyWorkUnitClass;

  File cbpProteinFile;
  File targetProteinFile;
};

class ManuallyAddDisulfidePatternWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ProteinPtr protein = Protein::createFromXml(context, proteinFile);
    if (!protein)
      return false;

    String ps = protein->getPrimaryStructure()->toString();
    std::cout << ps << std::endl;
    const std::vector<size_t>& cysIndices = protein->getCysteinIndices();
    std::cout << "Num. Cysteins: " << cysIndices.size() << std::endl;
    for (size_t i = 0; i < cysIndices.size(); ++i)
    {
      std::cout << "[" << i << "] Position: " << cysIndices[i] << " : ";
      String localPs = ps.substring(cysIndices[i] - 5, cysIndices[i] + 6);
      for (size_t j = 0; j < (size_t)localPs.length(); ++j)
        std::cout << AminoAcid::toThreeLettersCode((AminoAcidType)AminoAcid::fromOneLetterCode(localPs[j]).getInteger()) << " ";
      std::cout << std::endl;
    }

    SymmetricMatrixPtr dsb = Protein::createEmptyContactMap(cysIndices.size());

// INSERT BOND HERE
    dsb->setElement(0, 3, 1.f);
    dsb->setElement(1, 2, 1.f);
    dsb->setElement(7, 8, 1.f);

    protein->setDisulfideBonds(dsb);
    protein->saveToFile(context, proteinFile);

    return true;
  }

protected:
  friend class ManuallyAddDisulfidePatternWorkUnitClass;

  File proteinFile;
};

class ExtractPrimaryAndDisroderedSequencesFromPdb : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputFile.isDirectory())
    {
      juce::OwnedArray<File> files;
      inputFile.findChildFiles(files, File::findFiles, false, T("*.pdb"));
      bool res = true;
      for (size_t i = 0; i < (size_t)files.size(); ++i)
        res &= extract(context, *files[i], outputFile.getChildFile(files[i]->getFileName()));
      return res;
    }
    else
      return extract(context, inputFile, outputFile);
  }

protected:
  friend class ExtractPrimaryAndDisroderedSequencesFromPdbClass;

  File inputFile;
  File outputFile;

  bool extract(ExecutionContext& context, File pdbFile, File output) const
  {
    ProteinPtr protein = Protein::createFromPDB(context, pdbFile);
    if (!protein)
    {
      context.errorCallback(T("ExtractPrimaryAndDisroderedSequencesFromPdb::extract"), T("No protein parsed in file: ") + pdbFile.getFullPathName());
      return false;
    }

    OutputStream* o = output.createOutputStream();
    *o << protein->getPrimaryStructure()->toString() << "\n";
    *o << protein->getDisorderRegions()->toString();
    delete o;
    return true;
  }
};

};

#endif // !_PROTEINS_BRICO_BOX_
