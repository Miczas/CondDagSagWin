#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <string>


//#include <sys/resource.h>

#include "OptionParser.h"

#include "config.h"


#include "problem.hpp"
#include "space.hpp"

#include "io.hpp"

#define CONFIG_COLLECT_SCHEDULE_GRAPH 1

#define MAX_PROCESSORS 512

// command line options
static bool want_naive = false;
static bool want_dense = false;
static bool want_prm_iip = false;
static bool want_cw_iip = false;

static bool want_precedence = false;
static std::string precedence_file;

static bool want_aborts = false;
static std::string aborts_file;

static bool want_multiprocessor = true;
static unsigned int num_processors = 2;

#ifdef CONFIG_COLLECT_SCHEDULE_GRAPH
static bool want_dot_graph = true;
#endif
static double timeout;
static unsigned int max_depth = 0;

static bool want_rta_file;

static bool continue_after_dl_miss = false;
//
//
//
struct Analysis_result {
	bool schedulable;
	bool timeout;
	unsigned long number_of_states, number_of_edges, max_width, number_of_jobs;
	double cpu_time;
	std::string graph;
	std::string response_times_csv;
};

template<class Time, class Space>
static Analysis_result analyze(
	std::istream &in,
	std::istream &dag_in,
	std::istream &aborts_in)
{


	// Parse input files and create NP scheduling problem description
	NP::Scheduling_problem<Time> problem{
		NP::parse_file<Time>(in),
		NP::parse_dag_file(dag_in),
		NP::parse_abort_file<Time>(aborts_in),
		num_processors};

	// Set common analysis options
	NP::Analysis_options opts;
	opts.timeout = timeout;
	opts.max_depth = max_depth;
	opts.early_exit = !continue_after_dl_miss;
	opts.num_buckets = problem.jobs.size();
	opts.be_naive = want_naive;

	// Actually call the analysis engine
	auto space = Space::explore(problem, opts);

	// Extract the analysis results
	auto graph = std::ostringstream();

#ifdef CONFIG_COLLECT_SCHEDULE_GRAPH
	if (want_dot_graph)
		graph << space;
#endif

	auto rta = std::ostringstream();

	if (want_rta_file) {
		rta << "Task ID, Job ID, BCCT, WCCT, BCRT, WCRT" << std::endl;
		for (const auto& j : problem.jobs) {
			Interval<Time> finish = space.get_finish_times(j);
			rta << j.get_task_id() << ", "
			    << j.get_job_id() << ", "
			    << finish.from() << ", "
			    << finish.until() << ", "
			    << std::max<long long>(0,
			                           (finish.from() - j.earliest_arrival()))
			    << ", "
			    << (finish.until() - j.earliest_arrival())
			    << std::endl;
		}
	}

	return {
		space.is_schedulable(),
		space.was_timed_out(),
		space.number_of_states(),
		space.number_of_edges(),
		space.max_exploration_front_width(),
		problem.jobs.size(),
		space.get_cpu_time(),
		graph.str(),
		rta.str()
	};
}

static Analysis_result process_stream(
	std::istream &in,
	std::istream &dag_in,
	std::istream &aborts_in)
{
	if (want_multiprocessor && want_dense)
		return analyze<dense_t, NP::Global::State_space<dense_t>>(in, dag_in, aborts_in);
	else if (want_multiprocessor && !want_dense)
		return analyze<dtime_t, NP::Global::State_space<dtime_t>>(in, dag_in, aborts_in);
//	else if (want_dense && want_prm_iip)
//		return analyze<dense_t, NP::Uniproc::State_space<dense_t, NP::Uniproc::Precatious_RM_IIP<dense_t>>>(in, dag_in, aborts_in);
//	else if (want_dense && want_cw_iip)
//		return analyze<dense_t, NP::Uniproc::State_space<dense_t, NP::Uniproc::Critical_window_IIP<dense_t>>>(in, dag_in, aborts_in);
//	else if (want_dense && !want_prm_iip)
//		return analyze<dense_t, NP::Uniproc::State_space<dense_t>>(in, dag_in, aborts_in);
//	else if (!want_dense && want_prm_iip)
//		return analyze<dtime_t, NP::Uniproc::State_space<dtime_t, NP::Uniproc::Precatious_RM_IIP<dtime_t>>>(in, dag_in, aborts_in);
//	else if (!want_dense && want_cw_iip)
//		return analyze<dtime_t, NP::Uniproc::State_space<dtime_t, NP::Uniproc::Critical_window_IIP<dtime_t>>>(in, dag_in, aborts_in);
//	else
//		return analyze<dtime_t, NP::Uniproc::State_space<dtime_t>>(in, dag_in, aborts_in);
}

