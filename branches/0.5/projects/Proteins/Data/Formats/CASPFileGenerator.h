/*-----------------------------------------.---------------------------------.
| Filename: CASPFileGenerator.h            | CASP Prediction Files generator |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 17:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_CASP_FILE_GENERATOR_H_
# define LBCPP_PROTEIN_CASP_FILE_GENERATOR_H_

# include "../Protein.h"
# include <lbcpp/Data/Consumer.h>

namespace lbcpp
{

class CASPFileGenerator : public TextPrinter
{
public:
  CASPFileGenerator(ExecutionContext& context, const File& file, const String& method);

  virtual String getFormatSpecificationCode() const = 0;
  virtual void printPredictionData(ProteinPtr protein) = 0;

  virtual void consume(ExecutionContext& context, const Variable& variable);

protected:
  enum {maxColumns = 80};

  String method;

  void printRecord(const String& keyword, const String& data);
  void printMultiLineRecord(const String& keyword, const String& text);
  void printMultiLineRecordBase(const String& keyword, const String& text);
};

ConsumerPtr caspTertiaryStructureFileGenerator(ExecutionContext& context, const File& file, const String& method);
ConsumerPtr caspResidueResidueDistanceFileGenerator(ExecutionContext& context, const File& file, const String& method);
ConsumerPtr caspOrderDisorderRegionFileGenerator(ExecutionContext& context, const File& file, const String& method);

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_CASP_FILE_GENERATOR_H_
