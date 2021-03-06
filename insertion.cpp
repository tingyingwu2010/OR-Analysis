#include "instance.h"
#include <limits>
#include <algorithm>

size_t Instance::greedy_request_insertion(vector<size_t> &request_bank) {
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
    vector<double> added_times(2,0);
    for(size_t request : request_bank){
    //see if other insertions are better
	for (size_t v = 0; v < routes.size(); v++) {
		for (size_t p = 1; p < routes[v].route.size(); p++) {
			for (size_t d = p + 1; d < routes[v].route.size()+1; d++) {
				double candidate_costs = costs_of_inserting_request(routes[v], p, d, request, added_times);
				if (candidate_costs < best_costs) {

                //Calculate waiting times for insertions
                double p_waiting = pickup_feasible(routes[v], p, request, pickup_nodes[request]);
                if(p_waiting != -1234567){
                double d_waiting = delivery_feasible(routes[v], p, d, request, p_waiting, added_times[1], pickup_nodes[request], delivery_nodes[request]);
                if(d_waiting != -1234567){
                //end waiting calculations
                    if(p==d-1){
                        if(*max_element(routes[v].current_capacity.begin() + p-1, routes[v].current_capacity.begin() + p) < vehicle_capacity &&
                           check_slack_times(routes[v], d-1, routes[v].route.size(), added_times[1] + p_waiting + d_waiting) &&
                           insertion_ride_times_feasible(routes[v], p, added_times[1] + p_waiting + d_waiting)){
                                best_costs = candidate_costs;
                                best_vehicle = v;
                                best_pickup_location = p;
                                best_delivery_location = d;
                                best_request = request;
                        }
                    }else{
                        if(*max_element(routes[v].current_capacity.begin() + p-1, routes[v].current_capacity.begin() + d-1) < vehicle_capacity &&
                        check_slack_times(routes[v], d-1, routes[v].route.size(), added_times[1] + added_times[0] + p_waiting + d_waiting) &&
                        check_slack_times(routes[v], p, d-1, added_times[0] + p_waiting) &&
                        insertion_ride_times_feasible(routes[v], p, added_times[0] + p_waiting) &&
                        insertion_delivery_times_feasible(routes[v], p, d, added_times[0] + p_waiting, added_times[1] + d_waiting)){
                                best_costs = candidate_costs;
                                best_vehicle = v;
                                best_pickup_location = p;
                                best_delivery_location = d;
                                best_request = request;

                        }
                    }
                }
                }
				}
			}
		}
	}
    }
	if (best_vehicle < routes.size()) {
        routes[best_vehicle].add_node(best_pickup_location, pickup_nodes[best_request]);
        routes[best_vehicle].add_node(best_delivery_location, delivery_nodes[best_request]);
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

size_t Instance::regret_2_insertion(vector<size_t> &request_bank){
    vector<vector<size_t>> first_loc(request_bank.size(), {routes.size(), 1, 2});
    vector<vector<size_t>> second_loc(request_bank.size(), {routes.size(), 1, 2});
    vector<double> first_cost(request_bank.size(), 0);
    vector<double> second_cost(request_bank.size(), numeric_limits<double>::max());

    for(size_t idx = 0; idx < request_bank.size(); idx++){
        first_cost[idx] = nearest_depot_insertion_cost[request_bank[idx]];
    }
    vector<double> added_times(2,0);
    for(size_t idx = 0; idx < request_bank.size(); idx++){
        for (size_t v = 0; v < routes.size(); v++) {
            for (size_t p = 1; p < routes[v].route.size(); p++) {
                for (size_t d = p + 1; d < routes[v].route.size()+1; d++) {
                    double candidate_costs = costs_of_inserting_request(routes[v], p, d, request_bank[idx], added_times);
                    if (candidate_costs < second_cost[idx]) {

                    //Calculate waiting times for insertions
                    double p_waiting = pickup_feasible(routes[v], p, request_bank[idx], pickup_nodes[request_bank[idx]]);
                    if(p_waiting != -1234567){
                    double d_waiting = delivery_feasible(routes[v], p, d, request_bank[idx], p_waiting, added_times[1], pickup_nodes[request_bank[idx]], delivery_nodes[request_bank[idx]]);
                    if(d_waiting != -1234567){
                    //end waiting calculations

                        if(p == d-1){
                            if(*max_element(routes[v].current_capacity.begin() + p-1, routes[v].current_capacity.begin() + p) < vehicle_capacity &&
                                check_slack_times(routes[v], d-1, routes[v].route.size(), added_times[1] + p_waiting + d_waiting) &&
                                insertion_ride_times_feasible(routes[v], p, added_times[1] + p_waiting + d_waiting) &&
                                candidate_costs < first_cost[idx]){
                                        second_cost[idx] = first_cost[idx];
                                        first_cost[idx] = candidate_costs;
                                        second_loc[idx] = first_loc[idx];
                                        first_loc[idx] = {v, p, d};
                            }
                        }else{
                            if(*max_element(routes[v].current_capacity.begin() + p-1, routes[v].current_capacity.begin() + d-1) < vehicle_capacity &&
                                check_slack_times(routes[v], d-1, routes[v].route.size(), added_times[1] + added_times[0] + p_waiting + d_waiting) &&
                                check_slack_times(routes[v], p, d-1, added_times[0] + p_waiting) &&
                                insertion_ride_times_feasible(routes[v], p, added_times[0] + p_waiting) &&
                               insertion_delivery_times_feasible(routes[v], p, d, added_times[0] + p_waiting, added_times[1] + d_waiting) &&
                                candidate_costs < first_cost[idx]){
                                        second_cost[idx] = first_cost[idx];
                                        first_cost[idx] = candidate_costs;
                                        second_loc[idx] = first_loc[idx];
                                        first_loc[idx] = {v, p, d};
                            }
                        }
                    }
                    }
                    }
                }
            }
        }
    }
    //saves cost differences in second_cost vector
    transform(second_cost.begin(), second_cost.end(), first_cost.begin(), second_cost.begin(), minus<double>());
    size_t best = distance(second_cost.begin(), max_element(second_cost.begin(), second_cost.end()));

    if (second_loc[best][0] < routes.size()) {
        routes[second_loc[best][0]].add_node(second_loc[best][1], pickup_nodes[request_bank[best]]);
        routes[second_loc[best][0]].add_node(second_loc[best][2], delivery_nodes[request_bank[best]]);
	} else if (second_loc[best][0] == routes.size()){
        routes.push_back(Vehicle());
        routes[routes.size()-1].v_index = routes.size()-1;
        routes[routes.size()-1].add_node(1, pickup_nodes[request_bank[best]]);
        routes[routes.size()-1].add_node(2, delivery_nodes[request_bank[best]]);
	} else { cout << "insertion failed\n"; }
    return best;
}

size_t Instance::random_request_greedy_insertion(vector<size_t> &request_bank){
    size_t best_request_loc = rand() % request_bank.size();
    size_t request = request_bank[best_request_loc];

    size_t best_vehicle = routes.size();
	size_t best_pickup_location = 1;
	size_t best_delivery_location = 2;
	double best_costs = nearest_depot_insertion_cost[request];

    vector<double> added_times(2,0);
    for (size_t v = 0; v < routes.size(); v++) {
		for (size_t p = 1; p < routes[v].route.size(); p++) {
			for (size_t d = p + 1; d < routes[v].route.size()+1; d++) {
				double candidate_costs = costs_of_inserting_request(routes[v], p, d, request, added_times);
				if (candidate_costs < best_costs) {

                //Calculate waiting times for insertions
                double p_waiting = pickup_feasible(routes[v], p, request , pickup_nodes[request]);
                if(p_waiting != -1234567){
                double d_waiting = delivery_feasible(routes[v], p, d, request, p_waiting, added_times[0], pickup_nodes[request], delivery_nodes[request]);
                if(d_waiting != -1234567){
                //end waiting calculations

                    if(p==d-1){
                        if(*max_element(routes[v].current_capacity.begin() + p-1, routes[v].current_capacity.begin() + p-1) < vehicle_capacity &&
                            check_slack_times(routes[v], d-1, routes[v].route.size(), added_times[1] + p_waiting + d_waiting) &&
                           insertion_ride_times_feasible(routes[v], p, added_times[1] + p_waiting + d_waiting)){
                                best_costs = candidate_costs;
                                best_vehicle = v;
                                best_pickup_location = p;
                                best_delivery_location = d;
                        }
                    }else{
                        if(*max_element(routes[v].current_capacity.begin() + p-1, routes[v].current_capacity.begin() + d-1) < vehicle_capacity &&
                            check_slack_times(routes[v], d-1, routes[v].route.size(), added_times[1] + added_times[0] + p_waiting + d_waiting) &&
                            check_slack_times(routes[v], p, d-1, added_times[0] + p_waiting) &&
                           insertion_delivery_times_feasible(routes[v], p, d, added_times[0] + p_waiting, added_times[1] + d_waiting) &&
                            insertion_ride_times_feasible(routes[v], p, added_times[0] + p_waiting)){
                                best_costs = candidate_costs;
                                best_vehicle = v;
                                best_pickup_location = p;
                                best_delivery_location = d;

                        }
                    }
                }
                }
                }
			}
		}
	}
    if (best_vehicle < routes.size()) {
        routes[best_vehicle].add_node(best_pickup_location, pickup_nodes[request]);
        routes[best_vehicle].add_node(best_delivery_location, delivery_nodes[request]);
	} else if (best_vehicle == routes.size()){
	    size_t insert_loc = routes.size();
        routes.push_back(Vehicle());
        routes[routes.size()-1].v_index = routes.size()-1;
        routes[insert_loc].add_node(1, pickup_nodes[request]);
        routes[insert_loc].add_node(2, delivery_nodes[request]);
	} else { cout << "insertion failed\n"; }

    return best_request_loc;
}

size_t Instance::greedy_route_insertion(vector<size_t> &request_bank) {
    size_t best_request_loc = rand() % request_bank.size();
    size_t request = request_bank[best_request_loc];

	size_t k1 = 0, k2 = 0, best_p = 0, best_d = 0, best_td = 0, best_tp = 0;
	double best_costs = numeric_limits<double>::max();
	size_t transfer_node = 10000;
	vector<Transfer_Node> open_facilities;

	for (size_t i = 0; i < transfer_nodes.size(); i++) {
		if (transfer_nodes[i].open) { open_facilities.push_back(transfer_nodes[i]); }
	}

	// Case 1: if there are 0 vehicles -> find best 2 vehicles and insert a request with transfer
	// Case 2: if there is 1 vehicle -> find one other vehicle and insert a request with transfer in those vehicles
    vector<double> added_times_p(2,0);
    vector<double> added_times_d(2,0);
	for (Transfer_Node tn : open_facilities) {
        for(size_t iter = 0; iter < 20; iter++){
        size_t v1 = rand()%routes.size();
        size_t v2 = rand()%routes.size();
		//for (size_t v1 = 0; v1 < routes.size(); v1++) {
		//	for (size_t v2 = 0; v2 < routes.size(); v2++) {
				if (v1 == v2) {continue;}
				for (size_t p = 1; p < routes[v1].route.size(); p++) {
					for (size_t td = p + 1; td < routes[v1].route.size() + 1; td++) {
						double pickup_costs = costs_of_inserting_request_with_transfer_pickup(routes[v1], p, td, request, tn, added_times_p);
						//Check feasibility for pickup vehicle
                        double p_waiting = pickup_feasible(routes[v1], p, request, pickup_nodes[request]);
                        if(p_waiting == -1234567){continue;}
                        double td_waiting = delivery_feasible(routes[v1], p, td, request, p_waiting, added_times_p[0], pickup_nodes[request], tn);
                        if(td_waiting == -1234567){continue;}
                        if(p==td-1){
                            if(*max_element(routes[v1].current_capacity.begin() + p-1, routes[v1].current_capacity.begin() + p-1) >= vehicle_capacity ||
                                !check_slack_times(routes[v1], td-1, routes[v1].route.size(), added_times_p[1] + p_waiting + td_waiting) ||
                                !insertion_ride_times_feasible(routes[v1], td, added_times_p[1] + p_waiting + td_waiting)){continue;}
                            tn.lower_bound = routes[v1].time_at_node[td-2] + added_times_p[1] + p_waiting + td_waiting - arcs[tn.gen_idx][routes[v1].route[td-1].gen_idx];
                        }else{
                            if(*max_element(routes[v1].current_capacity.begin() + p-1, routes[v1].current_capacity.begin() + td-1) >= vehicle_capacity ||
                                !check_slack_times(routes[v1], td-1, routes[v1].route.size(), added_times_p[1] + added_times_p[0] + p_waiting + td_waiting) ||
                                !check_slack_times(routes[v1], p, td-1, added_times_p[0] + p_waiting)||
                                !insertion_ride_times_feasible(routes[v1], p, added_times_p[0] + p_waiting) ||
                                !insertion_delivery_times_feasible(routes[v1], p, td, added_times_p[0] + p_waiting, added_times_p[1] + td_waiting)){continue;}
                            tn.lower_bound = routes[v1].time_at_node[td-2] + added_times_p[0] + added_times_p[1] + p_waiting + td_waiting - arcs[tn.gen_idx][routes[v1].route[td-1].gen_idx];
                        }

						//Delivery vehicle
						for (size_t tp = 1; tp < routes[v2].route.size(); tp++) {
								// Only look for possible transfers, e.g. when time windows are correct for the transfer
								if(routes[v1].time_at_node[td-2] + routes[v1].waiting_times[td-2] + routes[v1].route[td-2].service_time > routes[v2].time_at_node[tp-1] + routes[v2].waiting_times[tp-1] + routes[v2].route[tp-1].service_time){continue;}
								for (size_t d = tp + 1; d < routes[v2].route.size() + 1; d++) {

									double delivery_costs = costs_of_inserting_request_with_transfer_delivery(routes[v2], tp, d, request, tn, added_times_d);
									//feasibility check delivery vehicle
                                    double tp_waiting = pickup_feasible(routes[v2], tp, request, tn);
                                    if(p_waiting == -1234567){continue;}
                                    double d_waiting = delivery_feasible(routes[v2], tp, d, request, tp_waiting, added_times_d[0], tn, delivery_nodes[request]);
                                    if(td_waiting == -1234567){continue;}
                                    if(tp==d-1){
                                        if(*max_element(routes[v2].current_capacity.begin() + tp-1, routes[v2].current_capacity.begin() + tp-1) >= vehicle_capacity ||
                                            !check_slack_times(routes[v2], d-1, routes[v2].route.size(), added_times_d[1] + tp_waiting + d_waiting) ||
                                            !insertion_ride_times_feasible(routes[v2], tp, added_times_d[1] + tp_waiting + d_waiting)){continue;}
                                    }else{
                                        if(*max_element(routes[v2].current_capacity.begin() + tp-1, routes[v2].current_capacity.begin() + d-1) >= vehicle_capacity ||
                                            !check_slack_times(routes[v2], d-1, routes[v2].route.size(), added_times_d[1] + added_times_d[0] + tp_waiting + d_waiting) ||
                                            !check_slack_times(routes[v2], tp, d-1, added_times_d[0] + tp_waiting)||
                                            !insertion_ride_times_feasible(routes[v2], tp, added_times_d[0] + tp_waiting)||
                                            !insertion_delivery_times_feasible(routes[v2], tp, d, added_times_d[0] + tp_waiting, added_times_d[1] + d_waiting)
                                            ){continue;}
                                    }
									//cout << "Delivery costs: " << delivery_costs << "\n";
									if (pickup_costs + delivery_costs < best_costs) {
										best_costs = pickup_costs + delivery_costs;
										k1 = v1;
										k2 = v2;
										best_p = p;
										best_d = d;
										best_td = td;
										best_tp = tp;
										transfer_node = tn.index;
									}
							}
						}

					}
				}
			//}
		}
	}
	if(transfer_node != 10000){
        routes[k1].add_node(best_p, pickup_nodes[request]);
        double time = routes[k1].add_delivery_transfer(best_td, transfer_nodes[transfer_node], request);
        routes[k2].add_pickup_transfer(best_tp, transfer_nodes[transfer_node], time, request);
        routes[k2].add_node(best_d, delivery_nodes[request]);
	}else{
        best_request_loc = this->random_request_greedy_insertion(request_bank);
	}
    return best_request_loc;
}

double Instance::costs_of_inserting_request(Vehicle &v, size_t p, size_t d, size_t request, vector<double> &information) {
	double arc_lengths = 0.0, arc_lengths_right = 0.0;

	if (d == p + 1) {
		arc_lengths += arcs[pickup_nodes[request].gen_idx][delivery_nodes[request].gen_idx]
						- arcs[v.route[p-1].gen_idx][v.route[p].gen_idx];
	} else {
		arc_lengths += arcs[pickup_nodes[request].gen_idx][v.route[p].gen_idx]
						- arcs[v.route[p-1].gen_idx][v.route[p].gen_idx];
		arc_lengths_right += arcs[v.route[d-2].gen_idx][delivery_nodes[request].gen_idx]
							- arcs[v.route[d-2].gen_idx][v.route[d-1].gen_idx];
	}

	if (p > 1 && d < v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx]
							+ arcs[delivery_nodes[request].gen_idx][v.route[d-1].gen_idx];
		} else {
			arc_lengths += arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx];
			arc_lengths_right += arcs[delivery_nodes[request].gen_idx][v.route[d-1].gen_idx];
		}
	} else if (p > 1 && d == v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx]
							+ arcs[delivery_nodes[request].gen_idx][nearest_depot_gen_idx_d[request]];
		} else {
			arc_lengths += arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx];
			arc_lengths_right += arcs[delivery_nodes[request].gen_idx][nearest_depot_gen_idx_d[request]];
		}
	} else if (p == 1 && d < v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[delivery_nodes[request].gen_idx][v.route[d-1].gen_idx]
							+ arcs[nearest_depot_gen_idx_p[request]][pickup_nodes[request].gen_idx];
		} else {
			arc_lengths += arcs[nearest_depot_gen_idx_p[request]][pickup_nodes[request].gen_idx];
			arc_lengths_right += arcs[delivery_nodes[request].gen_idx][v.route[d-1].gen_idx];
		}
	} else if (p == 1 && d == v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[nearest_depot_gen_idx_p[request]][pickup_nodes[request].gen_idx]
							+ arcs[delivery_nodes[request].gen_idx][nearest_depot_gen_idx_d[request]];
		} else {
			arc_lengths += arcs[nearest_depot_gen_idx_p[request]][pickup_nodes[request].gen_idx];
			arc_lengths_right += arcs[delivery_nodes[request].gen_idx][nearest_depot_gen_idx_d[request]];
		}
	} else {
		cout << "Error calculating costs for normal insertion!\n";
		cout << "Pickup transfer - p: " << p << ", d: " << d << ", route size:" << v.route.size() << "\n";
	}
	/*if(arc_lengths < -100000){
        cout << "Error, large negative arc lengths in cost calculation. Request:  " << request << '\n';
	}*/

	if (d == p + 1) {
		information[0] = arc_lengths + pickup_nodes[request].service_time + delivery_nodes[request].service_time;
		information[1] = arc_lengths + pickup_nodes[request].service_time + delivery_nodes[request].service_time;
		return travel_cost*arc_lengths;
	} else {
		information[0] = arc_lengths + pickup_nodes[request].service_time;
		information[1] = arc_lengths_right + delivery_nodes[request].service_time;
		return travel_cost*(arc_lengths + arc_lengths_right);
	}
}

