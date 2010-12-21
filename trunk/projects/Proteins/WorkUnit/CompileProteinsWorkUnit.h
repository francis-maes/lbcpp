/*-----------------------------------------.---------------------------------.
 | Filename: CompileProteinsWorkUnit.h      | Merge proteins with pssms and   |
 | Author  : Julien Becker                  |  dssp files                     |
 | Started : 25/11/2010 11:28               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_COMPILE_PROTEINS_WORK_UNIT_H_
# define LBCPP_PROTEINS_COMPILE_PROTEINS_WORK_UNIT_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{
  
class CompileProteinsWorkUnit : public WorkUnit
{
public:
  CompileProteinsWorkUnit() : outputProteinFile(File::getCurrentWorkingDirectory()) {}
  
  virtual String toString() const
    {return T("Merge protein with PSSM and DSSP files");}
  
  virtual bool run(ExecutionContext& context);
  
protected:
  friend class CompileProteinsWorkUnitClass;
  
  File inputProteinFile;
  File pssmFile;
  File dsspFile;
  File outputProteinFile;
};

};


#endif //!LBCPP_PROTEINS_COMPILE_PROTEINS_WORK_UNIT_H_
