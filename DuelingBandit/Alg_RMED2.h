// Implementation of RMED2 in "Regret Lower Bound and Optimal Algorithm in Dueling Bandit Problem"
#pragma once
#include "Alg.h"
class CAlg_RMED2 :
	public CAlg
{
public:
	CAlg_RMED2();
	~CAlg_RMED2();
private:
	double scale_factor; 
	int num_pairs;
	vector<vector<double>> counts;   // total number of comparison
	vector<vector<double>> wins; // statistics of comparison results
	vector<vector<double>> empi_beat_prob;
	vector<double> empi_div;

	vector<int> current_set;         // L_C in the paper
	int current_ind;                 // the index of the first candidate in L_C
	vector<int> remain_set;          // L_R 
	vector<int> next_set;            // L_N
public:
	void init(int num_slots, int num_arms, struConfig config);
	int pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair); // select the estimated best arm
private:
	bool find(vector<int> list, int target);
	void calEmpiBeatProb();
};

