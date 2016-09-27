// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-08-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CAlg_RMED.h : Relative Minimum Empirical Divergence (RMED) algorithm
// Reference:    J. Komiyama, et al., "Regret Lower Bound and Optimal Algorithm in Dueling Bandit Problem", COLT'15
//


#pragma once
#include "Alg.h"
class CAlg_RMED :
	public CAlg
{
public:
	CAlg_RMED();
	CAlg_RMED(int Ind);
	~CAlg_RMED();
private:
	int indAlg;                      // algorithm name
	double scale_factor;             // scale factor (alpha) for RMED2
	int num_pairs;
	vector<vector<double>> counts;   // total number of comparison
	vector<vector<double>> wins;     // number of wins
	vector<vector<double>> empi_beat_prob; // empirical preference probability
	vector<double> empi_div;         // empirical divergence

	vector<int> current_set;         // L_C in the paper
	int current_ind;                 // the index of the first candidate in L_C
	vector<int> remain_set;          // L_R 
	vector<int> next_set;            // L_N
public:
	// initialization
	void init(int num_slots, int num_arms, struConfig config);   
	// select the arms
	int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair); 
private:	
	// calculate the empirical beating probability
	void calEmpiBeatProb();
	// find the minimum empirical divergence
	double minEmpiDiv(int& ind_min_div);
	// find the minimum ratio (for RMED2, \hat{b}^* in the paper)
	double minRatio(int ind_first, int ind_min_div, int& ind_min_ratio);
};

