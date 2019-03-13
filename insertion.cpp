#include "instance.h"
#include <limits>
#include <algorithm>

size_t Instance::greedy_request_insertion(vector<size_t> request_bank) {
    //calculate costs for a new vehicle as a base case
	size_t best_vehicle = routes.size();
	size_t best_pickup_location = 1;
	size_t best_delivery_location = 2;
	double best_costs = numeric_limits<double>::max();
	size_t best_request = 0;
    for(size_t req : request_bank){
        if(nearest_depot_insertion_cost[req] < best_costs){
            best_costs = nearest_depot_insertion_cost[req];
            best_request = req;
        }
    }
    for(size_t request : request_bank){
    //see if other insertions are better
	for (size_t v = 0; v < routes.size(); v++) {
		for (size_t p = 1; p < routes[v].route.size(); p++) {
			for (size_t d = p; d < routes[v].route.size(); d++) {
				double candidate_costs = costs_of_inserting_request(routes[v], p, d, request);
				if (candidate_costs < best_costs) {
					/*vector<Vehicle> old_routes = routes;
					vector<size_t> old_pickup_vehicle = pickup_vehicle;
					vector<size_t> old_delivery_vehicle = delivery_vehicle;

					routes[best_vehicle].add_node(p, pickup_nodes[request]);
					routes[best_vehicle].add_node(d + 1, delivery_nodes[request]);
					if (routes[v].maximum_ride_time_correct() && routes[v].time_windows_correct() && routes[v].current_capacity_correct()) {*/
						best_costs = candidate_costs;
						best_vehicle = v;
						best_pickup_location = p;
						best_delivery_location = d;
						best_request = request;
					/*}
					routes = old_routes;
					pickup_vehicle = old_pickup_vehicle;
					delivery_vehicle = old_delivery_vehicle;*/
				}
			}
		}
	}
    }
	if (best_vehicle < routes.size()) {
        routes[best_vehicle].add_node(best_pickup_location, pickup_nodes[best_request]);
        routes[best_vehicle].add_node(best_delivery_location + 1, delivery_nodes[best_request]);
	} else if (best_vehicle == routes.size()){
	    size_t insert_loc = routes.size();
        routes.push_back(Vehicle());
        routes[routes.size()-1].v_index = routes.size()-1;
        routes[insert_loc].add_node(1, pickup_nodes[best_request]);
        routes[insert_loc].add_node(2, delivery_nodes[best_request]);
	} else { cout << "insertion failed\n"; }

	size_t best_request_loc = 0;
	for (size_t idx = 0; idx < request_bank.size(); idx++) {
        if (request_bank[idx] == best_request) {
            best_request_loc  = idx;
        }
	}

    return best_request_loc;
}

void Instance::random_request_insertion(size_t request) {

}

void Instance::greedy_route_insertion(size_t request) {
	size_t k1 = 0, k2 = 0, best_p = 0, best_d = 0, best_td = 0, best_tp = 0;
	double best_costs = numeric_limits<double>::max();
	Transfer_Node transfer_node;
	vector<Transfer_Node> open_facilities;

	for (Transfer_Node tn : transfer_nodes) {
		if (tn.open) { open_facilities.push_back(tn); }
	}
	
	// Case 1: if there are 0 vehicles -> find best 2 vehicles and insert a request with transfer
	// Case 2: if there is 1 vehicle -> find one other vehicle and insert a request with transfer in those vehicles
	
	for (Transfer_Node tn : open_facilities) {
		routes.push_back(Vehicle());
		routes.push_back(Vehicle());
		for (size_t v1 = 0; v1 < routes.size(); v1++) {
			for (size_t v2 = 0; v2 < routes.size(); v2++) {
				if (v1 == v2) {
					continue;
				}
				for (size_t p = 1; p < routes[v1].route.size() - 1; p++) {
					for (size_t td = p + 1; td < routes[v1].route.size(); td++) {
						
						double pickup_costs = costs_of_inserting_request_with_transfer(routes[v1], p, td, request, true, tn);
						for (size_t tp = 1; tp < routes[v2].route.size() - 1; tp++) {
							
							double minimum_slack = *min_element(routes[v2].slack_at_node.begin(), routes[v2].slack_at_node.end());
							if (routes[v1].time_at_node[td] + tn.service_time < routes[v2].time_at_node[tp] + minimum_slack) {
								// Only look for possible transfers, e.g. when time windows are correct for the transfer

								for (size_t d = tp + 1; d < routes[v2].route.size(); d++) {
									double delivery_costs = costs_of_inserting_request_with_transfer(routes[v2], tp, d, request, false, tn);
									if (pickup_costs + delivery_costs < best_costs) {
										best_costs = pickup_costs + delivery_costs;
										k1 = v1;
										k2 = v2;
										best_p = p;
										best_d = d;
										best_td = td;
										best_tp = tp;
										transfer_node = tn;
									}
								}
							}
						}
						
					}
				}
			}
		}
	}
	routes[k1].add_node(best_p, pickup_nodes[request]);
	routes[k1].add_delivery_transfer(best_td, transfer_node, request);
	routes[k2].add_pickup_transfer(best_tp, transfer_node, 0, request);
	routes[k2].add_node(best_d, delivery_nodes[request]);
}


void Instance::random_route_insertion(size_t request) {

}

