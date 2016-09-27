// 
// Created by Huasen Wu (huasenwu@gmail.com), on 05-03-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CAlg_Sparring.h : the Sparring algorithm
// Reference:   N. Ailon, et al., "Reducing Dueling Bandits to Cardinal Bandits", ICML'14
//

#pragma once
#include "Alg.h"

#include "SBM.h"

class CAlg_Sparring :
	public CAlg
{
public:
	CAlg_Sparring();
	~CAlg_Sparring();

public:
	void init(int num_slots, int num_arms, struConfig config);
	int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair); // select the estimated best arm

private:
	CSBM* sbm_first;
	CSBM* sbm_second;
};


