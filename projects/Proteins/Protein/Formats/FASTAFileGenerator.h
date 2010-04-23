/*-----------------------------------------.---------------------------------.
| Filename: FASTAFileGenerator.h           | FASTA File generator            |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 17:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_FORMATS_FASTA_FILE_GENERATOR_H_
# define LBCPP_PROTEINS_FORMATS_FASTA_FILE_GENERATOR_H_

# include "../Protein.h"

namespace lbcpp
{

class FASTAFileGenerator : public TextObjectPrinter
{
public:
  FASTAFileGenerator(const File& file)
    : TextObjectPrinter(file) {}

  virtual void consume(ObjectPtr object)
  {
    ProteinPtr protein = object.dynamicCast<Protein>();
    jassert(protein);
    print(T(">") + protein->getName(), true);
    LabelSequencePtr aminoAcidSequence = protein->getAminoAcidSequence();
    jassert(aminoAcidSequence);
    String aa = aminoAcidSequence->toString();
    jassert((size_t) aa.length() == aminoAcidSequence->size());
    print(aa, true);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_FORMATS_FASTA_FILE_GENERATOR_H_
