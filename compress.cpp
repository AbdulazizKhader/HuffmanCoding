#include "HCTree.hpp"
#include "Helper.hpp"
#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
using namespace std;


int main(int argc, char* argv[]){
    if(argc != 3){
        error("Incorrect parameters. \nUSAGE: ./compress <original_file> <compressed_file>.");    
    }

    FancyInputStream encodeFile(argv[1]);
    if(!encodeFile.good()){
        error("File not found.");
    }
    FancyOutputStream destFile(argv[2]);
    if(!destFile.good()){
        error("Unable to write to destination file.");
    }

    //Reading the characters one byte at a time if it was characters
    int fileSize = encodeFile.filesize();
    vector<int> frequencies(256,0);
    int symbolFreq;
    while((symbolFreq = encodeFile.read_byte()) != -1){
        frequencies[symbolFreq]++;
    }
    
    //Now that we have the frequencies, we can create the Huffman Tree
    HCTree HuffmanTree;
    HuffmanTree.build(frequencies);

    //If the Huffman Tree is empty then the file is empty
    if(HuffmanTree.getRoot() == nullptr){
        return 0;
    }

    /**
     * Now write the file header. Consists of the frequencies we need to make the Huffman Tree.
     * If the file size is < 10MB we use the optimal encoding. Else we use normal encoding.
     */ 
    bool greaterThan10MB = false;
    if(fileSize > 10000000){
        greaterThan10MB = true;
        destFile.write_bit(1);
    }
    else{
        destFile.write_bit(0);
    }
    destFile.flush_bitwise();

    for(int i = 0; i < 256; i++){
        if(greaterThan10MB){
            destFile.write_int(frequencies[i]);
        }
        else{
            destFile.write_int_optimal(frequencies[i]);
        }
    }

    //Now that we have the tree, encode the file
    //Resetting the input stream to be able to encode the characters
    encodeFile.reset();
    if(!encodeFile.good()){
        error("File is not good. Check flags.");
    }

    for(int i = 0; i < fileSize; i++){
        char letterToEncode = encodeFile.read_byte();
        HuffmanTree.encode(letterToEncode, destFile);
    }

    return 0;
}