/*
 * The MIT License (MIT)
 * Copyright (c) <2016> <Mark McKenney>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * */


#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include "parPlaneSweep.h"
#include "d2hex.h"
#include <fstream>
using namespace std;

/**
 * Function that tokenizes a string.  
 * @param [in] str: the string to tokenize
 * @param [out] tokens: a vecotr of string tokens
 * @param [in] delimiters:  A string consisting of delimiters on which to tokenize
 */
void tokenizeString(const std::string& str, 
        std::vector<string>& tokens, 
        const string& delimiters );


/**
 * The main function provides examples of how to call the serial and 
 * parallel versions of the plane sweep algorithm.  This code implements
 * a O((n \lg n) + (k \lg k)) version of the algorithm, that returns a 
 * sorted list of halfsegments. 
 *
 * This particular function expects input in a hexadecimal format, and includes code
 * to extract floating point numbers from exadecimal representation.
 *
 * The command line arguments required are:
 *  - [an input hex file with region 1]
 *  - [an input hex file with region 2]
 *  - [the number of strips to begin running the program with]
 *  - [the number of strips to stop at]
 *
 *  The program repeatedly runs a plan sweep algorithm on the input
 *  with increasing numbers of strips. Strip counts increase quadratically. 
 */
int main( int argc, char * argv[] ) 
{

    // read the argument regions
    std::string inputFileName1, inputFileName2;
    vector< halfsegment >v1, v2, result;
    int minStrips, maxStrips;
    if( argc != 5 )
    {
        std::cerr << "usage: exe  [input file name 1] [input file name 2] [min strips][max strips]" << std::endl;
        exit( -1 );
    }
    {
        std::stringstream ss1;
        ss1 << argv[1];
        ss1 >> inputFileName1;
    }
    {
        std::stringstream ss1;
        ss1 << argv[2];
        ss1 >> inputFileName2;
    }
    {
        std::stringstream ss1;
        ss1 << argv[3];
        ss1 >> minStrips;
    }
    {
        std::stringstream ss1;
        ss1 << argv[4];
        ss1 >> maxStrips;
    }

    ifstream inFileStrm1;
    inFileStrm1.open( argv[1] );
    if( ! inFileStrm1 )
    {
        cerr << "Error: could not open file: " << argv[1] << endl;
        exit( -1 );
    }
    ifstream inFileStrm2;
    inFileStrm2.open( argv[2] );
    if( ! inFileStrm2 )
    {
        cerr << "Error: could not open file: " << argv[2] << endl;
        exit( -1 );
    }

    cerr << "Reading files: " << argv[1] << ", " <<argv[2] << endl;
    vector<string> splitLine;
    string line("  ");
    double x, y;
    int la, lb;
    int i = 0;
    while(inFileStrm1.good() )
    {
        getline(inFileStrm1, line);
        if( inFileStrm1.good() )
        {
            if( line.size() == 0 || line[0] == '#' )
                continue;
            splitLine.clear();
            string delim(" \t" );
            tokenizeString( line, splitLine, delim );
            v1.push_back( halfsegment() );
            v1[i].dx = doubleHexConverter::hex2d(splitLine[0]);
            v1[i].dy = doubleHexConverter::hex2d(splitLine[1]);
            v1[i].sx = doubleHexConverter::hex2d(splitLine[2]);
            v1[i].sy = doubleHexConverter::hex2d(splitLine[3]);
            stringstream ss1( splitLine[4] );
            ss1 >> la;
            stringstream ss2( splitLine[5] );
            ss2 >> lb;
            v1[i].la = la;
            v1[i].ola = la;
            v1[i].lb = lb;
            v1[i].olb = lb;
            v1[i].regionID = 2;
            i++;
            v1.push_back( v1[i-1].getBrother() );
            i++;
        }
    }
    cerr <<"file 1 finished reading"<<endl;
    i = 0;
    while(inFileStrm2.good() )
    {
        getline(inFileStrm2, line);
        if( inFileStrm2.good() )
        {
            if( line.size() == 0 || line[0] == '#' )
                continue;
            splitLine.clear();
            string delim(" \t" );
            tokenizeString( line, splitLine, delim );
            v2.push_back( halfsegment() );
            v2[i].dx = doubleHexConverter::hex2d(splitLine[0]);
            v2[i].dy = doubleHexConverter::hex2d(splitLine[1]);
            v2[i].sx = doubleHexConverter::hex2d(splitLine[2]);
            v2[i].sy = doubleHexConverter::hex2d(splitLine[3]);
            stringstream ss1( splitLine[4] );
            ss1 >> la;
            stringstream ss2( splitLine[5] );
            ss2 >> lb;
            v2[i].la = la;
            v2[i].ola = la;
            v2[i].lb = lb;
            v2[i].olb = lb;
            v2[i].regionID = 3;
            i++;
            v2.push_back( v2[i-1].getBrother() );
            i++;
        }
    }
    cerr <<"file 2 finished reading"<<endl;


    std::sort( v1.begin(), v1.end() );
    std::sort( v2.begin(), v2.end() );
    if( minStrips < 1 ) {
        minStrips = 1;
    }
    for( int i = minStrips; i <= maxStrips; i= (i==1)? 2: i*2 ){
        // start the timer
        cout << "TTT num strips: " << i << endl;
        if( i == 1 ){
            overlayPlaneSweep( &(v1[0]), v1.size(), &(v2[0]), v2.size(), result );
        }
        else {
            parallelOverlay( v1, v2, result, i);
        }
        cout << "num segs: " << result.size()/2<<endl;

    }

}


void tokenizeString(const std::string& str, std::vector<string>& tokens, const string& delimiters )
{
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);	// Skip delimiters at beginning.
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);	// Find first "non-delimiter".

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        tokens.push_back(str.substr(lastPos, pos - lastPos));	        // Found a token, add it to the vector.
        lastPos = str.find_first_not_of(delimiters, pos);		// Skip delimiters.  Note the "not_of"
        pos = str.find_first_of(delimiters, lastPos);			// Find next "non-delimiter"
    }
}

