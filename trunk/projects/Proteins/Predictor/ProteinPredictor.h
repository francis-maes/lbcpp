/*-----------------------------------------.---------------------------------.
| Filename: ProteinPredictor.h             | Protein Predictor               |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 14:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PREDICTOR_H_
# define LBCPP_PROTEIN_PREDICTOR_H_

# include "ProteinPredictorParameters.h"

namespace lbcpp
{

// protein, protein -> protein
class ProteinPredictor : public CompositeFunction
{
public:
  ProteinPredictor(ProteinPredictorParametersPtr parameters);
  ProteinPredictor()
    : activeGlobalPerception(false),
      activeResiduePerception(false),
      activeResiduePairPerception(false),
      activeDisulfideResiduePairPerception(false),
      activeDisulfideSymmetricResiduePairPerception(false),
      activeCysteinResiduePerception(false)
    {}

  void addTarget(ProteinTarget target);

  virtual void buildFunction(CompositeFunctionBuilder& builder);

  virtual bool loadFromXml(XmlImporter& importer);
  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  friend class ProteinPredictorClass;

  ProteinPredictorParametersPtr parameters;
  std::vector< std::pair<ProteinTarget, FunctionPtr> > targetPredictors;

  bool activeGlobalPerception;
  bool activeResiduePerception;
  bool activeResiduePairPerception;
  bool activeDisulfideResiduePairPerception;
  bool activeDisulfideSymmetricResiduePairPerception;
  bool activeCysteinResiduePerception;

  TypePtr residuePerceptionType;
  TypePtr residuePairPerceptionType;
};

typedef ReferenceCountedObjectPtr<ProteinPredictor> ProteinPredictorPtr;
  
// protein, protein -> protein
class ProteinSequentialPredictor : public CompositeFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  void addPredictor(ProteinPredictorPtr predictor)
    {jassert(predictor); predictors.push_back(predictor);}
  
  void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(proteinClass, T("input"));
    size_t supervision = builder.addInput(proteinClass, T("supervision"));
    
    size_t lastPredictor = input;
    for (size_t i = 0; i < predictors.size(); ++i)
      lastPredictor = builder.addFunction(predictors[i], lastPredictor, supervision, T("Stage ") + String((int)i));
  }
  
protected:
  friend class ProteinSequentialPredictorClass;
  
  std::vector<ProteinPredictorPtr> predictors;
};

typedef ReferenceCountedObjectPtr<ProteinSequentialPredictor> ProteinSequentialPredictorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PREDICTOR_H_
