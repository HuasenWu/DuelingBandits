#include "stdafx.h"
#include "Alg_RCS.h"


CAlg_RCS::CAlg_RCS()
{
}


CAlg_RCS::~CAlg_RCS()
{
}

void CAlg_RCS::init(int num_slots, int num_arms, struConfig config) {
	this->num_slots = num_slots;
	this->num_arms = num_arms;
	scale_factor = config.ucb_scale_factor;

	champion_count.clear();
	counts.clear();
	wins.clear();
	rucb.clear();
	samples.clear();

	for (int i = 0; i < num_arms; i++) {
		champion_count.push_back(0);
		counts.push_back(vector<double>(num_arms, 0.000001));
		wins.push_back(vector<double>(num_arms, 0.00001));
		rucb.push_back(0.0);
		samples.push_back(vector<double>(num_arms, 0.5));
	}

	// random generator
	random_device rd;
	rndEngine = mt19937(rd());
}

int CAlg_RCS::pull(int ind_slot, vector<double> reward, vector<vector<int>> beat, vector<int>& cmpPair) {
	cmpPair.clear();

	// Find the first candidate, using TS	
	vector<int> win_count(num_arms, 0);
	// draw samples from the posterior distribution
	genSamples();
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		for (int ind_arm2 = 0; ind_arm2 < ind_arm1; ind_arm2++) {			
			double sample = samples[ind_arm1][ind_arm2];
			if (sample > 0.5) {
				win_count[ind_arm1]++;
			}
			else {
				win_count[ind_arm2]++;
			}
		}
	}
	int ind_first = argmax(win_count);
	// When no arm beats all others, choose the one that has been the champion least often
	if (win_count[ind_first] < num_arms - 1) {
		ind_first = 0;
		for (int ind_arm = 1; ind_arm < num_arms; ind_arm++) {
			if (champion_count[ind_arm] < champion_count[ind_first]) {
				ind_first = ind_arm;
			}
		}
	}
	champion_count[ind_first]++;

	// find the second candidate, using RUCB
	calRUCB(ind_slot, ind_first);
	int ind_second = argmax(rucb);

	counts[ind_first][ind_second] += 1.0;
	counts[ind_second][ind_first] += 1.0;
	wins[ind_first][ind_second] += beat[ind_first][ind_second];
	wins[ind_second][ind_first] += (1 - beat[ind_first][ind_second]);
	cmpPair.push_back(ind_first);
	cmpPair.push_back(ind_second);
	if (beat[ind_first][ind_second] == 1) {
		return ind_first;
	}
	else {
		return ind_second;
	}
}
void CAlg_RCS::calRUCB(int ind_slot, int ind_first) {
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		if (ind_arm != ind_first) {
			rucb[ind_arm] = wins[ind_arm][ind_first] / counts[ind_arm][ind_first]
				+ sqrt(scale_factor * log(ind_slot + 1.0) / counts[ind_arm][ind_first]);
		}
		else {
			rucb[ind_arm] = 0.5;
		}
	}
}

void CAlg_RCS::genSamples() {
	double gamma_scale = 2.0;
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		for (int ind_arm2 = 0; ind_arm2 <= ind_arm1; ind_arm2++) {
			if (ind_arm2 == ind_arm1) {
				samples[ind_arm1][ind_arm2] = 0.5;
			}
			else {
				gamma_distribution<> gamma1(wins[ind_arm1][ind_arm2] + 1.0, gamma_scale);
				gamma_distribution<> gamma2(wins[ind_arm2][ind_arm1] + 1.0, gamma_scale);
				double x = gamma1(rndEngine);
				double y = gamma2(rndEngine);
				if (x + y == 0) {
					samples[ind_arm1][ind_arm2] = 0.5;
				}
				else{
					samples[ind_arm1][ind_arm2] = x / (x + y);
				}
				samples[ind_arm2][ind_arm1] = 1 - samples[ind_arm1][ind_arm2];
			}
		}
	}
}

