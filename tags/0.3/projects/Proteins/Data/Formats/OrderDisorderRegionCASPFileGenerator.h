/*-----------------------------------------.---------------------------------.
| Filename: OrderDisorderRegionCASPFile...h| CASP Prediction Files generator |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 17:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_FORMATS_ORDER_DISORDER_REGION_CASP_FILE_GENERATOR_H_
# define LBCPP_PROTEINS_FORMATS_ORDER_DISORDER_REGION_CASP_FILE_GENERATOR_H_

# include "CASPFileGenerator.h"

namespace lbcpp
{

class OrderDisorderRegionCASPFileGenerator : public CASPFileGenerator
{
public:
  OrderDisorderRegionCASPFileGenerator(const File& file, const String& method)
    : CASPFileGenerator(file, method) {}

  virtual String getFormatSpecificationCode() const
    {return T("DR");}

  virtual void printPredictionData(ProteinPtr protein)
  {
    size_t n = protein->getLength();

    VectorPtr primaryStructure = protein->getPrimaryStructure();
    VectorPtr disorderRegions = protein->getDisorderRegions();
    jassert(disorderRegions);
    
    for (size_t i = 0; i < n; ++i)
    {
      double disorderProbability = disorderRegions->getVariable(i).getDouble();
      print(String(AminoAcid::toOneLetterCode((AminoAcidType)primaryStructure->getVariable(i).getInteger())) +
        T(" ") + (disorderProbability > 0.5 ? T("D") : T("O")) + T(" ") + 
        String(disorderProbability, 2), true);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_FORMATS_ORDER_DISORDER_REGION_CASP_FILE_GENERATOR_H_
