// 
// Created by Huasen Wu (huasenwu@gmail.com), on 05-03-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// SBM.h : the Singleton Bandit Machine used by the MultiSBM and Sparring algorithms
// Reference:   N. Ailon, et al., "Reducing Dueling Bandits to Cardinal Bandits", ICML'14
//

#pragma once
#include <vector>
#include <cmath>
using namespace std;
class CSBM {
public:
	CSBM(int num_arms, double alpha);
	~CSBM();
public:
	void update(int ind_arm, double reward);
	int pull(int ind_slot);
private:
	double alpha;
	int num_arms;
	vector<double> counts;
	vector<double> cum_rewards;
};
