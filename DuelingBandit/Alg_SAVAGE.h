// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-08-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CAlg_SAVAGE.h : Sensitivity Analysis of VAriables for Generic Exploration
// Reference:    T. Urvoy, et al., "Generic Exploration and K-armed Voting Bandits", ICML'13
//

#pragma once
#include "Alg.h"
class CAlg_SAVAGE :
	public CAlg
{
public:
	CAlg_SAVAGE();
	~CAlg_SAVAGE();
private:
	int winner_type;                     // winner type
	vector<int> explore_set;             // the pairs to explore
	int ind_best;                        // estimate best arm
	vector<vector<double>> wins;         // number of wins
	vector<vector<double>> counts;       // number of comparisons
	vector<vector<double>> rucb;         // relative confidence bound
	vector<vector<double>> rlcb;         // relative confidence bound, for NON-condorcet winners
	vector<int> upper_cpld;              // upper bound for the Copeland score
	vector<int> lower_cpld;              // lower bound for the Copeland score

public:
	// initialization
	void init(int num_slots, int num_arms, struConfig config);
	// select the pair for comparison
	int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair); // select the estimated best arm

private:
	// independent test
	void independentTest();
	// find the arguments of minima for a given ind_set
	int argminGivenIndSet(vector<vector<double>> val, vector<int> ind_set);  // the ind_pair in the index set is given by (ind_arm1 * num_arms + ind_arm2)
	// calculate RCB  
	void calRCB();                                  // not used here
	void updateRCB(int ind_first, int ind_second);  // the confidence bounds only depends on the number of counts, do not need to update every slot
};

