#include "stdafx.h"
#include "MAB_singleAlg.h"
#include "Alg_BTM.h"
#include "Alg_SAVAGE.h"
#include "Alg_RUCB.h"
#include "Alg_RCS.h"
#include "Alg_MultiSBM.h"
#include "Alg_Sparring.h"
#include "Alg_CCB.h"
#include "Alg_SCB.h"
#include "Alg_RMED.h"
#include "Alg_ECWRMED.h"
#include "Alg_DTS.h"
#include "Alg_DTS2.h"

CMAB_singleAlg::CMAB_singleAlg()
{
	objAlg = new CAlg_DTS();

}

CMAB_singleAlg::CMAB_singleAlg(int algInd) {
	switch (algInd) {
	case ALG_BTM:
		objAlg = new CAlg_BTM();
		break;
	case ALG_SAVAGE:
		objAlg = new CAlg_SAVAGE();
		break;
	case ALG_RUCB:
		objAlg = new CAlg_RUCB();
		break;
	case ALG_RCS:
		objAlg = new CAlg_RCS();
		break;
	case ALG_MULTISBM:
		objAlg = new CAlg_MultiSBM();
		break;
	case ALG_SPARRING:
		objAlg = new CAlg_Sparring();
		break;
	case ALG_CCB:
		objAlg = new CAlg_CCB();
		break;
	case ALG_SCB:
		objAlg = new CAlg_SCB();
		break;
	case ALG_RMED1:
		objAlg = new CAlg_RMED(ALG_RMED1);
		break;
	case ALG_RMED2:
		objAlg = new CAlg_RMED(ALG_RMED2);
		break;
	case ALG_ECWRMED:
		objAlg = new CAlg_ECWRMED();
		break;
	case ALG_DTS:
		objAlg = new CAlg_DTS();
		break;
	case ALG_DTS2:
		objAlg = new CAlg_DTS2();
		break;
	default:
		break;
	}
}


CMAB_singleAlg::~CMAB_singleAlg()
{
}

vector<double> CMAB_singleAlg::singleRun(int num_slots, vector<vector<double>> pref_mat, struConfig config) {
	vector<double> total_reward(num_slots, 0.0);
	int num_arms = (int)pref_mat.size();

	// check the preference matrix
	for (int i = 0; i < num_arms; i++) {
		for (int j = 0; j <= i; j++) {
			if (pref_mat[i][j] + pref_mat[j][i] != 1.0) {
				cout << "MAB.cpp: Invalid preference matrix! " << endl;
				exit(ERROR_INVALID_PREF_MAT);
			}
		}
	}

	vector<double> copeland_score;
	for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
		double num_wins = 0;
		for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
			if (ind_arm1 != ind_arm2 && pref_mat[ind_arm1][ind_arm2] > 0.5) {
				num_wins += 1.0;
			}
		}
		copeland_score.push_back(num_wins / (num_arms - 1));
	}
	int ind_best = 0;
	for (int ind_arm = 1; ind_arm < num_arms; ind_arm++) {
		if (copeland_score[ind_arm] > copeland_score[ind_best]) {
			ind_best = ind_arm;
		}
	}

	// set the winner type
	if (copeland_score[ind_best] < 1.0) {
		config.winner_type = WINNER_TYPE_NON_CONDORCET;
	}
	else {
		config.winner_type = WINNER_TYPE_CONDORCET;
	}

	// print the winner index and maximum score
	cout << "ind_winners ";
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		if (copeland_score[ind_arm] == copeland_score[ind_best]) {
			cout << ind_arm << " ";
		}
	}
	cout << endl;
	cout << "Max_score " << copeland_score[ind_best] << endl;

	// beating matrix
	vector<vector<int>> beat;
	vector<double> reward(num_arms, 0);
	for (int ind_arm = 0; ind_arm < num_arms; ind_arm++) {
		vector<int> cur_beat(num_arms, 0);
		beat.push_back(cur_beat);
	}

	// initialize algorithm
	objAlg->init(num_slots, num_arms, config);

	// implement simulations
	for (int ind_slot = 0; ind_slot < num_slots; ind_slot++) {
		if (ind_slot % 10000 == 0) {
			cout << "t = " << ind_slot << endl;
		}
		// generate reward 
		for (int ind1 = 0; ind1 < num_arms; ind1++) {
			reward[ind1] = copeland_score[ind1];
			for (int ind2 = 0; ind2 < ind1; ind2++) {
				double beat_prob = pref_mat[ind1][ind2];
				beat[ind1][ind2] = int(double(rand()) / RAND_MAX < beat_prob);
				beat[ind2][ind1] = 1 - beat[ind1][ind2];
			}
			beat[ind1][ind1] = int(double(rand()) / RAND_MAX < 0.5);
		}

		// Compare and update
		vector<int> cmp_pair;
		
		objAlg->pull(ind_slot, reward, beat, cmp_pair);
		if (ind_slot > 0) {
			total_reward[ind_slot] = total_reward[ind_slot - 1] + reward[cmp_pair[0]] + reward[cmp_pair[1]];
		}
		else {
			total_reward[ind_slot] = reward[cmp_pair[0]] + reward[cmp_pair[1]];
		}
	}

	return total_reward;
}