double Instance::costs_of_inserting_request_with_transfer_pickup(Vehicle &v, size_t p, size_t d, size_t request, Transfer_Node &tn, vector<double> &information) {
	double arc_lengths = 0.0, arc_lengths_right = 0.0;
	if (d == p + 1) {
		arc_lengths += arcs[pickup_nodes[request].gen_idx][tn.gen_idx]
						- arcs[v.route[p-1].gen_idx][v.route[p].gen_idx];
	} else {
		arc_lengths += arcs[pickup_nodes[request].gen_idx][v.route[p].gen_idx]
						- arcs[v.route[p-1].gen_idx][v.route[p].gen_idx];
		arc_lengths_right += arcs[v.route[d-2].gen_idx][tn.gen_idx]
							- arcs[v.route[d-2].gen_idx][v.route[d-1].gen_idx];
	}

	if (p > 1 && d < v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx]
							+ arcs[tn.gen_idx][v.route[d-1].gen_idx];
		} else {
			arc_lengths += arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx];
			arc_lengths_right += arcs[tn.gen_idx][v.route[d-1].gen_idx];
		}
	} else if (p > 1 && d == v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx]
							+ arcs[tn.gen_idx][nearest_depot_gen_idx_t[tn.index]];
		} else {
			arc_lengths += arcs[v.route[p-1].gen_idx][pickup_nodes[request].gen_idx];
			arc_lengths_right += arcs[tn.gen_idx][nearest_depot_gen_idx_t[tn.index]];
		}
	} else if (p == 1 && d < v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[tn.gen_idx][v.route[d-1].gen_idx]
							+ arcs[nearest_depot_gen_idx_p[request]][pickup_nodes[request].gen_idx];
		} else {
			arc_lengths += arcs[nearest_depot_gen_idx_p[request]][pickup_nodes[request].gen_idx];
			arc_lengths_right += arcs[tn.gen_idx][v.route[d-1].gen_idx];
		}
	} else if (p == 1 && d == v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[nearest_depot_gen_idx_p[request]][pickup_nodes[request].gen_idx]
							+ arcs[tn.gen_idx][nearest_depot_gen_idx_t[tn.index]];
		} else {
			arc_lengths += arcs[nearest_depot_gen_idx_p[request]][pickup_nodes[request].gen_idx];
			arc_lengths_right += arcs[tn.gen_idx][nearest_depot_gen_idx_t[tn.index]];
		}
	} else {
		cout << "Error calculating costs for transfer insertion!\n";
		cout << "Pickup transfer - p: " << p << ", d: " << d << ", route size:" << v.route.size() << "\n";
	}

	if (d == p + 1) {
		information[0] = arc_lengths + pickup_nodes[request].service_time + delivery_nodes[request].service_time;
		information[1] = arc_lengths + pickup_nodes[request].service_time + delivery_nodes[request].service_time;
		return travel_cost*arc_lengths;
	} else {
		information[0] = arc_lengths + pickup_nodes[request].service_time;
		information[1] = arc_lengths_right + delivery_nodes[request].service_time;
		return travel_cost*(arc_lengths + arc_lengths_right);
	}
}

