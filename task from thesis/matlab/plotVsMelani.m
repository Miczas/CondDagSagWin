% Example data
% Columns: [number_of_jobs, response_time]
my_analysis = [
31, 589;
25, 467;
19, 414;
14, 445;
19, 462;
20, 399;
20, 459;
21, 269;
23, 463;
18, 507;

19, 341;
23, 348;
32, 530;
24, 535;
22, 497;
14, 328;
18, 486;
27, 507;
30, 602;
22, 447;

14, 446;
20, 592;
32, 658;
21, 543;
14, 448;
22, 375;
14, 331;
31, 635;
29, 600;
21, 444;

14, 465;
18, 494;
14, 258;
27, 676;
31, 617;
21, 463;
20, 436;
22, 467;
23, 592;
26, 476;

31, 584;
14, 435;
30, 522;
18, 435;
25, 445;
20, 409;
14, 307;
24, 515;
21, 367;
20, 451;

19, 439;
21, 457;
21, 285;
14, 275;
32, 601;
14, 423;
19, 435;
32, 657;
14, 457;
20, 465;

14, 468;
18, 488;
23, 453;
21, 511;
14, 480;
14, 411;
32, 669;
20, 400;
31, 595;
14, 430;

19, 489;
14, 465;
30, 617;
18, 387;
18, 380;
14, 411;
20, 431;
14, 344;
32, 552;
25, 465;

19, 491;
14, 321;
19, 460;
18, 299;
14, 299;
22, 487;
20, 483;
22, 419;
18, 405;
20, 465;

22, 514;
32, 549;
23, 413;
32, 571;
14, 363;
19, 405;
20, 580;
28, 600;
19, 458;
22, 423;



];

Melani_analysis = [
31, 709;
25, 602;
19, 482;
14, 513;
19, 548;
20, 495;
20, 565;
21, 358;
23, 557;
18, 591;

19, 394;
23, 475;
32, 613;
24, 601;
22, 549;
14, 385;
18, 567;
27, 560;
30, 631;
22, 540;

14, 511;
20, 690;
32, 728;
21, 671;
14, 521;
22, 492;
14, 391;
31, 774;
29, 703;
21, 581;

14, 534;
18, 592;
14, 327;
27, 794;
31, 732;
21, 577;
20, 523;
22, 569;
23, 694;
26, 548;

31, 681;
14, 497;
30, 658;
18, 520;
25, 588;
20, 500;
14, 345;
24, 641;
21, 431;
20, 538;

19, 506;
21, 546;
21, 372;
14, 348;
32, 714;
14, 482;
19, 524;
32, 766;
14, 499;
20, 588;

14, 541;
18, 560;
23, 550;
21, 594;
14, 558;
14, 448;
32, 748;
20, 516;
31, 691;
14, 500;

19, 573;
14, 520;
30, 719;
18, 511;
18, 494;
14, 489;
20, 496;
14, 406;
32, 634;
25, 551;

19, 588;
14, 376;
19, 546;
18, 446;
14, 365;
22, 559;
20, 596;
22, 517;
18, 501;
20, 545;

22, 609;
32, 687;
23, 514;
32, 632;
14, 440;
19, 498;
20, 686;
28, 761;
19, 550;
22, 485;


];

% Unique number of jobs
jobs = unique(my_analysis(:, 1));

% Initialize arrays to store average response times and percentage reductions
my_avg_response = zeros(length(jobs), 1);
other_avg_response = zeros(length(jobs), 1);
percentage_reduction = zeros(length(jobs), 1);

% Calculate average response times and percentage reductions
for i = 1:length(jobs)
    job = jobs(i);
    
    % My analysis
    my_responses = my_analysis(my_analysis(:, 1) == job, 2);
    my_avg_response(i) = mean(my_responses);
    
    % Other analysis
    other_responses = Melani_analysis(Melani_analysis(:, 1) == job, 2);
    other_avg_response(i) = mean(other_responses);
    
    % Average percentage reduction for this number of jobs
    percentage_reduction(i) = mean((other_responses - my_responses) ./ other_responses * 100);
end

% Initialize array to store all percentage reductions
all_percentage_reductions = [];

% Calculate raw percentage reductions for each individual point
for i = 1:size(my_analysis, 1)
    job = my_analysis(i, 1);
    my_response = my_analysis(i, 2);
    other_response = Melani_analysis(i, 2);
    
    % Calculate percentage reduction for the current point
    reduction = ((other_response - my_response) / other_response) * 100;
    all_percentage_reductions = [all_percentage_reductions; reduction];
end


% Calculate raw max, min, and average percentage reductions
max_reduction = max(all_percentage_reductions);
min_reduction = min(all_percentage_reductions);
avg_reduction = mean(all_percentage_reductions);

% Display the results
fprintf('Raw Maximum Percentage Reduction: %.2f%%\n', max_reduction);
fprintf('Raw Minimum Percentage Reduction: %.2f%%\n', min_reduction);
fprintf('Raw Average Percentage Reduction: %.2f%%\n', avg_reduction);
% Plotting

figure;

plot(jobs, percentage_reduction, 'o');
xlabel('Number of Jobs');
ylabel('Average Percentage Reduction in Response Time (%)');
title('Percentage Reduction in Response Time per Number of Jobs (Four cores)');
axis([12 34 0 30])
fontsize(16,"points")
grid on;