int totalSched = 0;

static void process_file(const std::string& fname)
{
	try {
		Analysis_result result;

		auto empty_dag_stream = std::istringstream("\n");
		auto empty_aborts_stream = std::istringstream("\n");
		auto dag_stream = std::ifstream();
		auto aborts_stream = std::ifstream();

		if (want_precedence)
			dag_stream.open(precedence_file);

		if (want_aborts)
			aborts_stream.open(aborts_file);

		std::istream &dag_in = want_precedence ?
			static_cast<std::istream&>(dag_stream) :
			static_cast<std::istream&>(empty_dag_stream);

		std::istream &aborts_in = want_aborts ?
			static_cast<std::istream&>(aborts_stream) :
			static_cast<std::istream&>(empty_aborts_stream);

		if (fname == "-")
			result = process_stream(std::cin, dag_in, aborts_in);
		else {
			auto in = std::ifstream(fname, std::ios::in);
			result = process_stream(in, dag_in, aborts_in);
#ifdef CONFIG_COLLECT_SCHEDULE_GRAPH
			if (want_dot_graph) {
				std::string dot_name = fname;
				auto p = dot_name.find(".csv");
				if (p != std::string::npos) {
					dot_name.replace(p, std::string::npos, ".dot");
					auto out  = std::ofstream(dot_name,  std::ios::out);
					out << result.graph;
					out.close();
				}
			}
#endif
			if (want_rta_file) {
				std::string rta_name = fname;
				auto p = rta_name.find(".csv");
				if (p != std::string::npos) {
					rta_name.replace(p, std::string::npos, ".rta.csv");
					auto out  = std::ofstream(rta_name,  std::ios::out);
					out << result.response_times_csv;
					out.close();
				}
			}
		}



		std::cout << fname;

		if ((int)result.schedulable)
		{
			totalSched++;
		}

		if (max_depth && max_depth < result.number_of_jobs)
			// mark result as invalid due to debug abort
			std::cout << ",  X";
		else
			std::cout << ",  " << (int) result.schedulable;

		std::cout << ",  " << result.number_of_jobs
		          << ",  " << result.number_of_states
		          << ",  " << result.number_of_edges
		          << ",  " << result.max_width
		          << ",  " << std::fixed << result.cpu_time
		          << ",  " << (int) result.timeout
		          << ",  " << num_processors
				  << ",  " << totalSched
		          << std::endl;
	} catch (std::ios_base::failure& ex) {
		std::cerr << fname;
		if (want_precedence)
			std::cerr << " + " << precedence_file;
		std::cerr <<  ": parse error" << std::endl;
		exit(1);
	} catch (NP::InvalidJobReference& ex) {
		std::cerr << precedence_file << ": bad job reference: job "
		          << ex.ref.job << " of task " << ex.ref.task
			      << " is not part of the job set given in "
			      << fname
			      << std::endl;
		exit(3);
	} catch (std::exception& ex) {
		std::cerr << fname << ": '" << ex.what() << "'" << std::endl;
		exit(1);
	}
}

static void print_header(){
	std::cout << "# file name"
	          << ", schedulable?"
	          << ", #jobs"
	          << ", #states"
	          << ", #edges"
	          << ", max width"
	          << ", CPU time"
	          << ", memory"
	          << ", timeout"
	          << ", #CPUs"
	          << std::endl;
}

