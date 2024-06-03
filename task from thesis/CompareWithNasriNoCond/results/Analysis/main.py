import pandas as pd
import os

# Print the current working directory
print("Current Working Directory:", os.getcwd())


# Function to read the results file and process it into a DataFrame
def read_results(file_path):
    column_names = ['test_name', 'schedulability_result', 'num_jobs', 'num_states', 'num_edges',
                    'zero1', 'cpu_time', 'zero2', 'timeout', 'cpus', 'total_schedulability']
    df = pd.read_csv(file_path, header=None, names=column_names)

    # Convert relevant columns to appropriate data types
    df['cpu_time'] = pd.to_numeric(df['cpu_time'], errors='coerce')  # Convert cpu_time to float
    df['schedulability_result'] = pd.to_numeric(df['schedulability_result'],
                                                errors='coerce')  # Convert schedulability_result to integer

    return df


# Read MyResults and NarsiResults
my_results = read_results(r'C:\Users\micha\Documents\studia\CondDagSagWin\task from thesis\CompareWithNasriNoCond\results\Analysis\MyResults.csv')
narsi_results = read_results(r'C:\Users\micha\Documents\studia\CondDagSagWin\task from thesis\CompareWithNasriNoCond\results\Analysis\NarsiResults.csv')

# Initialize lists to store the results
average_slowdown_per_bundle = []
schedulable_tests_my_per_bundle = []
schedulable_tests_narsi_per_bundle = []

# Process results in bundles of 100 tests
for i in range(0, len(my_results), 100):
    my_bundle = my_results.iloc[i:i+100]
    narsi_bundle = narsi_results.iloc[i:i+100]

    # Check if total cpu_time in narsi_bundle is zero to avoid division by zero
    if narsi_bundle['cpu_time'].sum() == 0:
        print(f"Skipping bundle {i//100 + 1} due to zero CPU time in NarsiResults")
        continue

    # Calculate average slowdown
    slowdown = (my_bundle['cpu_time'].sum() / narsi_bundle['cpu_time'].sum())
    average_slowdown_per_bundle.append(slowdown)

    # Calculate number of schedulable tests
    schedulable_tests_my = my_bundle['schedulability_result'].sum()
    schedulable_tests_narsi = narsi_bundle['schedulability_result'].sum()

    schedulable_tests_my_per_bundle.append(schedulable_tests_my)
    schedulable_tests_narsi_per_bundle.append(schedulable_tests_narsi)

# Print results
for idx, (slowdown, sched_my, sched_narsi) in enumerate(zip(average_slowdown_per_bundle, schedulable_tests_my_per_bundle, schedulable_tests_narsi_per_bundle)):
    print(f"Bundle {idx+1}:")
    print(f"  Average Slowdown (MyResults vs NarsiResults): {slowdown}")
    print(f"  Number of Schedulable Tests (MyResults): {sched_my}")
    print(f"  Number of Schedulable Tests (NarsiResults): {sched_narsi}")
    print()