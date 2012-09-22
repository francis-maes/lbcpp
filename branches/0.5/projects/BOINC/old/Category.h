/*
 *  Category.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 20/10/10.
 *  Copyright 2010 Ulg. All rights reserved.
 *
 */

#ifndef CATEGORY_H
#define CATEGORY_H

#include <lbcpp/Data/RandomGenerator.h>
#include <vector>

using namespace lbcpp;
using namespace juce;
using namespace std;

class Category {
public: //TODO private
	String name;
	vector<String> vec;
	vector<double> probs;
public:
	Category(String name, XmlElement* category);
	String getRandomValue();
	String getRandomValue(double sum);

};

#endif
