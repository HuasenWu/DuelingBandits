// 
// Created by Huasen Wu (huasenwu@gmail.com), on 05-06-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CAlg_ECW_RMED.h : Efficient Copeland Winner - Relative Minimum Empirical Divergence (ECW-RMED) algorithm
// Reference:    J. Komiyama, et al., "Copeland Dueling Bandit Problem: Regret Lower Bound, Optimal Algorithm, and Computationally Efficient Algorithm", ICML'16
//

#pragma once
#include <algorithm>
#include "Alg.h"
class CAlg_ECWRMED :
	public CAlg
{
public:
	CAlg_ECWRMED();
	~CAlg_ECWRMED();

public:
	// initialization
	void init(int num_slots, int num_arms, struConfig config);
	// select the arms
	int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair);

private:
	vector<int> sort_indices(const vector<double> &val) {
		// sort indexes based on comparing values in val
		vector<int> idx((int)val.size());
		for (size_t i = 0; i < val.size(); i++) {
			idx[i] = i;
		}
		sort(idx.begin(), idx.end(),
			[&val](size_t i1, size_t i2) {return val[i1] < val[i2]; });
		return idx;
	};
	// update the comparison statistics
	void update(int ind_first, int ind_second, int firstBeatsSec);

	// find copeland winners and update the superior information
	vector<int> findCpldWinnersAndUpdateL(vector<vector<double>> pref_mat);  

	// check confidence
	int checkConfidence(int ind_slot, vector<int> candidates);

	// get the subset
	vector<vector<int>> getSubset(vector<int> set, int size);

	// find the pair with minimum empirical regret
	int findMinRegret(vector<vector<double>> pref_mat, vector<vector<double>>& argmin_q);
private:
	int num_pairs;                          // number of pairs
	double alpha;                           // parameter alpha
	double beta;                            // parameter beta, usually very small

	vector<vector<double>> counts;          // total number of comparisons
	vector<vector<double>> wins;            // number of wins
	vector<int> min_num_superiors;          // the first two smallest number of superiors
	vector<vector<double>> empi_beat_prob;  // empirical preference probability

	vector<int> current_set;         // L_C in the paper
	int current_ind;                 // the index of the candidate pair in L_C
	vector<int> remain_set;          // L_R 
	vector<int> next_set;            // L_N
};

