clear all;
%two cores conditional
utility = 0.2:0.2:2;
Melani = [100 100 100 100 100 96 35 3 0 0];
Me = [100 100 100 100 100 100 83 23 0 0];

%plot(utility,Melani,'-o'); 
%hold on;
%plot(utility,Me,'-s');

legend('Melani','Our solution','Location','NorthWest');
xlabel('Utilization'), ylabel('schedulability ratio')
title('Utilization test, single task with conditional branches, two processors')

clear all;
%four cores conditional
utility = 0.4:0.4:4;
Melani = [100 100 100 66 19 1 0 0 0 0];
Me = [100 100 100 94 52 17 0 0 0 0];

%four cores no conditions

utility = 0.4:0.4:4;
Melani = [100 100 100 72 23 0 0 0 0 0];
Me = [100 100 100 94 52 17 0 0 0 0];
Nasri = [100 100 100 94 52 17 0 0 0 0];


plot(utility,Melani,'-o'); 
hold on;
plot(utility,Me,'-s');
hold on;
plot(utility,Nasri,'-s');


legend('Melani','Our solution','Location','NorthWest');
xlabel('Utilization'), ylabel('schedulability ratio')
title('Utilization test, single task with conditional branches, four processors')
fontsize(16,"points")
grid on
