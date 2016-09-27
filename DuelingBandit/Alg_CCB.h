// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-08-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CAlg_CCB.h : Copeland Confidence Bound algorithm
// Reference:   M. Zoghi, et al., "Copeland Dueling Bandits", NIPS'15
//

#pragma once
#include "Alg.h"
class CAlg_CCB :
	public CAlg
{
public:
	CAlg_CCB();
	~CAlg_CCB();

private:
	double scale_factor;
	vector<vector<double>> counts;         // number of comparisons for each pair
	vector<vector<double>> wins;           // number of wins for each pair
	vector<int> potential_best;            // potential best arms
	vector<vector<int>> potential_challengers; // potential challengers
	vector<int> upper_Cpld;                // upper bound for the Copeland score
	vector<int> lower_Cpld;                // lower bound for the Copeland score
	int max_loss;                          // max losses of a Copeland winner

public:
	// initialization
	void init(int num_slots, int num_arms, struConfig config);
	// select the pair for comparison
	int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair); // select the estimated best arm

private:
	void resetHypotheses();
	bool checkHypotheses(vector<vector<double>> rlcb);
	void calRCB(int ind_slot, int ind_beaten, vector<double>& ucb, vector<double>& lcb);
	bool isOverlapped(vector<int> vec1, vector<int> vec2);
	vector<int> getIntersection(vector<int> vec1, vector<int> vec2);
};

