//
//  main.cpp
//  Created by Lyla Zegelstein on 11/12/21.
//

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <queue>
#include <filesystem>

using std::vector;
using std::map;
using std::string;
using std::queue;

void readInTableSchemas(std::string fileName, map<string, vector<string> > & table);
void tableParser(string line, map<string, vector<string> > & table);
string extractWord(string line, int start);
void nestedLoopJoin(string input1, string input2, map<string, vector<string> > tableSchema);
void getTuples(map<string, vector<string> > table, string fileName, string fileName2, vector<string> condition);
int findPosition(map<string, vector<string> > table, string fileName, string line, string condition);



int main(int argc, const char * argv[]) {
    map<string, vector<string> > tableSchema;
    readInTableSchemas("./tableNames.txt", tableSchema);
    nestedLoopJoin("Nation", "Region", tableSchema);
    return 0;
}


/**
 PHASE 1: READING IN THE TABLE SCHEMA'S AND STORING IT IN THE TABLE MAP
 */
void readInTableSchemas(std::string fileName, map<string, vector<string> > & table) {
    std::fstream data_file;
    std::string line;
    data_file.open(fileName, std::ios::in);

    if (data_file.is_open()) {

        while (std::getline(data_file, line)) {
            tableParser(line, table);
        }
        data_file.close();
    }
    else {
        std::cout<<"wrong file path"<<std::endl;
    }
}

/**
In this function, I need to first find the table name, then find all of the column names
The column names get added to a vector
Finally,  a pair is made between the table name and vector holding the column names
 */

void tableParser(string line, map<string, vector<string> > & table) {
    vector<string> columns;
    string tableName;
    
    for (int i = 0; i < line.size(); i ++) {
        if (line.at(i) == ' ' || line.at(i) == ',') {
            continue;
        }
        if (i == 0) {
            tableName = extractWord(line, 0);
            i += tableName.size() + 2;
            continue;
        }
        string word = extractWord(line, i);
        i += word.size();
        columns.push_back(word);
    }
    //Now need to add tableName as the key and the vector of columns as the value
    table.insert(std::pair<string, vector<string> >(tableName, columns));
}

string extractWord(string line, int start) {
    string word = "";
    
    for (int i = start; i < line.size(); i ++) {
        if (line.at(i) == ',' || line.at(i) == '|') {
            return word;
        }
        word.push_back(line.at(i));
    }
    return word;
}

/**
 PHASE 2: Performing the nestedJoin
 */

//example call: nestedLoopJoin(Part, Partsupp, tableSchema)
void nestedLoopJoin(string input1, string input2, map<string, vector<string> > tableSchema) {
    //step 1: find the columns to join on
    std::vector<string> condition;
    //use the key to find the vector of values and then search through for matches
    //anytime we see a match, push the column name to the condition vector
    for (auto i = tableSchema[input1].begin(); i != tableSchema[input1].end(); i ++) {
        for (auto j = tableSchema[input2].begin(); j != tableSchema[input2].end(); j ++) {
            if (*i == *j) {
                condition.push_back(*i);
            }
        }
    }
    //printing out the join condition
    for (auto w = condition.begin(); w != condition.end(); w ++) {
        std::cout<<"Joining on condition: "<<*w<<std::endl;
    }
    getTuples(tableSchema, input1, input2, condition);
}

string getFilePath(string fileName) {
    std::cout<<"fileName: "<<fileName<<std::endl;
    
    if (fileName == "lineitem" || fileName == "Lineitem") {
        return "./lineitem.tbl";
    }
    else if (fileName == "customer" || fileName == "customers") {
        return "./customer.tbl";
    }
    else if (fileName == "nation" || fileName == "Nation") {
        return "./nation.tbl";
    }
    else if (fileName == "orders" || fileName == "Orders") {
        return "./orders.tbl";
    }
    else if (fileName == "Part" || fileName == "part") {
        return "./part.tbl";
    }
    else if (fileName == "Partsupp" || fileName == "partsupp") {
        return "./partsupp.tbl";
    }
    else if (fileName == "region" || fileName == "Region") {
        return "./region.tbl";
    }
    else if (fileName == "supplier" || fileName == "Supplier") {
        return "./supplier.tbl";
    }
    return "ERROR";
}


void getTuples(map<string, vector<string> > table, string key, string key2, vector<string> condition) {
    std::fstream data_file;
    std::string line;
    std::fstream data_file2;
    std::string line2;
    string tuple1;
    int pos1;
    string tuple2;
    int pos2;
    string fileName = getFilePath(key);
    string fileName2 = getFilePath(key2);
    queue<string> joined;
    
    data_file.open(fileName, std::ios::in);
    
    if (data_file.is_open()) {
        while (std::getline(data_file, line)) {
            data_file2.open(fileName2, std::ios::in);
            if (data_file2.is_open()) {
                while (std::getline(data_file2, line2)) {
                    for (auto i = condition.begin(); i != condition.end(); i++) {
                        //must find the position in the line to check
                        pos1 = findPosition(table, key, line, *i);
                        string col1 = extractWord(line, pos1);
                        pos2 = findPosition(table, key2, line2, *i);
                        string col2 = extractWord(line2, pos2);
                        //if both satisfy the join condition
                        if (col1 == col2) {
                            //write the string to the output file
                            //Output the first line completely and then output everything in the second line except for the shared column
                            string output = line + line2.substr(0, pos2) + line2.substr(pos2 + 1, line2.size() - pos2 - col2.size()) + "\n";
                            //std::cout<<"output: "<<output<<std::endl;
                            joined.push(output);
                        }
                    }
                }
                data_file2.close();
            }
        }
        data_file.close();
        std::ofstream myfile;
        myfile.open("./output.txt");
        
        while (!joined.empty()) {
            myfile<<joined.front();
            joined.pop();
        }
        myfile.close();
    }
    else {
        std::cout<<"check spelling"<<std::endl;
    }
}

//string extractWord(string line, int start)
int findPosition(map<string, vector<string> > table, string fileName, string line, string condition) {
    int col = 0;
    for (auto i = table[fileName].begin(); i != table[fileName].end(); i ++) {
        //must find the condition
        if (*i == condition) {
            break;
        }
        col ++;
    }
    
    if (col == 0) {
        return 0;
    }
    
    for (int i = 0; i < line.size(); i ++) {
        if (line.at(i) == '|' && col > 0) {
            col --;
        }
        if (col == 0) {
            return i + 1;
        }
    }
    return -1;
}