double Instance::costs_of_inserting_request_with_transfer_delivery(Vehicle &v, size_t p, size_t d, size_t request, Transfer_Node &tn, vector<double> &information) {
	double arc_lengths = 0.0, arc_lengths_right = 0.0;
	if (d == p + 1) {
		arc_lengths += arcs[tn.gen_idx][delivery_nodes[request].gen_idx]
						- arcs[v.route[p-1].gen_idx][v.route[p].gen_idx];
	} else {
		arc_lengths += arcs[tn.gen_idx][v.route[p].gen_idx]
						- arcs[v.route[p-1].gen_idx][v.route[p].gen_idx];
		arc_lengths_right += arcs[v.route[d-2].gen_idx][delivery_nodes[request].gen_idx]
							- arcs[v.route[d-2].gen_idx][v.route[d-1].gen_idx];
	}

	if (p > 1 && d < v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[v.route[p-1].gen_idx][tn.gen_idx]
							+ arcs[delivery_nodes[request].gen_idx][v.route[d-1].gen_idx];
		} else {
			arc_lengths += arcs[v.route[p-1].gen_idx][tn.gen_idx];
			arc_lengths_right += arcs[delivery_nodes[request].gen_idx][v.route[d-1].gen_idx];
		}
	} else if (p > 1 && d == v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[v.route[p-1].gen_idx][tn.gen_idx]
							+ arcs[delivery_nodes[request].gen_idx][nearest_depot_gen_idx_d[request]];
		} else {
			arc_lengths += arcs[v.route[p-1].gen_idx][tn.gen_idx];
			arc_lengths_right += arcs[delivery_nodes[request].gen_idx][nearest_depot_gen_idx_d[request]];
		}
	} else if (p == 1 && d < v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[delivery_nodes[request].gen_idx][v.route[d-1].gen_idx]
							+ arcs[nearest_depot_gen_idx_t[tn.index]][tn.gen_idx];
		} else {
			arc_lengths += arcs[nearest_depot_gen_idx_t[tn.index]][tn.gen_idx];
			arc_lengths_right += arcs[delivery_nodes[request].gen_idx][v.route[d-1].gen_idx];
		}
	} else if (p == 1 && d == v.route.size()) {
		if (d == p + 1) {
			arc_lengths += arcs[nearest_depot_gen_idx_t[tn.index]][tn.gen_idx]
							+ arcs[delivery_nodes[request].gen_idx][nearest_depot_gen_idx_d[request]];
		} else {
			arc_lengths += arcs[nearest_depot_gen_idx_t[tn.index]][tn.gen_idx];
			arc_lengths_right += arcs[delivery_nodes[request].gen_idx][nearest_depot_gen_idx_d[request]];
		}
	} else {
		cout << "Error calculating costs for transfer insertion!\n";
		cout << "Delivery transfer - p: " << p << ", d: " << d << ", route size:" << v.route.size() << "\n";
	}
	if (d == p + 1) {
		information[0] = arc_lengths + pickup_nodes[request].service_time + delivery_nodes[request].service_time;
		information[1] = arc_lengths + pickup_nodes[request].service_time + delivery_nodes[request].service_time;
		return travel_cost*arc_lengths;
	} else {
		information[0] = arc_lengths + pickup_nodes[request].service_time;
		information[1] = arc_lengths_right + delivery_nodes[request].service_time;
		return travel_cost*(arc_lengths + arc_lengths_right);
	}
}
