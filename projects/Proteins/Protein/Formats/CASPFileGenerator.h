/*-----------------------------------------.---------------------------------.
| Filename: CASPFileGenerator.h            | CASP Prediction Files generator |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 17:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_CASP_FILE_GENERATOR_H_
# define LBCPP_PROTEIN_CASP_FILE_GENERATOR_H_

# include "../ProteinObject.h"

namespace lbcpp
{

class CASPFileGenerator : public TextObjectPrinter
{
public:
  CASPFileGenerator(const File& file, const String& method);

  virtual String getFormatSpecificationCode() const = 0;
  virtual void printPredictionData(ProteinObjectPtr protein) = 0;

  virtual void consume(ObjectPtr object);

protected:
  enum {maxColumns = 80};

  String method;

  void printRecord(const String& keyword, const String& data);
  void printMultiLineRecord(const String& keyword, const String& text);
  void printMultiLineRecordBase(const String& keyword, const String& text);
};

ObjectConsumerPtr caspTertiaryStructureFileGenerator(const File& file, const String& method);
ObjectConsumerPtr caspResidueResidueDistanceFileGenerator(const File& file, const String& method);
ObjectConsumerPtr caspOrderDisorderRegionFileGenerator(const File& file, const String& method);

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_CASP_FILE_GENERATOR_H_
