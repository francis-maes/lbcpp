
#include <lbcpp/Core/Function.h>
#include "../Predictor/LargeProteinPredictorParameters.h"

namespace lbcpp
{

class NeedlemanWunschAlgorithm : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return stringType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return pairClass(stringType, stringType);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    enum {gapPenality = -1};
    const String a = inputs[0].toString();
    const String b = inputs[1].toString();

    const size_t aLength = a.length() + 1;
    const size_t bLength = b.length() + 1;
    // Initialize matrix
    int** matrix = new int*[aLength];
    jassert(matrix);
    for (size_t i = 0; i < aLength; ++i)
    {
      matrix[i] = new int[bLength];
      jassert(matrix[i]);
    }

    for (size_t i = 0; i < aLength; ++i)
      matrix[i][0] = i * gapPenality;
    for (size_t i = 0; i < bLength; ++i)
      matrix[0][i] = i * gapPenality;
    for (size_t i = 1; i < aLength; ++i)
      for (size_t j = 1; j < bLength; ++j)
      {
        int matchScore  = matrix[i - 1][j - 1] + similarityScore(a[i-1], b[j-1]);
        int deleteScore = matrix[i - 1][j] + gapPenality;
        int insertScore = matrix[i][j - 1] + gapPenality;
        matrix[i][j] = juce::jmax(matchScore, deleteScore, insertScore);
      }

    // Reconstruct alignment
    String aResult;
    String bResult;
    int i = aLength - 1;
    int j = bLength - 1;
    while (i > 0 && j > 0)
    {
      const int score = matrix[i][j];
      const int matchScore = matrix[i - 1][j - 1];
      const int deleteScore = matrix[i - 1][j];
      const int insertScore = matrix[i][j - 1];
      // preference for gap at begining and end of sequences
      if ((size_t)(i + j) < (aLength + bLength) / 2 && score == matchScore + similarityScore(a[i-1], b[j-1]))
      {
        aResult += a[i-1];
        bResult += b[j-1];
        --i; --j;
      }
      else if (score == deleteScore + gapPenality)
      {
        aResult += a[i-1];
        bResult += T('_');
        --i;
      }
      else if (score == insertScore + gapPenality)
      {
        aResult += T('_');
        bResult += b[j-1];
        --j;
      }
      else if (score == matchScore + similarityScore(a[i-1], b[j-1]))
      {
        aResult += a[i-1];
        bResult += b[j-1];
        --i; --j;
      }
      else
        jassertfalse;
    }
    for ( ; i > 0; --i)
    {
      aResult += a[i-1];
      bResult += T('_');
    }
    for ( ; j >  0; --j)
    {
      aResult += T('_');
      bResult += b[j-1];
    }

    for (size_t i = 0; i < aLength; ++i)
      delete matrix[i];
    delete matrix;

    return new Pair(revertString(aResult), revertString(bResult));
  }

protected:
  int similarityScore(juce::tchar a, juce::tchar b) const
    {return a == b ? 1 : -5;}

  String revertString(const String& str) const
  {
    String res;
    const size_t n = str.length();
    for (int i = n - 1; i >= 0; --i)
      res += str[i];
    return res;
  }
};

