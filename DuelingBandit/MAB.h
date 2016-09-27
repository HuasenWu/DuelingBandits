// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-09-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CMAB.h : the K-Armed Dueling bandit model
// Note: we generate the same realization of comparison results for all algorithm
//       this will typically provide a more stable performance relationship
//

#pragma once
#include "DuelingBandit.h"
#include "Results.h"


class CMAB
{
public:
	CMAB();
	CMAB(bool flagEteOnly);
	~CMAB();
public:
	
	CResults singleRun(int num_slots, vector<vector<double>> pref_mat, struConfig config);

private:
	bool isEteOnly;    // Flag: only run the "explore-then-exploit" policies
	bool isLargeScale; // Flag: many arms or large time horizon; 
	                   //       in this case, we will ignore some not so good algorithms
};

