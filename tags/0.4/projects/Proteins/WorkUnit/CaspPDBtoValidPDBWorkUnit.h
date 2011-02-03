/*-----------------------------------------.---------------------------------.
 | Filename: CaspPDBtoValidPDBWorkUnit.h    | CaspPDBtoValidPDBWorkUnit       |
 | Author  : Julien Becker                  |                                 |
 | Started : 25/11/2010 11:01               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_CASP_PDB_TO_VALID_PDB_WORK_UNIT_H_
# define LBCPP_PROTEINS_CASP_PDB_TO_VALID_PDB_WORK_UNIT_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{
  
class CaspPDBtoValidPDBWorkUnit : public WorkUnit
{
public:
  CaspPDBtoValidPDBWorkUnit() : outputDirectory(File::getCurrentWorkingDirectory()) {}
  
  virtual String toString() const
    {return T("This program add some lines of header to CASP PDB file. \
              Those lines are necessary if you want to convert a PDB into a XML.");}
  
  virtual Variable run(ExecutionContext& context);
  
protected:
  friend class CaspPDBtoValidPDBWorkUnitClass;
  
  File pdbDirectory;
  File fastaFile;
  File outputDirectory;
};
  
};

#endif //!LBCPP_PROTEINS_CASP_PDB_TO_VALID_PDB_WORK_UNIT_H_
