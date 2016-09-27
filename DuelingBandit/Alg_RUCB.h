// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-08-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CAlg_RUCB.h : Relative Upper Confidence Bound algorithm
// Reference:   M. Zoghi, et al., "Relative Upper Confidence Bound for the K-Armed Dueling Bandit Problem", ICML'14
//

#pragma once
#include "Alg.h"
class CAlg_RUCB :
	public CAlg
{
public:
	CAlg_RUCB();
	~CAlg_RUCB();
private:
	double scale_factor;
	vector<vector<double>> counts;
	vector<vector<double>> wins;
	vector<int> hyp_best;
public:
	void init(int num_slots, int num_arms, struConfig config);
	int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair); // select the estimated best arm
private:
	void calRUCB(int ind_slot, vector<vector<double>>& ucb);
	bool find(vector<int> list, int target);
};

