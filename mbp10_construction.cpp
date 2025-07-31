#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;

struct Order {
	string ts_recv;	
	string ts_event; 
	uint16_t rtype;	
	uint16_t publisher_id;
 	uint32_t instrument_id;
	char action; 
	char side;	
	double price; 
	uint32_t size; 
	uint16_t channel_id;
	uint64_t order_id; 
	uint16_t flags; 
	string ts_in_delta;
	uint32_t sequence;
	string symbol;
};

struct Price {
	string ts_recv;	
	string ts_event; 
	uint16_t rtype;	
	uint16_t publisher_id;
 	uint32_t instrument_id;
	char action; 
	char side;	
	double price; 
	uint32_t size; 
	uint16_t flags;		
	string ts_in_delta;
	uint32_t sequence;
	string symbol;
	uint64_t order_id;
	double bid_px_N[10];
	double ask_px_N[10];			
	double bid_sz_N[10];			
	double ask_sz_N[10];			
	double bid_ct_N[10];			
	double ask_ct_N[10];		
};

map<double, pair<int,int> , greater<double>> bids;   // Top bids (descending)  bidPrice -> {orderCount, shareCount}
map<double, pair<int,int>> asks;                    // Top asks (ascending)		askPrice -> {orderCount, shareCount}

vector<string> split_csv_line(const string& line) {
    vector<string> result;
    stringstream ss(line);
    string item;

    while (getline(ss, item, ',')) {
        result.push_back(item);
    }
    return result;
}

Order parse_order(const vector<string>& fields) {
    Order order;
    order.ts_recv = fields[0];
    order.ts_event = fields[1];
    order.rtype = static_cast<uint16_t>(stoi(fields[2]));
    order.publisher_id = static_cast<uint16_t>(stoi(fields[3]));
    order.instrument_id = static_cast<uint32_t>(stoul(fields[4]));
    order.action = fields[5][0];
    order.side = fields[6][0];
    order.price = stod(fields[7]);
    order.size = static_cast<uint32_t>(stoul(fields[8]));
    order.channel_id = static_cast<uint16_t>(stoi(fields[9]));
    order.order_id = stoull(fields[10]);
    order.flags = static_cast<uint16_t>(stoi(fields[11]));
    order.ts_in_delta = fields[12];
    order.sequence = static_cast<uint32_t>(stoul(fields[13]));
    order.symbol = fields[14];
    return order;
}

Price updatePrice(Order order) {
	Price price;
	price.ts_recv = order.ts_recv;
	price.ts_event = order.ts_event;
	price.rtype = order.rtype;
	price.publisher_id = order.publisher_id;
	price.instrument_id = order.instrument_id;
	price.action = order.action;
	price.side = order.side;
	price.price = order.price;
	price.size = order.size;
	price.flags = order.flags;
	price.ts_in_delta = order.ts_in_delta;
	price.sequence = order.sequence;
	price.symbol = order.symbol;
	price.order_id = order.order_id;
	int bidInd = 0, askInd = 0;
	for(auto bid : bids) {
		price.bid_px_N[bidInd] = bid.first;
		price.bid_ct_N[bidInd] = bid.second.first;
		price.bid_sz_N[bidInd] = bid.second.second;
		bidInd++;
		if(bidInd == 10) break;
	}
	for(auto ask : asks) {
		price.ask_px_N[askInd] = ask.first;
		price.ask_ct_N[askInd] = ask.second.first;
		price.ask_sz_N[askInd] = ask.second.second;
		askInd++;
		if(askInd == 10) break;
	}
	for(int i = bidInd; i<10; i++) {
		price.bid_px_N[i] = -1;
		price.bid_ct_N[i] = -1;
		price.bid_sz_N[i] = -1;
	}
	for(int i = askInd; i<10; i++) {
		price.ask_px_N[i] = -1;
		price.ask_ct_N[i] = -1;
		price.ask_sz_N[i] = -1;
	}
	return price;
}

