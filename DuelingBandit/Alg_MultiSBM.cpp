#include "stdafx.h"
#include "Alg_MultiSBM.h"


CAlg_MultiSBM::CAlg_MultiSBM()
{
}


CAlg_MultiSBM::~CAlg_MultiSBM()
{
}

void CAlg_MultiSBM::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	cur_first = rand() % num_arms;
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		CSBM cur_sbm(num_arms, config.sbm_scale_factor);
		sbm.push_back(cur_sbm);
	}
}

int CAlg_MultiSBM::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	int ind_first, ind_second;
	cmpPair.clear();
	// first candidate
	ind_first = cur_first;

	// second candidate
	ind_second = sbm[ind_first].pull(ind_slot);

	// update 
	if (beat[ind_second][ind_first] == 1) {
		sbm[ind_first].update(ind_second, 1.0);
	}
	else {
		sbm[ind_first].update(ind_second, 0.0);
	}

	// set the next first candidate as the current second candidate
	cur_first = ind_second;

	// output the results
	cmpPair.push_back(ind_first);
	cmpPair.push_back(ind_second);
	if (beat[ind_first][ind_second] == 1) {
		return ind_first;
	}
	else {
		return ind_second;
	}
}

