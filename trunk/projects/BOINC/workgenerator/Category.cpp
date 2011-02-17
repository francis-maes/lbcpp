/*
 *  Category.cpp
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 20/10/10.
 *  Copyright 2010 Ulg. All rights reserved.
 *
 */

#include "Category.h"
#include <lbcpp/common.h>
Category::Category(String name, XmlElement* category) : name(name) {
	int nb = category->getNumChildElements();
	vec.reserve(nb);
	probs.reserve(nb);
	int i = 0;
	if (category->getFirstChildElement()->hasAttribute(String("prob"))) {
		forEachXmlChildElementWithTagName(*category, enumValue, String("enum")) {
			probs.push_back(enumValue->getDoubleAttribute(String("prob")));
			vec.push_back(enumValue->getStringAttribute(String("value")));
		}
	} else {
		float prob = 1.0/nb;
		forEachXmlChildElementWithTagName(*category, enumValue, String("enum")) {
			probs.push_back(prob);
			vec.push_back(enumValue->getStringAttribute(String("value")));
		}
	}
}

String Category::getRandomValue() {
	int i = RandomGenerator::getInstance()->sampleWithNormalizedProbabilities(probs);
	return vec.at(i);
}

String Category::getRandomValue(double sum) {
	int i = RandomGenerator::getInstance()->sampleWithProbabilities(probs, sum);
	return vec.at(i);
}