class CompareSPXFromFileAndFromPDB : public WorkUnit
{
public:
  Variable run(ExecutionContext& context)
  {
    size_t numConvertedFile = 0;

    juce::OwnedArray<File> files;
    fromFile.findChildFiles(files, File::findFiles, false, T("*.xml"));
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      File f = *files[i];
      String fileName = f.getFileNameWithoutExtension();
      jassert(fileName.length() == 5);
      ProteinPtr proteinFromFile = Protein::createFromFile(context, f).dynamicCast<Protein>();
      jassert(proteinFromFile);

      File outputFile(output.getChildFile(f.getFileName()));
      if (outputFile.exists())
      {
        context.progressCallback(new ProgressionState(i + 1, files.size(), T("file")));
        continue;
      }

      String pdbFileName = fileName.substring(0, 4) + T("_") + fileName.getLastCharacter();
      File pdbFile = fromPdb.getChildFile(pdbFileName + T(".xml"));

      if (!pdbFile.exists())
      {
        context.errorCallback(T("File not found: ") + pdbFile.getFileNameWithoutExtension());
        f.deleteFile();
        continue;
      }

      ProteinPtr proteinFromPdb = Protein::createFromFile(context, pdbFile);
      jassert(proteinFromPdb);

      context.enterScope(proteinFromFile->getName() + T(" vs ") + proteinFromPdb->getName());

      String strFromFile = proteinFromFile->getPrimaryStructure()->toString();
      String strFromPdb  = proteinFromPdb->getPrimaryStructure()->toString();
      
      strFromFile = strFromFile.replace(T("_"), T(""));
      jassert(!strFromPdb.contains(T("_")));

      FunctionPtr needlemanWunsch = new NeedlemanWunschAlgorithm();
      PairPtr alignment = needlemanWunsch->compute(context, strFromFile, strFromPdb).getObjectAndCast<Pair>();

      String alignFromFile = alignment->getFirst().toString();
      String alignFromPdb = alignment->getSecond().toString();

      if (alignFromPdb.contains(T("_")))
      {
        context.errorCallback(T("PDB contains gap"));

         std::cout << std::endl
         << fileName << " [" << i << "]" << std::endl
         << "fromFile: " << strFromFile << std::endl
         << "fromPdb : " << strFromPdb  << std::endl
         << "aFile:    " << alignFromFile << std::endl
         << "aPdb:     " << alignFromPdb << std::endl;
        
        for (int index = alignFromPdb.indexOfChar(T('_')); index != -1; index = alignFromPdb.indexOfChar(index, T('_')))
          alignFromFile[index++] = T('_');
      }

      const String trimmedAlign = trimCharacter(alignFromFile, T('_'));
      const size_t n = trimmedAlign.length();
      
      ProteinPtr convertedProtein = new Protein(proteinFromFile->getName());

      VectorPtr ps = genericVector(aminoAcidTypeEnumeration, n);
      convertedProtein->setPrimaryStructure(ps);
      const VectorPtr psFromFile = proteinFromFile->getPrimaryStructure();
      const VectorPtr psFromPdb = proteinFromPdb->getPrimaryStructure();
      
      ContainerPtr pssm = Protein::createEmptyPositionSpecificScoringMatrix(n);
      convertedProtein->setPositionSpecificScoringMatrix(pssm);
      ContainerPtr pssmFromFile = proteinFromFile->getPositionSpecificScoringMatrix();
      if (!pssmFromFile)
        pssmFromFile = Protein::createEmptyPositionSpecificScoringMatrix(proteinFromFile->getLength());
      
      ContainerPtr dssp = Protein::createEmptyDSSPSecondaryStructure(n, true);
      convertedProtein->setDSSPSecondaryStructure(dssp);
      const ContainerPtr dsspFromFile = proteinFromFile->getSecondaryStructure();
      
      DoubleVectorPtr sa = Protein::createEmptyProbabilitySequence(n);
      convertedProtein->setSolventAccessibilityAt20p(sa);
      const DoubleVectorPtr saFromFile = proteinFromFile->getSolventAccessibilityAt20p();
      
      TertiaryStructurePtr ts = new TertiaryStructure(n);
      convertedProtein->setTertiaryStructure(ts);
      const TertiaryStructurePtr tsFromPdb = proteinFromPdb->getTertiaryStructure();

      size_t shiftPdb = 0;
      for ( ; shiftPdb < (size_t)alignFromFile.length() && alignFromFile[shiftPdb] == T('_'); ++shiftPdb);
      strFromFile = proteinFromFile->getPrimaryStructure()->toString();
      for (size_t j = 0, indexFile = 0; j < n; ++j)
      {
        if (trimmedAlign[j] != T('_'))
        {
          if (strFromFile[indexFile] == T('_'))
          {
            ++indexFile;
            --j;
            continue;
          }
          else
          {
            ps->setElement(j, psFromFile->getElement(indexFile));
            pssm->setElement(j, pssmFromFile->getElement(indexFile));
            dssp->setElement(j, dsspFromFile->getElement(indexFile));
            sa->setElement(j, saFromFile->getElement(indexFile));
            ++indexFile;
          }
        }
        else
        {
          if (strFromFile[indexFile] == T('_'))
          {
            ps->setElement(j, psFromPdb->getElement(shiftPdb + j));
            pssm->setElement(j, pssmFromFile->getElement(indexFile));
            dssp->setElement(j, dsspFromFile->getElement(indexFile));
            sa->setElement(j, saFromFile->getElement(indexFile));
            ++indexFile;
          }
          else
          {
            ps->setElement(j, psFromPdb->getElement(shiftPdb + j));
          }
        }
        ts->setResidue(j, tsFromPdb->getResidue(shiftPdb + j));
      }

      if (checkPrimaryStructure(context, convertedProtein, proteinFromPdb)
          && checkTertiaryStructure(context, proteinFromPdb)
          && checkSecondaryStructureAndSolventAcceccibility(context, convertedProtein))
      {
        convertedProtein->saveToFile(context, outputFile);
        ++numConvertedFile;
      }
      else
        context.errorCallback(T("No matching"));

      context.leaveScope();
      context.progressCallback(new ProgressionState(i + 1, files.size(), T("file")));
    }

