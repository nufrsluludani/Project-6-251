//
// STARTER CODE: util.h
//
// Author: Daniel Luangnikone
// System: Windows 11 VSC
// Description: Making a file compression program using huffsman algorithm
//

#include <fstream>
#include <functional>  
#include <iostream>
#include <queue> 
#include <string>
#include <vector>  

#include "bitstream.h"
#include "hashmap.h"

#pragma once

typedef hashmap hashmapF;
typedef unordered_map <int, string> hashmapE;

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};

struct compare
{
    bool operator()(const HuffmanNode *lhs, const HuffmanNode *rhs)
    {
        return lhs->count > rhs->count;
    }
};

//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
    if(node == nullptr){
        return;
    }
    freeTree(node->zero);
    freeTree(node->one);
    delete node;
}

//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmapF &map) {
    if(isFile){
        ifstream inFS(filename);
        char c;
        while(inFS.get(c)){
            if(map.containsKey(int(c))){
                map.put(int(c), map.get(c) + 1);
            } 
            else{
                map.put(int(c), 1);
            }
        }
        inFS.close();
    }
    else{
        for(char c : filename){
            if(map.containsKey(int(c))){
                map.put(int(c), map.get(c) + 1);
            } 
            else{
                map.put(int(c), 1);
            }
        }
    }
    map.put(256, 1);
}

//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmap& map) {

    priority_queue<HuffmanNode*, vector<HuffmanNode*>, compare> pq;

    for (int keys : map.keys()) {
        HuffmanNode* node = new HuffmanNode();
        node->character = keys;
        node->count = map.get(keys);
        node->zero = nullptr;
        node->one = nullptr;
        pq.push(node);
    }
    while(pq.size() > 1){
        HuffmanNode* first = pq.top();
        pq.pop();
        HuffmanNode* second = pq.top();
        pq.pop();
        HuffmanNode* newNode = new HuffmanNode();
        newNode->character = 257;
        newNode->count = first->count + second->count;
        newNode->zero = first;
        newNode->one = second;
        pq.push(newNode);
    }
    return pq.top(); // implies that returning the root which is the only node left over where it is at the top hence pq.top()
}

//
// *Recursive helper function for building the encoding map.
//
void _buildEncodingMap(HuffmanNode* node, hashmapE &encodingMap, string str, HuffmanNode* prev) {
    
    prev = node;
    if (node->character != 257) {
        encodingMap.insert({int(node->character), str});
    }
    if (node->zero != nullptr) {
        _buildEncodingMap(node->zero, encodingMap, str += "0", prev);
    }
    str.pop_back();
    if (node->one != nullptr) {
        _buildEncodingMap(node->one, encodingMap, str += "1", prev);
    }
    
}

//
// *This function builds the encoding map from an encoding tree.
//
hashmapE buildEncodingMap(HuffmanNode* tree) {
    hashmapE encodingMap;

    HuffmanNode* node = tree;
    string string = "";
    _buildEncodingMap(node, encodingMap, string, node);
    
    return encodingMap;  // TO DO: update this return
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, hashmapE &encodingMap, ofbitstream& output, int &size, bool makeFile) {

    string result = "";
    char c;
    while (input.get(c)){
        result += encodingMap[int(c)];
        size += encodingMap[int(c)].size();
    }
    result += encodingMap[256]; // insert psuedo_eof 
    size += encodingMap[256].size();

    if(makeFile){
        for (char c : result){
            output.writeBit(c == '0' ? 0 : 1);
        }
    }
    return result;
}


//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
    
    string result = "";
    HuffmanNode* node = encodingTree;

    while (!input.eof()) {
        if (node->character != 257) { // leaf
            if (node->character == 256) { // if psuedo eof just break out 
                return result;
            } 
            else {
                output.put(char(node->character)); 
                result += char(node->character); // add to string
                node = encodingTree; // reset to root
            }
        }
        int bit = input.readBit();
        if (bit == 0) { // if 0 go left
            node = node->zero;
        } 
        else { // if 1 go right
            node = node->one;
        }
    }
    return result;
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename) {
    
    string result = "";
    int size = 0;

    hashmap frequencyMap;
    buildFrequencyMap(filename, true, frequencyMap);
    HuffmanNode *encodingTree = buildEncodingTree(frequencyMap);
    hashmapE encodingMap = buildEncodingMap(encodingTree);

    ofbitstream output(filename + ".huf");

    output << frequencyMap;

    ifstream input(filename);

    result = encode(input, encodingMap, output, size, true);

    input.close();
    output.close();
    freeTree(encodingTree);

    return result;  // TO DO: update this return
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function did.
//
string decompress(string filename) {

    string result = "";

    hashmap frequencyMap;

    ifbitstream input(filename);

    ofstream output(filename.substr(0,filename.size()-8) + "_unc.txt");
    
    input >> frequencyMap; 

    HuffmanNode *encodingTree = buildEncodingTree(frequencyMap);

    result = decode(input, encodingTree, output);

    input.close();
    output.close();
    freeTree(encodingTree);
    
    return result;  // TO DO: update this return
}
