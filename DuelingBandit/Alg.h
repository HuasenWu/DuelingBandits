// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-08-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// CAlg.h: base class of the algorithms
//               

#pragma once
#include "DuelingBandit.h"
#include "BetaDist.h"
class CAlg
{
public:
	CAlg();
	~CAlg();

protected:
	int num_slots; // time horizon
	int num_arms;  // # of arms

protected:
	double klDiv(double p, double q); // KL-Divergence
public:
	virtual void  init(int num_slots, int num_arms, struConfig config);
	virtual int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair); // select the estimated best arm

	// normalized the probability
	void normalize(vector<double>& prob);

	// find the arguments of the maxima in a vector
	// the first argument
	template <typename T> int argmax(vector<T> vec) {
		if (vec.size() == 0) {
			return -1;
		}
		int len = (int)vec.size();
		int ind_max = 0;
		for (int i = 1; i < len; i++) {
			if (vec[i] > vec[ind_max]) {
				ind_max = i;
			}
		}
		return ind_max;
	};
	// all arguments
	template <typename T> vector<int> argmaxAll(vector<T> vec) {
		int max_val = 0;
		vector<int> max_ind;
		if (vec.empty()) {
			return max_ind;
		}
		int len = (int)vec.size();
		for (int i = 0; i < len; i++) {
			if (vec[i] > max_val) {
				max_val = vec[i];
			}
		}
		for (int i = 0; i < len; i++) {
			if (vec[i] == max_val) {
				max_ind.push_back(i);
			}
		}
		return max_ind;
	}
	// find the argument of the maxima in a given index set; break tie randomly
	template <typename T> int argmaxGivenIndSet(vector<T> vec, vector<int> ind_set) {
		if (ind_set.empty()) {
			return -1;
		}
		// find the maximum value
		int len = (int)ind_set.size();
		T max_val = vec[ind_set[0]];
		for (int i = 1; i < len; i++) {
			if (vec[ind_set[i]] > max_val) {
				max_val = vec[ind_set[i]];
			}
		}
		vector<int> max_ind;
		for (int i = 0; i < (int)ind_set.size(); i++) {
			if (vec[ind_set[i]] == max_val) {
				max_ind.push_back(ind_set[i]);
			}
		}
		return max_ind[rand() % (int)max_ind.size()];
	}

	// find the argument of the minima in a given index set; break tie randomly
	template <typename T> int argminGivenIndSet(vector<T> vec, vector<int> ind_set) {
		if (ind_set.empty()) {
			return -1;
		}
		// find the minimum value
		int len = (int)ind_set.size();
		T min_val = vec[ind_set[0]];
		for (int i = 1; i < len; i++) {
			if (vec[ind_set[i]] < min_val) {
				min_val = vec[ind_set[i]];
			}
		}
		vector<int> min_ind;
		for (int i = 0; i < (int)ind_set.size(); i++) {
			if (vec[ind_set[i]] == min_val) {
				min_ind.push_back(ind_set[i]);
			}
		}
		return min_ind[rand() % (int)min_ind.size()];
	}

	// search a value in a vector
	template <typename T> bool find(vector<T> vec, T target) {
		if (vec.empty()) {
			return false;
		}
		int len = (int)vec.size();
		for (int i = 0; i < len; i++) {
			if (target == vec[i]) {
				return true;
			}
		}
		return false;
	};


	// generate a random value with distribution "prob" over "val"
	template <typename T> int randomChoice(vector<T> val, vector<double> prob) {
		if (val.size() != prob.size()) {
			cout << "CAlg::randomChoice: Invalid distribution!" << endl;
			exit(ERROR_INVALID_DIST);
		}
		int len = (int)val.size();
		double rand_samp = ((double)rand()) / RAND_MAX;
		double cum_prob = 0.0;
		for (int i = 0; i < (int)prob.size(); i++) {
			cum_prob += prob[i];
			if (cum_prob >= rand_samp) {
				return val[i];
			}
		}
		return val[(int)val.size() - 1];
	};
	// choose a random value from 0, 1, ..., n, given distribution "prob"
	int randomChoice(int n, vector<double> prob) {
		double rand_samp = ((double)rand()) / RAND_MAX;
		double cum_prob = 0.0;
		for (int i = 0; i < (int)prob.size(); i++) {
			cum_prob += prob[i];
			if (cum_prob >= rand_samp) {
				return i;
			}
		}
		return n - 1;
	};
};



