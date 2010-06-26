/*-----------------------------------------.---------------------------------.
| Filename: ResidueResidueDistanceCASPF...h| CASP Prediction Files generator |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 17:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_FORMATS_RESIDUE_RESIDUE_DISTANCE_CASP_FILE_GENERATOR_H_
# define LBCPP_PROTEINS_FORMATS_RESIDUE_RESIDUE_DISTANCE_CASP_FILE_GENERATOR_H_

# include "CASPFileGenerator.h"

namespace lbcpp
{

class ResidueResidueDistanceCASPFileGenerator : public CASPFileGenerator
{
public:
  ResidueResidueDistanceCASPFileGenerator(const File& file, const String& method)
    : CASPFileGenerator(file, method) {}

  virtual String getFormatSpecificationCode() const
    {return T("RR");}

  virtual void printPredictionData(ProteinObjectPtr protein)
  {
    size_t n = protein->getLength();

    LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
    printAminoAcidSequence(aminoAcidSequence);
    
    ScoreSymmetricMatrixPtr residueResidueContactMatrix8Cb = protein->getResidueResidueContactMatrix8Cb();
    jassert(residueResidueContactMatrix8Cb && residueResidueContactMatrix8Cb->getDimension() == n);
    size_t lineCount = 0;
    for (size_t i = 0; i < n; ++i)
    {
      for (size_t j = i + 6; j < n; ++j)
      {
        double probability = residueResidueContactMatrix8Cb->getScore(i, j);
        jassert(probability >= 0.0 && probability <= 1.0);
        print(lbcpp::toString(i + 1) + T(" ") + lbcpp::toString(j + 1) + T(" 0 8 ") + String(probability, 2), true);
        ++lineCount;
      }
    }
  }

  void printAminoAcidSequence(LabelSequencePtr aminoAcidSequence)
  {
    enum {numAminoAcidsPerLine = 50};

    size_t begin = 0;
    size_t length = aminoAcidSequence->size();
    while (begin < length)
    {
      size_t end = begin + numAminoAcidsPerLine;
      if (end > length)
        end = length;

      String line;
      for (size_t i = begin; i < end; ++i)
        line += aminoAcidSequence->getString(i);
      jassert(line.length() <= numAminoAcidsPerLine);
      print(line, true);

      begin = end;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_FORMATS_RESIDUE_RESIDUE_DISTANCE_CASP_FILE_GENERATOR_H_
