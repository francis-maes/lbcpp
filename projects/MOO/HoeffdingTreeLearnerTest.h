# include <oil/Execution/WorkUnit.h>
# include "HoeffdingTreeLearner.h"
# include <limits>

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

		for (size_t i = 0; i < nbSamples; i++) {
			double x1, x2, y;
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
			std::cout << "aabc\n";
			context.enterScope(string((double) i));
			std::cout << "defef\n";
			context.resultCallback("i", (double) i);
			std::cout << "agregeabc\n";
			context.resultCallback("att0", x1);
			context.resultCallback("att1", x2);
			context.resultCallback("targetValue", y);

			std::vector<float> samplex;
			samplex.push_back(x1);
			samplex.push_back(x2);
			samplex.push_back(y);

			LeafNode* leaf = htl.traverseSample(samplex);

			std::cout << "weight0\n";
			leaf->linearModel->numericalWeights;
			std::cout << "wwwwww\n";
			//std::cout << "yyyyyy"<<leaf->linearModel->numericalWeights.size()<<"\n";
			//context.resultCallback("weightAtt0", leaf->linearModel->numericalWeights[0]);
			//std::cout << "weight1\n";
			//context.resultCallback("weightAtt1", leaf->linearModel->numericalWeights[1]);
			//context.resultCallback("threshold", leaf->linearModel->threshold);
			if (i % 4 == 0)
				context.resultCallback("predictionError",
						0.03 * samplex[0] + 0.07 * samplex[1] + 2 - htl.predict(samplex));
			else if (i % 4 == 1)
				context.resultCallback("predictionError",
						-0.05 * samplex[0] - 0.02 * samplex[1] + 3 - htl.predict(samplex));
			else if (i % 4 == 2)
				context.resultCallback("predictionError",
						-0.02 * sample[0] + 0.3 * sample[1] - 1 - htl.predict(samplex));
			else
				context.resultCallback("predictionError",
						0.06 * sample[0] - 0.04 * sample[1] - 3 - htl.predict(samplex));

			// split quality bug
			std::vector<Split>* bestSplits = htl.findBestSplitPerAttribute(*leaf);
			context.resultCallback("splitQualityAtt0", (*bestSplits)[0].quality);
			context.resultCallback("splitQualityAtt1", (*bestSplits)[1].quality);
			double ratio;
			if ((*bestSplits)[0].quality > (*bestSplits)[1].quality && (*bestSplits)[0].quality != 0) {
				ratio = (*bestSplits)[1].quality / (*bestSplits)[0].quality;
			} else if ((*bestSplits)[1].quality > (*bestSplits)[0].quality && (*bestSplits)[1].quality != 0) {
				ratio = (*bestSplits)[0].quality / (*bestSplits)[1].quality;
			} else {
				ratio = 1;
			}

			context.resultCallback("splitQualityRatio", ratio);
			context.resultCallback("hoeffdingBound", 1 - MathUtils::hoeffdingBound(1, i, 0.1));
			context.resultCallback("splitValueAtt0", (*bestSplits)[0].value);
			context.resultCallback("splitValueAtt1", (*bestSplits)[1].value);
			context.resultCallback("Split?", htl.splitWasMade);
			context.resultCallback("SplitNbOfLeaves", (double) htl.nbOfLeavesSplit);

			EBST* att0Model = leaf->numericalAttributeObservations[0].model;
			EBST* att1Model = leaf->numericalAttributeObservations[0].model;

			htl.addTrainingSample(sample);

			cout << "eerste print: \n";
			htl.pprint();
			htl.pprint();

			context.leaveScope();
			cout << "tweede print: \n";
			htl.pprint();
			cout << "epsilon: " << std::numeric_limits<double>::epsilon() << endl;
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
