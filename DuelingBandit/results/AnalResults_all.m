% Analysis results
%% Initialization
clear; close all; clc
Colors = { [.05 0.05 0.85], [.15 .6 .35],[.95 .84 0.05],  'c', ...
    [.8 .6 0.1], [.85 .7 0.9], ... 
    [.05 0.85  0.05], [.75 .5 .4],'m', [.87 .49 .0], 'r', [0.3, 0.7, 0.8]};
% setting
numShows = 1000;
numMarked = 20;

% Load Training Data
fprintf('Loading and Visualizing Data ...\n')
data_set = 'MSLR_Informational_5_Condorcet';
results = load ([data_set, '_results.mat'], '-ascii');
nSlots = results(:, 1);
rewards.opt = results(:, 2);
rewards.btm = results(:, 3);
rewards.savage = results(:, 4);
rewards.rucb = results(:, 5);
rewards.rcs = results(:, 6);
rewards.multisbm = results(:, 7);
rewards.sparring = results(:, 8);
rewards.ccb = results(:, 9);
rewards.scb = results(:, 10);
rewards.rmed1 = results(:, 11);
rewards.rmed2 = results(:, 12);
rewards.ecw_rmed = results(:, 13);
rewards.dts = results(:, 14);
rewards.dts2 = results(:, 15);

maxShow = nSlots(end);
indShow = 0 : log(maxShow) / numShows : log(maxShow);
indShow = ceil(exp(indShow));
indMarked = indShow(1 : length(indShow) / numMarked : end);




% if BTM/SAVAGE are run seperately
results_EteOnly = load ([data_set, '_results_EteOnly.mat'], '-ascii');
nSlots_EteOnly = [results_EteOnly(:, 1); nSlots(end)];
rewards_EteOnly.opt = [results_EteOnly(:, 2); rewards.opt(end)];
rewards_EteOnly.btm = [results_EteOnly(:, 3); rewards.btm(end)];
rewards_EteOnly.savage = [results_EteOnly(:, 4); rewards.savage(end)];

figure(3)

