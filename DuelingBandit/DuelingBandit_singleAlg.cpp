// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-08-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// DuelingBandit.cpp : Main file for the simulations.
//

#include "stdafx.h"
#include <random>

#include "DuelingBandit.h"
#include "MAB_singleAlg.h"

void getPrefMat(int input_type, string inputFilePath, int& num_arms, vector<vector<double>>& pref_mat);

int main()
{
	srand((unsigned int)time(NULL));

	// Simulation setting
	// Algrithm Index
	int algInd = ALG_DTS2;

	// directory and file names
	// data set
	string strSimConfig = "MSLR_Informational_5_non_Condorcet_test";     
	// input directory
	string strInputDir = "E:\\DuelingBanditSim\\DuelingBandit\\data\\";
	// ouput directory
	string strOutputDir = "E:\\DuelingBanditSim\\DuelingBandit\\results\\";

	// simulation config
	int num_sims = 5;
	int num_slots = 10000000;

	// preference matrix
	int num_arms = 6;     // default number of arms
	int input_type = INPUT_TYPE_FILE;

	// Policy parameters
	struConfig config;
	config.regret_type = REGRET_TYPE_DIFF;
	config.btm_gamma = 1.3;
	config.ucb_scale_factor = 0.51;
	config.klucb_ctrl_factor = 3.0;
	config.klucb_eps = 0.000001;
	config.sbm_scale_factor = 3.0;
	config.rmed2_alpha = 3.0;
	config.ecwrmed_alpha = 3.0;
	config.ecwrmed_beta = 0.01;

	// input file	
	string inputFileName = strSimConfig + ".txt";
	string inputFilePath = strInputDir + inputFileName;	

	// output files
	ofstream fout_data, fout_results;
	string settingFile = strOutputDir + strSimConfig + "_results" + ".txt";
	string names = strOutputDir + strSimConfig + "_Alg_" + to_string(algInd) + "_results" + ".mat";
	fout_data.open(settingFile.c_str());
	fout_results.open(names.c_str());
	if (!fout_data.is_open() || !fout_results.is_open())
	{
		cout << "DuelingBandit.cpp: Failed to create the output file!" << endl;
		exit(ERROR_OPEN_OUPUT_FILE);
	}


	vector<vector<double>> pref_mat;
	getPrefMat(input_type, inputFilePath, num_arms, pref_mat);
	for (int i = 0; i < num_arms; i++) {
		for (int j = 0; j < num_arms; j++) {
			cout << pref_mat[i][j] << " ";
		}
		cout << endl;
	}

	
	// MAB model
	CMAB_singleAlg objMAB = CMAB_singleAlg(algInd);
	vector<double> total_reward(num_slots, 0.0);

	// Runsim
	for (int ind_sim = 0; ind_sim < num_sims; ind_sim++) {
		cout << "Sim # " << ind_sim << endl;
		//shuffle the pref_mat
		vector<int> ind_arms;
		for (int i = 0; i < num_arms; i++) {
			ind_arms.push_back(i);
		}
		random_shuffle(ind_arms.begin(), ind_arms.end());
		vector<vector<double>> cur_pref_mat;
		for (int i = 0; i < num_arms; i++) {
			vector<double> pref_vec;
			for (int j = 0; j < num_arms; j++) {
				pref_vec.push_back(pref_mat[ind_arms[i]][ind_arms[j]]);
			}
			cur_pref_mat.push_back(pref_vec);
		}
		for (int i = 0; i < num_arms; i++) {
			for (int j = 0; j < num_arms; j++) {
				cout << cur_pref_mat[i][j] << " ";
			}
			cout << endl;
		}
		vector<double> total_reward_temp = objMAB.singleRun(num_slots, cur_pref_mat, config);
		for (int ind_slot = 0; ind_slot < num_slots; ind_slot++) {
			total_reward[ind_slot] += total_reward_temp[ind_slot];
		}
	}
	for (int ind_slot = 0; ind_slot < num_slots; ind_slot++) {
		total_reward[ind_slot] /= num_sims;
		fout_results << ind_slot << " " << total_reward[ind_slot] << " " << endl;
	}

	
	fout_data.close();
	fout_results.close();

	system("PAUSE");
	return 0;
}

// generate or load the preference matrix
void getPrefMat(int input_type, string inputFilePath, int& num_arms, vector<vector<double>>& pref_mat) {
	pref_mat.clear();
	if (input_type == INPUT_TYPE_RANDOM) {
		for (int i = 0; i < num_arms; i++) {
			pref_mat.push_back(vector<double>(num_arms, 0.0));
		}
		for (int i = 0; i < num_arms; i++) {
			for (int j = 0; j <= i; j++) {
				if (j == i) {
					pref_mat[i][j] = 0.5;
				}
				else {
					pref_mat[i][j] = (double)rand() / RAND_MAX;
					pref_mat[j][i] = 1 - pref_mat[i][j];
				}

			}
		}
	}
	else if (input_type == INPUT_TYPE_FILE) {
		ifstream fin_data;
		fin_data.open(inputFilePath);
		if (!fin_data.is_open()) {
			cout << "DuelingBandit.cpp: Failed to create the input file!" << endl;
			exit(ERROR_OPEN_INPUT_FILE);
		}
		fin_data >> num_arms;
		for (int i = 0; i < num_arms; i++) {
			vector<double> pref_vec;
			for (int j = 0; j < num_arms; j++) {
				double pref_prob;
				fin_data >> pref_prob;
				pref_vec.push_back(pref_prob);
			}
			pref_mat.push_back(pref_vec);
		}
	}
	// For select the arms from the MSLR dataset
	else if (input_type == INPUT_TYPE_CUSTOMIZED) {
		ifstream fin_data;
		fin_data.open(inputFilePath);
		if (!fin_data.is_open()) {
			cout << "DuelingBandit.cpp: Failed to create the input file!" << endl;
			exit(ERROR_OPEN_INPUT_FILE);
		}
		vector<vector<double>> pref_mat_original;		
		for (int i = 0; i < num_arms; i++) {
			vector<double> pref_vec;
			for (int j = 0; j < num_arms; j++) {
				double pref_prob;
				fin_data >> pref_prob;
				pref_vec.push_back(pref_prob);
			}
			pref_mat_original.push_back(pref_vec);
		}
		for (int i = 3; i < num_arms - 8; i += 4) {
			vector<double> pref_vec;
			for (int j = 3; j < num_arms - 8; j += 4) {
				pref_vec.push_back(pref_mat_original[i][j]);
			}
			pref_mat.push_back(pref_vec);
		}
		num_arms = (int)pref_mat.size();
		cout << "num_arms " << num_arms << endl;
		vector<double> copeland_score;
		double max_score = -1.0;
		for (int ind_arm1 = 0; ind_arm1 < num_arms; ind_arm1++) {
			double num_wins = 0;
			for (int ind_arm2 = 0; ind_arm2 < num_arms; ind_arm2++) {
				if (ind_arm1 != ind_arm2 && pref_mat[ind_arm1][ind_arm2] > 0.5) {
					num_wins += 1.0;
				}
			}
			cout << num_wins / (num_arms - 1) << " ";
			if (num_wins / (num_arms - 1) > max_score) {
				max_score = num_wins / (num_arms - 1);
			}
		}
		cout << endl;
		cout << "Max Score " << max_score << endl;
	}
	else {
		cout << "DuelingBandit.cpp: Invalid input type!" << endl;
		exit(ERROR_INPUT_TYPE);
	}
}