    context.resultCallback(T("numConvertedFile"), numConvertedFile);

    return numConvertedFile;
  }

protected:
  friend class CompareSPXFromFileAndFromPDBClass;

  File fromFile;
  File fromPdb;
  File output;

  String trimCharacter(const String& str, juce::tchar characterToTrim) const
  {
    size_t startIndex = 0;
    for ( ; startIndex < (size_t)str.length() && str[startIndex] == characterToTrim; ++startIndex);
    size_t endIndex = str.length();
    for ( ; endIndex > 0 && str[endIndex - 1] == characterToTrim; --endIndex);    
    return endIndex > startIndex ? str.substring(startIndex, endIndex) : T("");
  }

  bool checkPrimaryStructure(ExecutionContext& context, const ProteinPtr& a, const ProteinPtr& b) const
  {
    if (a->getPrimaryStructure() && b->getPrimaryStructure()
        && b->getPrimaryStructure()->toString().contains(a->getPrimaryStructure()->toString()))
      return true;
    context.errorCallback(T("checkPrimaryStructure failed"));

    std::cout << "b must contains a" << std::endl
    << "a: " << a->getPrimaryStructure()->toString() << std::endl
    << "b: " << b->getPrimaryStructure()->toString() << std::endl;
    return false;
  }

  bool checkSecondaryStructureAndSolventAcceccibility(ExecutionContext& context, const ProteinPtr& p) const
  {
    if (p->getSecondaryStructure() && p->getSolventAccessibilityAt20p()
        && p->getLength() == p->getSecondaryStructure()->getNumElements()
        && p->getLength() == p->getSolventAccessibilityAt20p()->getNumElements())
      return true;
    context.errorCallback(T("checkSecondaryStructureAndSolventAcceccibility failed"));
    return false;
  }

  bool checkTertiaryStructure(ExecutionContext& context, const ProteinPtr& p) const
  {
    if (p->getTertiaryStructure()
        && p->getLength() == p->getTertiaryStructure()->getNumResidues())
    {
      bool isOk = true;
      const VectorPtr ps = p->getPrimaryStructure();
      const TertiaryStructurePtr ts = p->getTertiaryStructure();
      const size_t n = ts->getNumResidues();
      for (size_t i = 0; i < n; ++i)
        if (ts->getResidue(i) && ts->getResidue(i)->getAminoAcidType() != ps->getElement(i).getInteger())
        {
          isOk = false;
          break;
        }
      if (isOk)
        return true;
    }
    context.errorCallback(T("checkTertiaryStructure"));
    return false;
  }
};

class ProteinLearnerFunction : public SimpleUnaryFunction
{
public:
  ProteinLearnerFunction(ProteinTarget target, const String& proteinsPath
                         , const ContainerPtr& trainingProteins, const ContainerPtr& testingProteins
                         , const LargeProteinPredictorParametersPtr& largePredictor)
    : SimpleUnaryFunction(largeProteinParametersClass, doubleType, T("ProteinLearner"))
    , target(target), proteinsPath(proteinsPath)
    , trainingProteins(trainingProteins), testingProteins(testingProteins)
    , largePredictor(largePredictor) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    LargeProteinParametersPtr parameters = input.getObjectAndCast<LargeProteinParameters>(context);
    if (!parameters)
      return 101.f;

