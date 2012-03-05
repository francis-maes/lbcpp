/*-----------------------------------------.---------------------------------.
| Filename: TemperatureEvolution.h         | TemperatureEvolution            |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 1, 2012  8:32:21 AM        |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_TEMPERATUREEVOLUTION_H_
# define LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_TEMPERATUREEVOLUTION_H_

namespace lbcpp
{

class TemperatureEvolution : public Object
{
public:
  TemperatureEvolution(double initialTemperature, double finalTemperature, size_t numIterations);

protected:
  friend class TemperatureEvolutionClass;

  double initialTemperature;
  double finalTemperature;
};

typedef ReferenceCountedObjectPtr<TemperatureEvolution> TemperatureEvolutionPtr;

}
; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_TEMPERATUREEVOLUTION_H_

