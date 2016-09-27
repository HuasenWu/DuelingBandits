// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-09-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CMAB_singleAlg.h : the K-Armed Dueling bandit model
// Note: Run for a single algorithm, to avoid "bad-allocation" error for many-arm or large-horizon scenarios
//

#pragma once
#include "DuelingBandit.h"
#include "Alg.h"
class CMAB_singleAlg
{
public:
	CMAB_singleAlg();
	CMAB_singleAlg(int algInd);
	~CMAB_singleAlg();
public:
	vector<double> singleRun(int num_slots, vector<vector<double>> pref_mat, struConfig config);
private:
	CAlg* objAlg;
};