    ContainerPtr trainData = trainingProteins;
    if (!trainData)
      trainData = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinsPath).getChildFile(T("train/")), 0,
                                             T("Loading training proteins"));

    if (trainData->getNumElements() == 0)
      return 102.f;

    LargeProteinPredictorParametersPtr clone = largePredictor->cloneAndCast<LargeProteinPredictorParameters>(context);
    clone->setParameters(parameters);

    ProteinPredictorPtr predictor = new ProteinPredictor(clone);
    predictor->addTarget(target);

    if (!predictor->train(context, trainData, ContainerPtr(), T("Training")))
      return 103.f;
  
    ContainerPtr testData = testingProteins;
    if (!testData)
      testData = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinsPath).getChildFile(T("test/")), 0,
                                                        T("Loading testing proteins"));
    if (testData->getNumElements() == 0)
      return 103.f;

    ProteinEvaluatorPtr testEvaluator = new ProteinEvaluator();
    CompositeScoreObjectPtr testScores = predictor->evaluate(context, testData, testEvaluator, T("Evaluate on test proteins"));
    return testEvaluator->getScoreObjectOfTarget(testScores, target)->getScoreToMinimize();
  }

protected:
  friend class ProteinLearnerFunctionClass;

  ProteinTarget target;

  String proteinsPath;
  ContainerPtr trainingProteins;
  ContainerPtr testingProteins;

  LargeProteinPredictorParametersPtr largePredictor;

  ProteinLearnerFunction() : SimpleUnaryFunction(proteinPredictorParametersClass, doubleType, T("ProteinLearner")) {}
};

class BestFirstSearchProteinLearner : public WorkUnit
{
public:
  BestFirstSearchProteinLearner() : loadProteins(false), target(noTarget), learningMachine(T("kNN")) {}

  Variable run(ExecutionContext& context)
  {
    if (target == noTarget)
      return false;
    ContainerPtr trainingProteins;
    ContainerPtr testingProteins;
    if (loadProteins)
    {
      trainingProteins = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinsPath).getChildFile(T("train/")), 0, T("Loading training proteins"));
      testingProteins = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinsPath).getChildFile(T("test/")), 0, T("Loading testing proteins"));
    }

    LargeProteinPredictorParametersPtr largePredictor = new LargeProteinPredictorParameters();
    largePredictor->learningMachineName = learningMachine;

    FunctionPtr toOptimize = new ProteinLearnerFunction(target, proteinsPath, trainingProteins, testingProteins, largePredictor);
    OptimizationProblemPtr problem = new OptimizationProblem(toOptimize, new LargeProteinParameters());
    OptimizerPtr optimizer = bestFirstSearchOptimizer(LargeProteinParameters::createSingleTaskSingleStageStreams(), context.getFile(optimizerFile));
    return optimizer->compute(context, problem);
  }

protected:
  friend class BestFirstSearchProteinLearnerClass;

  bool loadProteins;
  String proteinsPath;

  ProteinTarget target;
  String optimizerFile;
  String learningMachine;
};

class ExporteDisulfidePatternScoreObject : public DisulfidePatternScoreObject
{
public:
  ExporteDisulfidePatternScoreObject(OutputStream* o) : o(o) {}

  virtual void addPrediction(bool isCorrect)
  {
    DisulfidePatternScoreObject::addPrediction(isCorrect);
    *o << " " << (isCorrect ? 1 : 0);
  }

private:
  OutputStream* o;
};

class ExportPredictionWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinDirectory), 0, T("Loading proteins"))->randomize();

    OutputStream* o = context.getFile(T("bfs_dsb.bandit")).createOutputStream();

    const std::vector<StreamPtr> streams = Lin09Parameters::createStreams();
    const ObjectPtr baseObject = Lin09Parameters::createInitialObject();
    for (size_t i = 0; i < streams.size(); ++i)
    {
      StreamPtr stream = streams[i];
      stream->rewind();
      for (size_t valueIndex = 0; !stream->isExhausted(); ++valueIndex)
      {
        const Variable value = stream->next();
        //std::cout << lin09ParametersClass->getMemberVariableName(i) << std::endl;
        *o << lin09ParametersClass->getMemberVariableName(i) << "[" << value.toString() << "]";
        std::cout << lin09ParametersClass->getMemberVariableName(i) << "[" << value.toString() << "] - ";
        ObjectPtr candidate = baseObject->clone(context);
        candidate->setVariable(i, value);

        exportPredictionOfCandidate(context, candidate, proteins, o);

        *o << "\n";
      }
    }

    delete o;
    return true;
  }

protected:
  friend class ExportPredictionWorkUnitClass;

  String proteinDirectory;
  
  void exportPredictionOfCandidate(ExecutionContext& context, const ObjectPtr& obj, const ContainerPtr& proteins, OutputStream* const o) const
  {
    Lin09PredictorParametersPtr predictorParameters = new Lin09PredictorParameters(obj.staticCast<Lin09Parameters>());
    
    FunctionPtr proteinPerception = predictorParameters->createProteinPerception();
    FunctionPtr disulfideFunction = predictorParameters->createDisulfideSymmetricResiduePairVectorPerception();
    
    VectorPtr examples = vector(pairClass(doubleVectorClass(enumValueType, doubleType), probabilityType)); 
    
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
          examples->append(new Pair(featuresVector->getElement(j,k), supervision->getElement(j,k)));
    }

    FunctionPtr knnFunction = binaryNearestNeighbor(5, true, false);
    if (!knnFunction->train(context, examples))
      jassertfalse;
    
    EvaluatorPtr eval = new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6, 0.0), 0.0);
    ScoreObjectPtr score = new ExporteDisulfidePatternScoreObject(o);
    for (size_t i = 0; i < n; ++i)
    {
      Variable perception = proteinPerception->compute(context, proteins->getElement(i).getObjectAndCast<Pair>()->getFirst());
      SymmetricMatrixPtr featuresVector = disulfideFunction->compute(context, perception).getObjectAndCast<SymmetricMatrix>();
      SymmetricMatrixPtr supervision = proteins->getElement(i).getObjectAndCast<Pair>()->getSecond().getObjectAndCast<Protein>()->getDisulfideBonds(context);

      const size_t dimension = featuresVector->getDimension();
      SymmetricMatrixPtr prediction = symmetricMatrix(probabilityType, dimension);
      for (size_t j = 0; j < dimension; ++j)
        for (size_t k = j + 1; k < dimension; ++k)
        {
          Variable value = knnFunction->compute(context, featuresVector->getElement(j,k), Variable());
          prediction->setElement(j, k, value);
        }
      eval->updateScoreObject(context, score, new Pair(featuresVector, supervision), prediction);
    }
    eval->finalizeScoreObject(score, FunctionPtr());
    std::cout << "Score: " << score->getScoreToMinimize() << std::endl;
/*
    ContainerPtr randomizedExamples = examples->randomize();
    const size_t m = randomizedExamples->getNumElements();
    for (size_t i = 0; i < m; ++i)
    {
      Variable prediction = knnFunction->compute(context, randomizedExamples->getElement(i).getObjectAndCast<Pair>()->getFirst(), Variable());
      bool res = prediction.getDouble() > 0.5
                  == randomizedExamples->getElement(i).getObjectAndCast<Pair>()->getSecond().getDouble() > 0.5;
      *o << " " << (res ? 1 : 0);
    }
*/
  }
};

class SamplingFunction : public SimpleUnaryFunction
{
public:
  SamplingFunction() : SimpleUnaryFunction(doubleType, doubleType, T("Sampling")) {}

protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    return (double)fabs(context.getRandomGenerator()->sampleDoubleFromGaussian(input.getDouble(), input.getDouble()));
  }
};

class TestBanditEDAOptimizer : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    OptimizationProblemPtr problem = new OptimizationProblem(new SamplingFunction(), Variable(), uniformScalarSampler(0.f, 1.f));
    OptimizerPtr optimizer = banditEDAOptimizer(5, 20, 5, 0.5f, 1000, 2);
    return optimizer->compute(context, problem);
  }
};

};