double Instance::costs_of_inserting_request(Vehicle v, size_t p, size_t d, size_t request) {
	double arc_lengths = 0.0;
	vector<double> new_arc_durations = v.arc_durations;
	
	// Pickup insertion
	if (p == 1) {
        arc_lengths += arcs[nearest_depot_gen_idx_p[request]][pickup_nodes[request].gen_idx] + arcs[pickup_nodes[request].gen_idx][v.route[p].gen_idx] - new_arc_durations[p-1];
        new_arc_durations.erase(new_arc_durations.begin() + p-1);
        new_arc_durations.insert(new_arc_durations.begin() + p-1, arcs[pickup_nodes[request].gen_idx][v.route[p].gen_idx]);
        new_arc_durations.insert(new_arc_durations.begin() + p-1, arcs[nearest_depot_gen_idx_p[request]][pickup_nodes[request].gen_idx]);
	} else {
        arc_lengths += arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx] + arcs[pickup_nodes[request].gen_idx][v.route[p].gen_idx] - new_arc_durations[p-1];
        new_arc_durations.erase(new_arc_durations.begin() + p-1);
        new_arc_durations.insert(new_arc_durations.begin() + p-1, arcs[pickup_nodes[request].gen_idx][v.route[p].gen_idx]);
        new_arc_durations.insert(new_arc_durations.begin() + p-1, arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx]);
	}
	
	//short feasibility check
    double min_slack = *min_element(v.slack_at_node.begin() + p, v.slack_at_node.end());
	if ((arc_lengths + pickup_nodes[request].service_time > min_slack)||
       (v.time_at_node[p-1] +  new_arc_durations[p-1] + v.route[p-1].service_time + v.waiting_times[p-1] > pickup_nodes[request].upper_bound)){
        arc_lengths = numeric_limits<double>::max()/2;
	}

	// Delivery insertion
	if (d == p) {
        if(d == v.route.size()-1){
            arc_lengths += arcs[pickup_nodes[request].gen_idx][delivery_nodes[request].gen_idx] + arcs[delivery_nodes[request].gen_idx][nearest_depot_gen_idx_d[request]] - new_arc_durations[d];
        }else{
            arc_lengths += arcs[pickup_nodes[request].gen_idx][delivery_nodes[request].gen_idx] + arcs[delivery_nodes[request].gen_idx][v.route[d].gen_idx] - new_arc_durations[d];
        }
	} else {
        if(d == v.route.size()-1){
            arc_lengths += arcs[v.route[d-1].gen_idx][delivery_nodes[request].gen_idx] + arcs[delivery_nodes[request].gen_idx][nearest_depot_gen_idx_d[request]] - new_arc_durations[d-1];
        }else{
            arc_lengths += arcs[v.route[d-1].gen_idx][delivery_nodes[request].gen_idx] + arcs[delivery_nodes[request].gen_idx][v.route[d].gen_idx] - new_arc_durations[d];
        }
    }
	
    //short feasibility check
    min_slack = *min_element(v.slack_at_node.begin() + d, v.slack_at_node.end());
    if ((arc_lengths + pickup_nodes[request].service_time + delivery_nodes[request].service_time> min_slack) ||
       (v.time_at_node[d-1] + v.waiting_times[d-1] + v.route[d-1].service_time + arc_lengths -  arcs[delivery_nodes[request].gen_idx][v.route[d].gen_idx] > delivery_nodes[request].upper_bound)  ){
        arc_lengths = numeric_limits<double>::max()/2;
	}
	return travel_cost*arc_lengths;
}

double Instance::costs_of_inserting_request_with_transfer(Vehicle v, size_t p, size_t d, size_t request, bool pickup, Transfer_Node tn) {
	double arc_lengths = 0.0;
	if (pickup) {
		if (d == p + 1) {
			if (p == 1) {
				
			}
			// adding 3 arcs and deleting 1
			arc_lengths += arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx] 
						+ arcs[pickup_nodes[request].gen_idx][tn.gen_idx] 
						+ arcs[tn.gen_idx][v.route[p].gen_idx] 
						- arcs[v.route[p-1].gen_idx][v.route[p].gen_idx] // v.arc_durations[p-1]
		} else {
			if (p == 1 && d < v.route.size() - 1) {
				
			} else if (d == v.route.size() - 1 && p > 1) {
				
			} else {
			
			}
			// adding 4 arcs and deleting 2
			arc_lengths += arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx] 
						+ arcs[pickup_nodes[request].gen_idx][v.route[p].gen_idx]
						+ arcs[v.route[d-1].gen_idx][tn.gen_idx]
						+ arcs[tn.gen_idx][v.route[d-1].gen_idx]
						- arcs[v.route[p-1].gen_idx][v.route[p].gen_idx] // v.arc_durations[p-1]
						- arcs[v.route[d-2].gen_idx][v.route[d-1].gen_idx]; // v.arc_durations[d-2]
		}
	} else {
		if (d == p + 1) {
			// adding 3 arcs and deleting 1
			arc_lengths += arcs[v.route[p-1].gen_idx][tn.gen_idx] 
						+ arcs[tn.gen_idx][delivery_nodes[request].gen_idx] 
						+ arcs[delivery_nodes[request].gen_idx][v.route[p].gen_idx] 
						- arcs[v.route[p-1].gen_idx][v.route[p].gen_idx];  // v.arc_durations[p-1]
		} else {
			// adding 4 arcs and deleting 2
			arc_lengths += arcs[v.route[p-1].gen_idx][tn.gen_idx] 
						+ arcs[tn.gen_idx][v.route[p].gen_idx]
						+ arcs[v.route[d-1].gen_idx][delivery_nodes[request].gen_idx]
						+ arcs[delivery_nodes[request].gen_idx][v.route[d-1].gen_idx]
						- arcs[v.route[p-1].gen_idx][v.route[p].gen_idx] // v.arc_durations[p-1]
						- arcs[v.route[d-2].gen_idx][v.route[d-1].gen_idx]; // v.arc_durations[d-2]
		}
	}
	return travel_cost*arc_lengths;
}