pair<Price, bool> proessOrder(Order order)  {
	bool isBookChanged = false;
	if(order.action == 'T') {
		isBookChanged = true;
	} else if(order.side == 'B') { // buy/bid
		if(order.action == 'A') {
			if(bids.find(order.price) == bids.end()) {
				bids[order.price] = {1, order.size};
				isBookChanged = true;
			} else {
				isBookChanged = true;
				auto existingOrder = bids[order.price];
				bids[order.price] = {existingOrder.first + 1, existingOrder.second + order.size};
			}
		} else if(order.action == 'C') {
			if(bids.find(order.price) != bids.end()) {
				isBookChanged = true;
				auto existingOrder = bids[order.price];
				bids[order.price] = {existingOrder.first - 1, existingOrder.second - order.size};
				if(existingOrder.first <= 1) {
					bids.erase(order.price);
				}
			}
		}
		
	} else if(order.side == 'A') { //sell
		if(order.action == 'A') {
			isBookChanged = true;
			if(asks.find(order.price) == asks.end()) {
					isBookChanged = true;
					asks[order.price] = {1, order.size};
			} else {
				isBookChanged = true;
				auto existingOrder = asks[order.price];
				asks[order.price] = {existingOrder.first + 1, existingOrder.second + order.size};
			}
		} else if(order.action == 'C') {
			if(asks.find(order.price) != asks.end()) {
				isBookChanged = true;
				auto existingOrder = asks[order.price];
				asks[order.price] = {existingOrder.first - 1, existingOrder.second - order.size};
				if(existingOrder.first <= 1) {
					asks.erase(order.price);
				}
			}
		}
	}
	return {updatePrice(order), isBookChanged};
}
void writeOuputHeader(ofstream& out) {
	out << "ts_recv" <<"," << "ts_event" << "," << "rtype" 
	<< "," << "publisher_id" << "," << "instrument_id" << "," 
	<< "action" << "," << "side" <<"," << "depth" << "," << "price" << "," 
	<< "size" << "," << "flags" << "," << "ts_in_delta" << "," 
	<< "sequence"; 
	out << ","<<"bid_px_00"<<","<<"bid_sz_00"<<","<<"bid_ct_00";
	out << ","<<"ask_px_00"<<","<<"ask_sz_00"<<","<<"ask_ct_00";
	out << ","<<"bid_px_01"<<","<<"bid_sz_01"<<","<<"bid_ct_01";
	out << ","<<"ask_px_01"<<","<<"ask_sz_01"<<","<<"ask_ct_01";
	out << ","<<"bid_px_02"<<","<<"bid_sz_02"<<","<<"bid_ct_02";
	out << ","<<"ask_px_02"<<","<<"ask_sz_02"<<","<<"ask_ct_02";
	out << ","<<"bid_px_03"<<","<<"bid_sz_03"<<","<<"bid_ct_03";
	out << ","<<"ask_px_03"<<","<<"ask_sz_03"<<","<<"ask_ct_03";
	out << ","<<"bid_px_04"<<","<<"bid_sz_04"<<","<<"bid_ct_04";
	out << ","<<"ask_px_04"<<","<<"ask_sz_04"<<","<<"ask_ct_04";
	out << ","<<"bid_px_05"<<","<<"bid_sz_05"<<","<<"bid_ct_05";
	out << ","<<"ask_px_05"<<","<<"ask_sz_05"<<","<<"ask_ct_05";
	out << ","<<"bid_px_06"<<","<<"bid_sz_06"<<","<<"bid_ct_06";
	out << ","<<"ask_px_06"<<","<<"ask_sz_06"<<","<<"ask_ct_06";
	out << ","<<"bid_px_07"<<","<<"bid_sz_07"<<","<<"bid_ct_07";
	out << ","<<"ask_px_07"<<","<<"ask_sz_07"<<","<<"ask_ct_07";
	out << ","<<"bid_px_08"<<","<<"bid_sz_08"<<","<<"bid_ct_08";
	out << ","<<"ask_px_08"<<","<<"ask_sz_08"<<","<<"ask_ct_08";
	out << ","<<"bid_px_09"<<","<<"bid_sz_09"<<","<<"bid_ct_09";
	out << ","<<"ask_px_09"<<","<<"ask_sz_09"<<","<<"ask_ct_09"<< ","<<"symbol"<<","<<"order_id";
	out<<endl;
}

void writePriceToOutputFile(Price price , ofstream& out) {

	out << price.ts_recv <<"," << price.ts_event << "," << price.rtype 
	<< "," << price.publisher_id << "," << price.instrument_id << "," 
	<< price.action << "," << price.side << "," << 0 << "," << price.price << "," 
	<< price.size << "," << price.flags << "," << price.ts_in_delta << "," 
	<< price.sequence ;

	for(int i=0; i<10; i++) {
		if(price.bid_px_N[i] == -1) {
			out<<",,,";
		} else {
			out<<","<<price.bid_px_N[i]<<","<<price.bid_sz_N[i]<<","<<price.bid_ct_N[i];
		}
		if(price.ask_px_N[i] == -1) {
			out<<",,,";
		} else {
			out<<","<<price.ask_px_N[i]<<","<<price.ask_sz_N[i]<<","<<price.ask_ct_N[i];
		}
	}
	out << "," << price.symbol <<"," << price.order_id<<endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Error: Please provide a filename.\n";
        return 1;
    }

    ifstream inputFile(argv[1]);
    ofstream outputFile("outtest.csv");

    if (!inputFile.is_open()) {
        cerr << "Error: Could not open file " << argv[1] << "\n";
        return 1;
    }
    string line;
    getline(inputFile, line);
    getline(inputFile, line);
    writeOuputHeader(outputFile);
    string prevLine, line2;
	vector<string> fields;

	Order prevT, prevF;
	bool waitingForFillCancel = false;

	while (getline(inputFile, line)) {
	    if (line.empty()) continue;
	    fields = split_csv_line(line);
	    if (fields.size() != 15) {
	        cerr << "Skipping invalid line: " << line << endl;
	        continue;
	    }

	    Order current = parse_order(fields);

	    if (waitingForFillCancel && current.action == 'F') {
	        prevF = current;
	        continue;
	    }

	    if (waitingForFillCancel && current.action == 'C') {
	        // Construct synthetic cancel order based on previous T and current C
	        Order synthetic = prevT;
	        if (synthetic.side == 'N') {
	            waitingForFillCancel = false;
	            continue; // Skip invalid T actions
	        }

	        // Flip side
	        synthetic.side = (synthetic.side == 'A') ? 'B' : 'A';
	        synthetic.action = 'C';
	        synthetic.size = current.size;      // From C row
	        synthetic.order_id = current.order_id; // Optional but for completeness

	        auto processed = proessOrder(synthetic);
	        if (processed.second)
	            writePriceToOutputFile(processed.first, outputFile);

	        waitingForFillCancel = false;
	        continue;
	    }

	    if (current.action == 'T') {
	        prevT = current;
	        waitingForFillCancel = true;
	        continue;
	    }

	    // If not part of T-F-C, process normally (A or C)
	    auto processed = proessOrder(current);
	    if (processed.second)
	        writePriceToOutputFile(processed.first, outputFile);
}

    return 0;
}
