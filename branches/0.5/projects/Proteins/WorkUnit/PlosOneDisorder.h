
#include <lbcpp/Core/Function.h>
# include "../Model/SimpleProteinModel.h"
# include "../../../src/Learning/Numerical/AddBiasLearnableFunction.h"

namespace lbcpp
{

class ExtraTreesDisorderPredictor : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ProteinPtr protein = Protein::createFromFile(context, inputFile);

    VectorPtr data = vector(pairClass(proteinClass, proteinClass), 1);
    data->setElement(0, new Pair(protein, ProteinPtr()));

    SimpleProteinModelPtr m = new SimpleProteinModel(drTarget);
    m->pssmWindowSize = 21;
    m->saSeparationProfileSize = 21;
    m->aaLocalHistogramSize = 60;
    m->ss3WindowSize = 11;

    m->learnerFunction = Function::createFromFile(context, context.getFile("x3Disorder.xml"));
    m->train(context, data, data, T("PreComputing ExtraTrees"));
    ProteinPtr prediction = m->compute(context, protein, protein).getObjectAndCast<Protein>();

    exportResult(context, prediction);
    return true;
  }

protected:
  friend class ExtraTreesDisorderPredictorClass;

  File inputFile;
  File outputFile;

private:
  void exportResult(ExecutionContext& context, const ProteinPtr& protein) const
  {
    if (outputFile.exists())
      outputFile.deleteFile();
    
    OutputStream* o = outputFile.createOutputStream();
    *o << "#------------------------------------------#\n";
    *o << "|                x3Disorder                |-------------------------#\n";
    *o << "#------------------------------------------#          Julien Becker  |\n";
    *o << "                                      |                Francis Maes  |\n";
    *o << "      Disorder State Probability      |              Louis Wehenkel  |\n";
    *o << "                                      #------------------------------#\n";
    *o << "\n";
    *o << "\n";
    *o << "\n";

    String ps = protein->getPrimaryStructure()->toString();
    String formatedPs;
    for (size_t i = 0; i < (size_t)ps.length(); ++i)
    {
      if (i % 50 == 0)
        formatedPs += T("\n     ");
      else if (i % 5 == 0)
        formatedPs += T(" ");
      formatedPs += ps[i];
    }

    *o << "#- Query ------------------------------------------------------------#\n";
           
    *o << "Sequence : " << formatedPs << "\n";
    *o << "\n";
    *o << "\n";
    
    *o << "#- Disorder State Probabilities ------ (Index, State, Probability) --#\n";

    const DenseDoubleVectorPtr dr = protein->getDisorderRegions();
    const size_t n = protein->getLength();
    for (size_t i = 0; i < n; ++i)
    {
      *o << toFixedLengthStringLeftJustified(String((int)i + 1), 4) << " ";
      *o << (dr->getValue(i) > 0.5 ? 'D' : 'O') << " ";
      *o << toFixedLengthStringLeftJustified(String(dr->getValue(i), 5), 7);
      *o << "\n";
    }

    delete o;
  }
  
  String toFixedLengthStringLeftJustified(const String& str, int length) const
  {
    jassert(str.length() <= length);
    String res = str;
    while (res.length() < length)
      res += T(" ");
    return res;
  }
};

class ExtraTreesDisorderLearner : public WorkUnit
{
public:
  Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, 10, T("Loading proteins"));
    
    SimpleProteinModelPtr m = new SimpleProteinModel(drTarget);
    m->pssmWindowSize = 21;
    m->saSeparationProfileSize = 21;
    m->aaLocalHistogramSize = 60;
    m->ss3WindowSize = 11;

    m->x3Trees = 20;
    m->x3Attributes = 0;
    m->x3Splits = 1;
    m->x3LowMemory = true;

    {
      const double bias = -0.0706; // Computed on Disorder723
      FunctionPtr biasFunction = addBiasLearnableFunction(binaryClassificationSensitivityAndSpecificityScore, bias, false);
      biasFunction->setBatchLearner(BatchLearnerPtr());
      
      FunctionPtr x3Function = binaryClassificationExtraTree(m->x3Trees, m->x3Attributes, m->x3Splits, m->x3LowMemory, context.getFile(T("x3Disorder")));    

      FunctionPtr compose = composeFunction(biasFunction, signedScalarToProbabilityFunction());
      compose->setBatchLearner(BatchLearnerPtr());
      m->learnerFunction = new PreProcessCompositeFunction(x3Function, compose);
    }

    m->train(context, proteins, ContainerPtr(), T("Training Model"));
    m->learnerFunction->saveToFile(context, context.getFile(T("x3Disorder.xml")));

    return true;
  }

protected:
  friend class ExtraTreesDisorderLearnerClass;

  File inputDirectory;
  File supervisionDirectory; 
};

}

