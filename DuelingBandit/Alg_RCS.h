// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-08-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CAlg_RCS.h : Relative Confidence Sampling
// Reference:   M. Zoghi, et al., "Relative confidence sampling for efficient on-line ranker evaluation", WSDM'14
//

#pragma once
#include "Alg.h"
#include "BetaDist.h"
class CAlg_RCS :
	public CAlg
{
public:
	CAlg_RCS();
	~CAlg_RCS();

private:
	double scale_factor;                 // scale factor for RUCB
	vector<int> champion_count;          // number of being chosen as the first candidate
	vector<vector<double>> counts;       // total number of comparison
	vector<vector<double>> wins;         // number of wins
	vector<double> rucb;                 // relative upper confidence bound, with respect to the first candidate
	vector<vector<double>> samples;      // the sample matrix

	mt19937 rndEngine;                   // random engine

public:
	// initialization
	void init(int num_slots, int num_arms, struConfig config);
	// choose the arms for comparison
	int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair); 

private:
	// calculate RUCB
	void calRUCB(int ind_slot, int ind_first);
	// draw samples from the posterior distribution
	void genSamples();                
};

