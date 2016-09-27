// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-08-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CAlg_DTS.h : our Double Thompson Sampling algorithm
// Reference:   Huasen Wu, et al., "Double Thompson Sampling for Dueling Bandits"
//


#pragma once
#include "Alg.h"

#include "BetaDist.h"
class CAlg_DTS :
	public CAlg
{
public:
	CAlg_DTS();
	~CAlg_DTS();
private:
	int num_pairs;                      // number of pairs
	double scale_factor_RCB;                // scale factor for calculating RUCB/RLCB
	double ctrl_factor_KLRCB;                // scale factor for calculating KL-RUCB/RLCB
	double eps_KLRCB;                   // tolerant error for KL-RUCB/RLCB
	vector<vector<double>> counts;      // total number of comparison
	vector<vector<double>> wins;        // statistics of comparison results

	vector<vector<double>> rucb;        // relative upper confidence bound
	vector<vector<double>> rlcb;        // relative lower confidence bound
	vector<int> upper_Cpld;             // upper bound for the Copeland score
	vector<int> lower_Cpld;             // lower bound for the Copeland score
	vector<vector<double>> samples;     // the sample matrix, \theta in the paper

	mt19937 rndEngine;                   // random engine

public:
	void init(int num_slots, int num_arms, struConfig config);
	int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair); // select the estimated best arm
private:
	void calRCB(int ind_slot);
	void calKLRCB(int ind_slot);
	void genSamples();                // draw samples from the posterior distribution
	void genSamples(int ind_col);     // draw samples from the posterior distribution, for the "ind_col" column
};

