#include "stdafx.h"
#include "Alg_Sparring.h"


CAlg_Sparring::CAlg_Sparring()
{
}


CAlg_Sparring::~CAlg_Sparring()
{
	delete sbm_first, sbm_second;
}

void CAlg_Sparring::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	sbm_first = new CSBM(num_arms, config.sbm_scale_factor);
	sbm_second = new CSBM(num_arms, config.sbm_scale_factor);
}

int CAlg_Sparring::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	int ind_first, ind_second;
	cmpPair.clear();
	// first candidate
	ind_first = sbm_first->pull(ind_slot);

	// second candidate
	ind_second = sbm_second->pull(ind_slot);

	// update 
	if (beat[ind_first][ind_second] == 1) {
		sbm_first->update(ind_first, 1.0);
		sbm_second->update(ind_second, 0.0);
	}
	else {
		sbm_first->update(ind_first, 0.0);
		sbm_second->update(ind_second, 1.0);
	}

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