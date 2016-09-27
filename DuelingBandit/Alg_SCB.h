// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-09-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CAlg_SCB.h : Scalable Copeland Bandit algorithm
// Reference:   M. Zoghi, et al., "Copeland Dueling Bandits", NIPS'15
//

#pragma once
#include "Alg.h"
class CAlg_SCB :
	public CAlg
{
public:
	CAlg_SCB();
	~CAlg_SCB();

private:
	double eps_KLRCB;                      // precision requirement for KL-RUCB/RLCB
	double scale_factor_KLRCB;             // scale factor for KL-RUCB/RLCB, \alpha in the authors' code

	int ind_best;                          // estimate best arm

	bool duringPairExplore;                // flag: is during the pair explore, until we are confident about p_{ij} ? 1/2
	int ind_curFirst;                      // index of the current first arm to explore
	int ind_curSecond;                     // index of the current second arm to explore


	int roundInd;                          // round index, r
	double roundLength;                    // current round length, T, for win32 app, using int (or its variants) may cause overflow
	int curStep;                           // current step in the current round, t
	double failProb;                       // failure probability for current round
	vector<int> remainArms;                // remaining arms in the current round

	vector<vector<double>> counts;         // number of comparisons for each pair (actual comparisons)
	vector<vector<double>> wins;           // number of wins for each pair
	vector<vector<int>> valid_arms;        // arms that need to comare for each arm

	int virtual_running_index;                     // index of step in the virtual MAB
	vector<double> virtual_plays;          // number of plays in the virtual MAB
	vector<double> virtual_rewards;        // total reward in the virtual MAB
	vector<double> ucb;                   // KL-UCB for the virtual MAB
	vector<double> lcb;                   // KL-LCB for the virtual MAB


public:
	// initialization
	void init(int num_slots, int num_arms, struConfig config);
	// select the pair for comparison
	int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair); // select the estimated best arm

private:
	// reset for each round
	void reset();
	void updateVirtualMAB(int reward);
	bool checkConfidence(double num_wins, double num_plays);
	void calKLCB();
};