%BTM
semilogx(nSlots_EteOnly, (rewards_EteOnly.opt-rewards_EteOnly.btm),'color',Colors{1}, ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
hold on;
semilogx(nSlots_EteOnly, (rewards_EteOnly.opt-rewards_EteOnly.btm),'color', Colors{1}, 'LineStyle', 'none', 'marker', 'x', ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots_EteOnly(end), (rewards_EteOnly.opt(end)-rewards_EteOnly.btm(end)),'color', Colors{1}, 'marker', 'x',...
    'LineWidth', 1.5);

%SAVAGE
semilogx(nSlots_EteOnly, (rewards_EteOnly.opt -rewards_EteOnly.savage), 'color', Colors{2},  ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots_EteOnly, (rewards_EteOnly.opt -rewards_EteOnly.savage), 'color', Colors{2}, 'LineStyle', 'none','marker', 'p', ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots_EteOnly(end), (rewards_EteOnly.opt(end) -rewards_EteOnly.savage(end)), 'color', Colors{2}, 'marker', 'p', ...
    'LineWidth', 1.5);

% RUCB
semilogx(nSlots(indShow), (rewards.opt(indShow)-rewards.rucb(indShow)),'color', Colors{3},...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(indMarked), (rewards.opt(indMarked)-rewards.rucb(indMarked)),'color', Colors{3}, 'LineStyle', 'none','marker', '^', ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(end), (rewards.opt(end)-rewards.rucb(end)),'color', Colors{3}, 'marker', '^', ...
    'LineWidth', 1.5);

% RCS
semilogx(nSlots(indShow), (rewards.opt(indShow)-rewards.rcs(indShow)),'color', Colors{4},...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(indMarked), (rewards.opt(indMarked)-rewards.rcs(indMarked)),'color', Colors{4}, 'LineStyle', 'none','marker', 'v', ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(end), (rewards.opt(end)-rewards.rcs(end)),'color', Colors{4}, 'marker', 'v', ...
    'LineWidth', 1.5);

% % MultiSBM
% semilogx(nSlots(indShow), (rewards.opt(indShow)-rewards.multisbm(indShow)),'color', Colors{5},...
%     'LineWidth', 1.5, 'HandleVisibility', 'off');
% semilogx(nSlots(indMarked), (rewards.opt(indMarked)-rewards.multisbm(indMarked)),'color', Colors{5}, 'LineStyle', 'none','marker', '+', ...
%     'LineWidth', 1.5, 'HandleVisibility', 'off');
% semilogx(nSlots(end), (rewards.opt(end)-rewards.multisbm(end)),'color', Colors{5}, 'marker', '+', ...
%     'LineWidth', 1.5);

% Sparring

semilogx(nSlots(indShow), (rewards.opt(indShow)-rewards.sparring(indShow)),'color', Colors{6},...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(indMarked), (rewards.opt(indMarked)-rewards.sparring(indMarked)),'color', Colors{6}, 'LineStyle', 'none','marker', 'h', ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(end), (rewards.opt(end)-rewards.sparring(end)),'color', Colors{6}, 'marker', 'h', ...
    'LineWidth', 1.5);

% CCB
semilogx(nSlots(indShow), (rewards.opt(indShow)-rewards.ccb(indShow)),'color', Colors{7},...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(indMarked), (rewards.opt(indMarked)-rewards.ccb(indMarked)),'color', Colors{7}, 'LineStyle', 'none','marker', '>', ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(end), (rewards.opt(end)-rewards.ccb(end)),'color', Colors{7}, 'marker', '>', ...
    'LineWidth', 1.5);

% SCB
semilogx(nSlots(indShow), (rewards.opt(indShow)-rewards.scb(indShow)),'color', Colors{8},...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(indMarked), (rewards.opt(indMarked)-rewards.scb(indMarked)),'color', Colors{8}, 'LineStyle', 'none','marker', '<', ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(end), (rewards.opt(end)-rewards.scb(end)),'color', Colors{8}, 'marker', '<', ...
    'LineWidth', 1.5);

% RMED1
semilogx(nSlots(indShow), (rewards.opt(indShow)-rewards.rmed1(indShow)),'color', Colors{9},...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(indMarked), (rewards.opt(indMarked)-rewards.rmed1(indMarked)),'color', Colors{9}, 'LineStyle', 'none','marker', 's', ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(end), (rewards.opt(end)-rewards.rmed1(end)),'color', Colors{9}, 'marker', 's', ...
    'LineWidth', 1.5);

% % RMED2
% semilogx(nSlots(indShow), (rewards.opt(indShow)-rewards.rmed2(indShow)),'color', Colors{10},...
%     'LineWidth', 1.5, 'HandleVisibility', 'off');
% semilogx(nSlots(indMarked), (rewards.opt(indMarked)-rewards.rmed2(indMarked)),'color', Colors{10}, 'LineStyle', 'none','marker', '*', ...
%     'LineWidth', 1.5, 'HandleVisibility', 'off');
% semilogx(nSlots(end), (rewards.opt(end)-rewards.rmed2(end)),'color', Colors{10}, 'marker', '*', ...
%     'LineWidth', 1.5);

% ECW-RMED
semilogx(nSlots(indShow), (rewards.opt(indShow)-rewards.ecw_rmed(indShow)),'color', Colors{10},...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(indMarked), (rewards.opt(indMarked)-rewards.ecw_rmed(indMarked)),'color', Colors{10}, 'LineStyle', 'none','marker', '*', ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(end), (rewards.opt(end)-rewards.ecw_rmed(end)),'color', Colors{10}, 'marker', '*', ...
    'LineWidth', 1.5);

% DTS
semilogx(nSlots(indShow), (rewards.opt(indShow)-rewards.dts(indShow)),'color', Colors{11},...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(indMarked), (rewards.opt(indMarked)-rewards.dts(indMarked)),'color', Colors{11}, 'LineStyle', 'none','marker', 'o', ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(end), (rewards.opt(end)-rewards.dts(end)),'color', Colors{11}, 'marker', 'o', ...
    'LineWidth', 1.5);

% DTS2
semilogx(nSlots(indShow), (rewards.opt(indShow)-rewards.dts2(indShow)),'color', Colors{12},...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(indMarked), (rewards.opt(indMarked)-rewards.dts2(indMarked)),'color', Colors{12}, 'LineStyle', 'none','marker', 'd', ...
    'LineWidth', 1.5, 'HandleVisibility', 'off');
semilogx(nSlots(end), (rewards.opt(end)-rewards.dts2(end)),'color', Colors{12}, 'marker', 'd', ...
    'LineWidth', 1.5);

hold off;
legend('BTM', 'SAVAGE', 'RUCB', 'RCS', 'Sparring', 'CCB', 'SCB', 'RMED1', 'ECW-RMED', 'D-TS', 'D-TS+');
xlabel('Time \itt', 'FontSize', 15);
ylabel('Regret', 'FontSize', 15);
set(gca, 'FontSize', 15);
grid on;
xlim([10, nSlots(end)+1]);


