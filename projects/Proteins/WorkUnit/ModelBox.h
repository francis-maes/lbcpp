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
    evaluator->addEvaluator(ss3Target, containerSupervisedEvaluator(classificationEvaluator()), T("SS3-SS8-StAl"), true);
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

};

#endif // _PROTEINS_MODEL_BOX_H_
