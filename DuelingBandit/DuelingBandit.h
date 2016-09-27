// 
// Created by Huasen Wu (huasenwu@gmail.com), on 04-08-2016
// Copyright (c) 2016 Huasen Wu. All rights reserved.
//
// DuelingBandit.h : Header file for the simulations.
//

#ifndef _DUELINGBANDIT_H_
#define _DUELINGBANDIT_H_
#include <cmath>
#include <vector>
#include <stdlib.h>
#include <search.h>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm> 

using namespace std;

// constant
#define PI                        (double)3.1415926535897

// input method
#define INPUT_TYPE_RANDOM         (int)0
#define INPUT_TYPE_FILE           (int)1
#define INPUT_TYPE_CUSTOMIZED     (int)2

// Winner type
#define WINNER_TYPE_CONDORCET     (int)0
#define WINNER_TYPE_NON_CONDORCET (int)1

// regret type
#define REGRET_TYPE_ALL_CPLD_SCORE (int)0  
#define REGRET_TYPE_DIFF           (int)1  // use normalized Copeland score for non-Condorcet dueling bandits, use Yue's definition for Condorcet dueling bandits

// algorithm index
#define ALG_BTM                   (int)0
#define ALG_SAVAGE                (int)1
#define ALG_RUCB                  (int)2
#define ALG_RCS                   (int)3
#define ALG_MULTISBM              (int)4
#define ALG_SPARRING              (int)5
#define ALG_CCB                   (int)6
#define ALG_SCB                   (int)7
#define ALG_RMED1                 (int)8
#define ALG_RMED2                 (int)9
#define ALG_ECWRMED               (int)10
#define ALG_DTS                   (int)11
#define ALG_DTS2                  (int)12

// error code
#define ERROR_INPUT_TYPE          (int)1
#define ERROR_WINNER_TYPE         (int)2
#define ERROR_OPEN_INPUT_FILE     (int)3
#define ERROR_OPEN_OUPUT_FILE     (int)4
#define ERROR_INVALID_PREF_MAT    (int)5
#define ERROR_INVALID_DIST        (int)6
#define ERROR_INVALID_INDEX       (int)7

// 
struct struConfig
{
	int winner_type;         // definition of winner
	int regret_type;         // definition of regret

	double btm_gamma;         // the gamma parameter for the BTM algorithm
	double ucb_scale_factor;  // scale factor for UCB
	double klucb_ctrl_factor; // scale factor for KL-UCB
	double klucb_eps;         // accuracy requirement for KL-UCB
	double sbm_scale_factor;  // scale factor, alpha, for UCB in MultiSBM and Sparring
	double rmed2_alpha;       // alpha for RMED2
	double ecwrmed_alpha;     // alpha for ECW_RMED
	double ecwrmed_beta;      // beta for ECW_RMED
};


#endif