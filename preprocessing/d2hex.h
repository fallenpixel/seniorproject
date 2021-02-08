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
#include <string>
#include <cstring>
using namespace std;

#ifndef D2HEX_H
#define D2HEX_H

/**
 * \class doubleHexConverter
 *
 *  \brief convert double numbers to hex strings and vice versa
 *
 *  Convert double precision numbers to thier raw hex representaion
 *  and vice versa.
 *
 *
 */
class doubleHexConverter
{
    public:
        /**
         *  convert a hexadecimal double precision number
         *  into double.
         *
         *  Assumes the hex value is a 64 bit double precision number
         *
         *  @param hexer [in] the hexadecimal string
         *  @return a double containing the number indicated by hexer
         */
        static double hex2d( const string & hexer )
        {
            long long int d =  0;
            int theBits;
            double result;
            for( int i = 0; i< hexer.size(); i++)
            {
                // convert the hex digit to a number
                theBits = hexDigit2dec( hexer[i] );
                //cout << "the num: " << theBits << endl;
                d = d<<4;
                d = d | theBits;
            }

            memcpy( &result, &d, sizeof( long long int ) );
            return result;
        }

        /**
         *  Convert a double precision number into a hexadecimal string
         *
         *  Computes a hex value that represents a 64 bit double precision number
         *
         *  @param d [in] the double precision number 
         *  \returns  the hexadicaml string encoding the number passed in 
         */
        static string d2hex( const double d )
        {
            string hexer;
            char *theChars;
            unsigned int bits;
            char mask = 0x01;
            hexer = "";
            //memcpy( theChars, &test, sizeof( int ) );
            theChars = (char*)(void*)&d;
            for( unsigned int i = 0; i < sizeof( double ); i++ )
            {

                bits = 0;
                // get a char, extract the first 4 bits
                for( unsigned int j = 0; j < 4; j++ )
                {
                    bits |= ((theChars[i] >> j) & mask) << j;
                }
                hexer =  dec2hexDigit(bits)+hexer;
                bits = 0;
                for( unsigned int j = 4; j < 8; j++ )
                {
                    bits |= ((theChars[i] >> j) & mask) << j-4;
                }
                hexer =  dec2hexDigit(bits)+hexer;
            }
            //cout  << hexer << endl;
            return hexer;
        }
        
        /**
         *  Convert a hexadecimal digit to a decimal digit.
         *
         *  @param hexDigit [in] a character containign a hex value.
         *  @return the decimal digit indicated by the hex value (0-15).
         */
        static unsigned int hexDigit2dec( const char hexDigit )
        {
            unsigned int decDigit = (int)hexDigit - (int)'0' ;
            if( decDigit > 9 )
                decDigit = (int)hexDigit - (int)'a' + 10;
            return decDigit;
        }
        /**
         *  Convert a decimal digit to a hexadecimal character value
         *
         *  @param decDigit [in] the decimal digit (0-15)
         *  @return the hex character representing the digit
         */
        static char dec2hexDigit( const unsigned int decDigit )
        {
            if( decDigit < 10 )
                return (char)( decDigit + (int)'0');
            else
                return (char)( (decDigit-10)+(int)'a');
        }

};

#endif



