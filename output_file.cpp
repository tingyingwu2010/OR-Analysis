#include "instance.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <string>

void Instance::write_output_file(size_t instance_number) {
	ostringstream file_name_stream;
	file_name_stream << "oracs_" << (instance_number + 1) << ".csv";
	string file_name = file_name_stream.str();
	ofstream output_file(file_name.c_str());
	if (output_file.fail()) { cout << "failed\n"; }
	output_file << "2\n";
	output_file << (instance_number + 1) << "\n";
	cout << "Now calculating costs for file\n";
	output_file << (roundf(this->calculate_obj_val() * 100) / 100) << "\n";
	output_file << routes.size() << "\n";
	for (Vehicle v : routes) {
		output_file << "\n";
		size_t element = 1;
		for (Node node : v.route) {
			if (node.type == 't') {
				// node is a transfer node
				string rep = "1";
				if (node.pickup) { rep.append("1"); }
				else { rep.append("0"); }

				// Node
				// number of digits : (i > 0 ? (int) log10 ((double) i) + 1 : 1)

				for (size_t i = 0; i < (3-((node.gen_idx+1) > 0 ? (int) log10 ((double) (node.gen_idx+1)) + 1 : 1)); i++) {
					rep.append("0");
				}
				rep.append(to_string((node.gen_idx+1)));

				// Request

				for (size_t i = 0; i < (3-((node.request_idx+1) > 0 ? (int) log10 ((double) (node.request_idx+1)) + 1 : 1)); i++) {
					rep.append("0");
				}
				rep.append(to_string(node.request_idx + 1));

				if (node.gen_idx == v.route.back().gen_idx) {
					output_file << rep << '\n';
				} else {
					output_file << rep << ',';
				}
			} else {
				// node is not a transfer node
				if (element == v.route.size()) {
					output_file << (node.gen_idx+1) << '\n';
				} else {
					output_file << (node.gen_idx+1) << ',';
				}
			}
			element++;
		}
		for (size_t idx = 0; idx < v.time_at_node.size(); idx++) {
			if (v.time_at_node[idx] == v.time_at_node.back()) {
				output_file << v.time_at_node[idx] + v.waiting_times[idx] << '\n';
			} else {
				output_file << v.time_at_node[idx] + v.waiting_times[idx] << ',';
			}
		}
	}
	output_file << "\n";
	output_file.close();
}

void Instance::output_vector(vector<double> &v, size_t instance_number) {
	ostringstream file_name_stream;
	file_name_stream << "vector_" << instance_number << ".csv";
	string file_name = file_name_stream.str();
	ofstream output_file(file_name.c_str());
	if (output_file.fail()) { cout << "failed\n"; }
	for (size_t i = 0; i < v.size(); i++) {
		output_file << v[i] << "\n";
	}
	output_file.close();
}

void Instance::output_data(vector<vector<size_t>> interactive_all, vector<vector<size_t>> interactive_acc1, vector<vector<size_t>> interactive_acc2,vector<vector<size_t>> interactive_acc3, vector<double> op_time, vector<double> op_del_time, size_t instance_number, size_t cost_rejection, size_t feasibility_rejection, double elapsed) {
	ostringstream file_name_stream;
	file_name_stream << "data_" << instance_number << ".csv";
	string file_name = file_name_stream.str();
	ofstream output_file(file_name.c_str());
	if (output_file.fail()) { cout << "failed\n"; }
	for (size_t i = 0; i < 4; i++) {
		for  (size_t j = 0; j < 5; j++) {
			if (j != 4) {
				output_file << interactive_all[i][j] << ",";
			} else {
				output_file << interactive_all[i][j] << "\n";
			}
		}
	}
	output_file << "\n";
	for (size_t i = 0; i < 4; i++) {
		for  (size_t j = 0; j < 5; j++) {
			if (j != 4) {
				output_file << interactive_acc1[i][j] << ",";
			} else {
				output_file << interactive_acc1[i][j] << "\n";
			}
		}
	}
	output_file << "\n";
	for (size_t i = 0; i < 4; i++) {
		for  (size_t j = 0; j < 5; j++) {
			if (j != 4) {
				output_file << interactive_acc2[i][j] << ",";
			} else {
				output_file << interactive_acc2[i][j] << "\n";
			}
		}
	}
	output_file << "\n";
	for (size_t i = 0; i < 4; i++) {
		for  (size_t j = 0; j < 5; j++) {
			if (j != 4) {
				output_file << interactive_acc3[i][j] << ",";
			} else {
				output_file << interactive_acc3[i][j] << "\n";
			}
		}
	}
	output_file << "\n";
	for (size_t i = 0; i < op_time.size(); i++) {
		if (i != op_time.size()-1) {
			output_file << op_time[i] << ",";
		} else {
			output_file << op_time[i] << "\n";
		}
	}
	output_file << "\n";
	for (size_t i = 0; i < op_del_time.size(); i++) {
		if (i != op_del_time.size()-1) {
			output_file << op_del_time[i] << ",";
		} else {
			output_file << op_del_time[i] << "\n";
		}
	}
	output_file << "\n";
	output_file << cost_rejection << "\n";
	output_file << feasibility_rejection << "\n";
	output_file << elapsed << "\n";
	output_file.close();
}