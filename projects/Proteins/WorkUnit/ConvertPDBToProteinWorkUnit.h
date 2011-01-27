/*-----------------------------------------.---------------------------------.
 | Filename: ConvertPDBToProteinWork...h    | Convert PDB To Protein          |
 | Author  : Julien Becker                  | WorkUnit                        |
 | Started : 25/11/2010 11:58               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_CONVERT_PDB_TO_PROTEIN_WORK_UNIT_H_
# define LBCPP_PROTEINS_CONVERT_PDB_TO_PROTEIN_WORK_UNIT_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{
  
class ConvertPDBToProteinWorkUnit : public WorkUnit
{
public:
  ConvertPDBToProteinWorkUnit() : outputFile(File::getCurrentWorkingDirectory().getChildFile(T("output"))) {}
  
  virtual String toString() const
    {return T("Convert a PDB file to a Protein file and vice-versa.");}
  
  virtual Variable run(ExecutionContext& context);
  
protected:
  friend class ConvertPDBToProteinWorkUnitClass;
  
  File inputFile;
  File outputFile;
};
  
};

#endif //!LBCPP_PROTEINS_CONVERT_PDB_TO_PROTEIN_WORK_UNIT_H_
