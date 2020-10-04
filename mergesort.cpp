#include <iostream>
#include <vector>
#include <deque>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <chrono>

using namespace std;

void LoadBuffer(ifstream &infile,deque <int> &buffer,int bufMax){
    string s;
    while(getline(infile,s)){
        buffer.push_back(stoi(s));
        if(buffer.size() == bufMax){
            return;
        }
    }
    return;
}

void DumpBuffer(deque<int> &buffer,string outputFileName){
    ofstream outfile(outputFileName,ofstream::app);
    if(!outfile){
        cerr << "fail to open file:" << outputFileName <<endl;
        exit(1);
    }

    for(int i = 0 ;i < buffer.size();++i){
        outfile << buffer.at(i) << "\n";
    }
    buffer.clear();
    outfile.close();
}

void DumpBufferOverride(vector<int> &buffer,string outputFileName){
    ofstream outfile(outputFileName,ofstream::out);
    for(int i = 0 ;i < buffer.size();++i){
        outfile << buffer.at(i) << "\n";
    }
    buffer.clear();
    outfile.close();
}

void CopyRestFile(ifstream &infile, deque<int> &buffer, int bufMax,string outputFileName){
    while(1){
        LoadBuffer(infile,buffer,bufMax);//load to output buffer
        DumpBuffer(buffer,outputFileName);
        if(infile.eof()){
            return;
        }
    }
}


void MergeFile(string filename1, string filename2,int subtextNum,int round,int memSize){
    size_t inBufNum = memSize *  (1000000000 / 16);//use quarter of memory and one int is 4 byte
    size_t outBufNum = memSize * (1000000000 / 8 );//use half of memory and one int is 4 byte
    string subfileName = to_string(round) + "_" + to_string(subtextNum) +".txt";//filename
    ifstream infile1(filename1);
    if(!infile1){
        cerr << "fail to open file:" << filename1 <<endl;
        exit(1);
    }

    ifstream infile2(filename2);
    if(!infile2){
        cerr << "fail to open file:" << filename2 <<endl;
        exit(1);
    }


    deque <int> inBuffer1,inBuffer2,outBuffer;

    while(1){//determind end of file with infinite loop with eof()
        //repeatly fill input buffer
        if(inBuffer1.empty()){
            LoadBuffer(infile1,inBuffer1,inBufNum);
        }
        if(inBuffer2.empty()){
            LoadBuffer(infile2,inBuffer2,inBufNum);
        }
       
        // test if one of file reach end
        if(infile1.eof() && inBuffer1.size() == 0){//all data in file1 is processed
            DumpBuffer(outBuffer,subfileName);//dump buffer first in case something left
            DumpBuffer(inBuffer2,subfileName);//dump input buffer2
            CopyRestFile(infile2,outBuffer,outBufNum,subfileName);
            break;
        }
        if(infile2.eof() && inBuffer2.size() == 0){//all data in file2 is processed
            DumpBuffer(outBuffer,subfileName);
            DumpBuffer(inBuffer1,subfileName);//dump input buffer1 
            CopyRestFile(infile1,outBuffer,outBufNum,subfileName);
            break;
        }
        //merge sort
        if(inBuffer1.front() < inBuffer2.front()){ // if first element in buffer1 > buffer2
            outBuffer.push_back(inBuffer1.front());
            inBuffer1.pop_front();//pop first element in buffer1
        }
        else{
            outBuffer.push_back(inBuffer2.front());
            inBuffer2.pop_front();
        }
		
        if(outBuffer.size() >= outBufNum){//output buffer full
            DumpBuffer(outBuffer,subfileName);
        }
    }
    infile1.close();
    infile2.close();
    remove(filename1.c_str());//delete redundant file
    remove(filename2.c_str());
    return;
}

int main(int argc,char * argv[]){
	chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();//timer start    
	//input file segmantation and sort
	int memSize = atoi(argv[1]);
	char*  inputFilePath = argv[2];
    size_t bufNum = memSize *(1000000000 / 8);//use half of memory and one int is 4 byte
    vector<int> buffer;
	buffer.reserve(bufNum);

    ifstream infile(inputFilePath);
    if(!infile){
        cerr << "fail to open file:test.txt"  <<endl;
        exit(1);
    }
    string readtext;
    int subtextNum = 0;
    int round = 0;
    
    remove("output.txt");//remove previous result
	
	int i = 0;
    while (getline (infile, readtext)) {
        buffer.push_back(stoi(readtext));
		++i;

        if(i == bufNum){//buffer full dump buffer
            sort(buffer.begin(),buffer.end()); //call lib function to sort buffer
            string subfileName = "0_" + to_string(subtextNum) +".txt";//filename
            DumpBufferOverride(buffer,subfileName);
			i = 0;
            ++subtextNum;
        }
    }
    if(i > 0){//sort remainder
        sort(buffer.begin(),buffer.end()); //call lib function to sort buffer
        string subfileName = "0_" + to_string(subtextNum) +".txt";//filename
        DumpBufferOverride(buffer,subfileName);
        ++subtextNum;
    }
    infile.close();
    // finish file segmantation

    //compute loop boundary
    int roundTextNum = subtextNum; //loop boundary for each rounds
    int maxRound = static_cast<int>(log2(static_cast<float>(roundTextNum)));//count how many rounds it need to do 2-way file merge

    //iterative merge file 
    for(int i = 0;i < maxRound;++i){
        subtextNum = 0;
        if (roundTextNum % 2 == 0){ //even files
            for(int j = 0;j < roundTextNum;j = j + 2){
                string inFileName1 =  to_string(i) + "_" + to_string(j) +".txt";
                string inFileName2 =  to_string(i) + "_" + to_string(j + 1) +".txt";
                MergeFile(inFileName1,inFileName2,subtextNum,i + 1,memSize);
                ++subtextNum;
            }
        }
        else{//odd files
            for(int j = 0;j < roundTextNum - 1; j = j +2){//sort the first even files first
                string inFileName1 =  to_string(i) + "_" + to_string(j) +".txt";
                string inFileName2 =  to_string(i) + "_" + to_string(j + 1) +".txt";
                MergeFile(inFileName1,inFileName2,subtextNum,i + 1,memSize);
                ++subtextNum;
            }
            //last file
            string inFileName1 =  to_string(i) + "_" + to_string(roundTextNum - 1) +".txt";// last file ex:0_4.txt
            string inFileName2 =  to_string(i + 1) + "_" + to_string(subtextNum - 1) +".txt";// last file in next round ex:1_1.txt
            MergeFile(inFileName1,inFileName2,subtextNum,i + 1,memSize);
            string oldFileName =  to_string(i + 1) + "_" + to_string(subtextNum) +".txt";
            string newFileName =  to_string(i + 1) + "_" + to_string(subtextNum - 1) +".txt";
            rename(oldFileName.c_str(), newFileName.c_str());
            
        }
        roundTextNum = subtextNum; //loop boundary for each rounds
    }
    rename((to_string(maxRound) + "_0.txt").c_str(),"output.txt");//rename to match assignment requirement
	chrono::steady_clock::time_point end = std::chrono::steady_clock::now();


    cout << "Elapsed time in milliseconds : "<< chrono::duration_cast<chrono::milliseconds>(end - begin).count()<< " ms" << endl;    
	return 0;
}
