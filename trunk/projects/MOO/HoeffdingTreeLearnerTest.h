# include <oil/Execution/WorkUnit.h>
# include "HoeffdingTreeLearner.h"
# include <stdlib.h> // abs

namespace lbcpp {

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class HoeffdingTreeLearnerTest: public WorkUnit {
public:
	int randomSeed;
	int nbSamples;

	HoeffdingTreeLearnerTest() :
			randomSeed(0), nbSamples(80) {
	}

public:
	virtual ObjectPtr run(ExecutionContext& context) {
		lbCppMLLibraryCacheTypes(context);
		//pick a random seed
		context.getRandomGenerator()->setSeed(randomSeed);

		// setup htl learner
		DataDefinition* dataDef = new DataDefinition();
		dataDef->addNumericalAttribute("numAtt0");
		dataDef->addNumericalAttribute("numAtt1");
		dataDef->addTargetAttribute("targetValue");
		HoeffdingTreeLearner htl = HoeffdingTreeLearner(context, 0.01, *dataDef);

		context.enterScope("HoeffdingTreeTest");
		context.enterScope("testFunctions");

		double x1, x2, y;
		for (size_t i = 0; i < nbSamples; i++) {
			std::vector<float> sample;
			if (i % 4 == 0) {
				x1 = MathUtils::randDouble() / 2;
				x2 = MathUtils::randDouble() / 10 * 3;
				sample.push_back(x1);
				sample.push_back(x2);
				y = 0.03 * sample[0] + 0.07 * sample[1] + 2;
				sample.push_back(y);
			} else if (i % 4 == 1) {
				x1 = MathUtils::randDouble() / 2;
				x2 = MathUtils::randDouble() / 10 * 7 + 3.0 / 10;
				sample.push_back(x1);
				sample.push_back(x2);
				y = -0.05 * sample[0] - 0.02 * sample[1] + 3;
				sample.push_back(y);
			} else if (i % 4 == 2) {
				x1 = MathUtils::randDouble() / 2 + 0.5;
				x2 = MathUtils::randDouble() / 10 * 6;
				sample.push_back(x1);
				sample.push_back(x2);
				y = -0.02 * sample[0] + 0.3 * sample[1] - 1;
				sample.push_back(y);
			} else {
				x1 = MathUtils::randDouble() / 2 + 0.5;
				x2 = MathUtils::randDouble() / 10 * 4 + 6.0 / 10;
				sample.push_back(x1);
				sample.push_back(x2);
				y = 0.06 * sample[0] - 0.04 * sample[1] - 3;
				sample.push_back(y);
			}
			std::cout << "sample" << i << " =" << x1 << ", " << x2 << " , " << y << "\n";
			context.enterScope(string((double) i));
			context.resultCallback("i", (double) i);

			std::vector<float> samplex;
			samplex.push_back(x1);
			samplex.push_back(x2);
			samplex.push_back(y);

			LeafNode* leaf = htl.traverseSample(samplex);

			EBST* att0Model = leaf->numericalAttributeObservations[0].model;
			EBST* att1Model = leaf->numericalAttributeObservations[0].model;

			htl.addTrainingSample(sample);

			context.resultCallback("prediction",htl.predict(samplex));
			if (i % 4 == 0)
				context.resultCallback("predictionError",
						abs(y - htl.predict(samplex))/y);
			else if (i % 4 == 1)
				context.resultCallback("predictionError",
						abs(y - htl.predict(samplex))/y);
			else if (i % 4 == 2)
				context.resultCallback("predictionError",
						abs(y - htl.predict(samplex))/y);
			else
				context.resultCallback("predictionError",
						abs(y - htl.predict(samplex))/y);

			context.leaveScope();
		}

		context.leaveScope();
		context.leaveScope();

		/*cout << "Result:\n";
		 std::vector<float> sample;
		 sample.push_back(MathUtils::randDouble()/2);
		 sample.push_back(MathUtils::randDouble()/2);
		 LeafNode* leaf = htl.traverseSample(sample);
		 cout << "w0: " << leaf->linearModel->numericalWeights[0] << "\n";
		 cout << "w1: " << leaf->linearModel->numericalWeights[1] << "\n";
		 cout << "t: " << leaf->linearModel->threshold << "\n";*/
		std::cout << "succesfull end \n";

		return new Boolean(true);
	}

};

}
;
/* namespace lbcpp */