int main(int argc, char** argv)
{
	auto parser = optparse::OptionParser();

	parser.description("Exact NP Schedulability Tester");
	parser.usage("usage: %prog [OPTIONS]... [JOB SET FILES]...");

	parser.add_option("-t", "--time").dest("time_model")
		.metavar("TIME-MODEL")
		.choices({ "dense", "discrete" }).set_default("discrete")
		.help("choose 'discrete' or 'dense' time (default: discrete)");

	parser.add_option("-l", "--time-limit").dest("timeout")
		.help("maximum CPU time allowed (in seconds, zero means no limit)")
		.set_default("0");

	parser.add_option("-d", "--depth-limit").dest("depth")
		.help("abort graph exploration after reaching given depth (>= 2)")
		.set_default("0");

	parser.add_option("-n", "--naive").dest("naive").set_default("0")
		.action("store_const").set_const("1")
		.help("use the naive exploration method (default: merging)");

	parser.add_option("-i", "--iip").dest("iip")
		.choices({ "none", "P-RM", "CW" }).set_default("none")
		.help("the IIP to use (default: none)");

	parser.add_option("-p", "--precedence").dest("precedence_file")
		.help("name of the file that contains the job set's precedence DAG")
		.set_default("");

	parser.add_option("-a", "--abort-actions").dest("abort_file")
		.help("name of the file that contains the job set's abort actions")
		.set_default("");

	parser.add_option("-m", "--multiprocessor").dest("num_processors")
		.help("set the number of processors of the platform")
		.set_default("1");

	parser.add_option("--threads").dest("num_threads")
		.help("set the number of worker threads (parallel analysis)")
		.set_default("0");

	parser.add_option("--header").dest("print_header")
		.help("print a column header")
		.action("store_const").set_const("1")
		.set_default("0");

	parser.add_option("-g", "--save-graph").dest("dot").set_default("0")
		.action("store_const").set_const("1")
		.help("store the state graph in Graphviz dot format (default: off)");

	parser.add_option("-r", "--save-response-times").dest("rta").set_default("0")
		.action("store_const").set_const("1")
		.help("store the best- and worst-case response times (default: off)");

	parser.add_option("-c", "--continue-after-deadline-miss")
		.dest("go_on_after_dl").set_default("0")
		.action("store_const").set_const("1")
		.help("do not abort the analysis on the first deadline miss "
			"(default: off)");


	auto options = parser.parse_args(argc, argv);

	const std::string& time_model = options.get("time_model");
	want_dense = time_model == "dense";

	const std::string& iip = options.get("iip");
	want_prm_iip = iip == "P-RM";
	want_cw_iip = iip == "CW";

	want_naive = options.get("naive");

	timeout = options.get("timeout");

	max_depth = options.get("depth");
	if (options.is_set_by_user("depth")) {
		if (max_depth <= 1) {
			std::cerr << "Error: invalid depth argument\n" << std::endl;
			return 1;
		}
		max_depth -= 1;
	}

	want_precedence = options.is_set_by_user("precedence_file");
	if (want_precedence && parser.args().size() > 1) {
		std::cerr << "[!!] Warning: multiple job sets "
			<< "with a single precedence DAG specified."
			<< std::endl;
	}
	precedence_file = (const std::string&)options.get("precedence_file");

	want_aborts = options.is_set_by_user("abort_file");
	if (want_aborts && parser.args().size() > 1) {
		std::cerr << "[!!] Warning: multiple job sets "
			<< "with a single abort action list specified."
			<< std::endl;
	}
	aborts_file = (const std::string&)options.get("abort_file");

	want_multiprocessor = options.is_set_by_user("num_processors");
	num_processors = options.get("num_processors");
	if (!num_processors || num_processors > MAX_PROCESSORS) {
		std::cerr << "Error: invalid number of processors\n" << std::endl;
		return 1;
	}

	want_rta_file = options.get("rta");

	continue_after_dl_miss = options.get("go_on_after_dl");

#ifdef CONFIG_COLLECT_SCHEDULE_GRAPH
	want_dot_graph = true;
#else
	if (options.is_set_by_user("dot")) {
		std::cerr << "Error: graph collection support must be enabled "
			<< "during compilation (CONFIG_COLLECT_SCHEDULE_GRAPH "
			<< "is not set)." << std::endl;
		return 2;
	}
#endif
	
	if (options.get("print_header"))
		print_header();


	/*for (int i = 0; i < 100; i++)
	{
		for (auto f : parser.args())
		{
			std::stringstream ss;
			ss << "..\\tests\\temp_" << std::setfill('0') << std::setw(3) << i << ".prec.csv";
			precedence_file = ss.str();

			std::stringstream ss1;
			ss1 << "..\\tests\\temp_" << std::setfill('0') << std::setw(3) << i << ".csv";
			f = ss1.str();

			process_file(f);
		}
	}
	*/

	for (auto f : parser.args())
		process_file(f);

	if (parser.args().empty())
		process_file("-");

	return 0;
}
