// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-08-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CAlg_BTM.h : Beat The Mean (BTM) algorithm
// Reference:   Y. Yue, et al., "Beat the Mean Bandit", ICML'11
//

#pragma once
#include "Alg.h"
class CAlg_BTM :
	public CAlg
{
public:
	CAlg_BTM();
	~CAlg_BTM();

private:
	double gamma;                     // the factor for the relaxed stochastic transitivity 
	vector<vector<int>> counts;       // number of comparisons
	vector<vector<int>> wins;         // number of wins
	vector<int> active_set;           // active set
	vector<int> counts_active;        // number of comparisons within the active set
	vector<int> wins_active;          // number of wins within the active set
	vector<double> empi_beat_prob;    // empirical winning probability within the active set

public:
	// initialization
	void init(int num_slots, int num_arms, struConfig config);
	// choose the arms for comparison
	int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair); // select the estimated best arm

};

