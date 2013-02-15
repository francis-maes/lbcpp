/*-----------------------------------------.---------------------------------.
| Filename: ModelBox.h                     |                                 |
| Author  : Julien Becker                  |                                 |
| Started : 09/02/2012 16:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef _PROTEINS_MODEL_BOX_H_
# define _PROTEINS_MODEL_BOX_H_

# include <lbcpp/Core/Function.h>
# include "../Model/SimpleProteinModel.h"
# include "../Evaluator/SegmentOverlapEvaluator.h"

namespace lbcpp
{

class SimpleModelWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr trainingProteins;
    ContainerPtr testingProteins;
    if (!loadProteins(context, trainingProteins, testingProteins))
      return false;

    SimpleProteinModelPtr m = new SimpleProteinModel(ss3Target);
    m->pssmWindowSize = 15;
    m->useNumCysteines = true;
    m->useNumOfEachResidue = true;
    m->aaDimericProfile = true;
    m->usePosition = true;
    m->useRelativePosition = true;
    m->aaSeparationProfileSize = 11;
    m->train(context, trainingProteins, testingProteins, T("Training Model"));

    ProteinEvaluatorPtr evaluator = createProteinEvaluator();
    CompositeScoreObjectPtr scores = m->evaluate(context, testingProteins, evaluator, T("EvaluateTest"));
    return evaluator->getScoreToMinimize(scores);
  }

protected:
  friend class SimpleModelWorkUnitClass;

  File inputDirectory;
  File supervisionDirectory;

  ProteinEvaluatorPtr createProteinEvaluator() const
  {
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    evaluator->addEvaluator(ss3Target, containerSupervisedEvaluator(classificationEvaluator()), T("SS3-By-Protein"));
    evaluator->addEvaluator(ss3Target, elementContainerSupervisedEvaluator(classificationEvaluator()), T("SS3-By-Residue"));
    evaluator->addEvaluator(ss3Target, new SegmentOverlapEvaluator(secondaryStructureElementEnumeration), T("SS3-SegmentOverlap"), true);

    return evaluator;
  }

  bool loadProteins(ExecutionContext& context, ContainerPtr& trainingProteins, ContainerPtr& testingProteins) const
  {
    size_t numProteinsToLoad = 0;
#if JUCE_MAC && JUCE_DEBUG
    numProteinsToLoad = 10;
#endif

    if (supervisionDirectory.getChildFile(T("train/")).exists()
        && supervisionDirectory.getChildFile(T("test/")).exists())
    {
      context.informationCallback(T("Train/Test split detected !"));
      trainingProteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory.getChildFile(T("train/")), supervisionDirectory.getChildFile(T("train/")), numProteinsToLoad, T("Loading training proteins"));
      testingProteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory.getChildFile(T("test/")), supervisionDirectory.getChildFile(T("test/")), numProteinsToLoad, T("Loading testing proteins"));
    }
    else
    {
      context.informationCallback(T("No train/test split detected. Splitting to 4:1"));
      ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, numProteinsToLoad, T("Loading proteins"));
      trainingProteins = proteins->invFold(0, 5);
      testingProteins = proteins->fold(0, 5);
    }

    if (!trainingProteins || trainingProteins->getNumElements() == 0 ||
        !testingProteins || testingProteins->getNumElements() == 0)
    {
      context.errorCallback(T("No proteins found !"));
      return false;
    }
    return true;
  }
};

class TestSegmentOverlapWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    VectorPtr obs = createVectorFromString(T("CEEEEEEEEEEC"));
    VectorPtr pre = createVectorFromString(T("CCCEEEEEECCC"));

    SupervisedEvaluatorPtr f = new SegmentOverlapEvaluator(secondaryStructureElementEnumeration);
    ScoreObjectPtr so = f->createEmptyScoreObject(context, FunctionPtr());
    f->addPrediction(context, pre, obs, so);
    f->finalizeScoreObject(so, FunctionPtr());
    
    std::cout << so->toString() << std::endl;
    return so;
  }

  VectorPtr createVectorFromString(const String str) const
  {
    DoubleVectorPtr coil = new SparseDoubleVector(secondaryStructureElementEnumeration, probabilityType);
    coil->setElement(2, 1.f);
    DoubleVectorPtr helix = new SparseDoubleVector(secondaryStructureElementEnumeration, probabilityType);
    helix->setElement(0, 1.f);
    
    VectorPtr res = vector(doubleVectorClass(secondaryStructureElementEnumeration, probabilityType), str.length());
    for (size_t i = 0; i < (size_t)str.length(); ++i)
    {
      switch (str[i])
      {
        case T('C'):
          res->setElement(i, coil);
          break;
        case T('E'):
          res->setElement(i, helix);
          break;
        default:
          jassertfalse;
      }
    }
    return res;
  }
};

};

#endif // _PROTEINS_MODEL_BOX_H_
