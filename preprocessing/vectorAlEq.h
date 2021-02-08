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





#include "halfsegment.h"
#include <cstdlib>
#include <iomanip>
// event queue contains a vector and is sorted in hseg order

#ifndef VECTORALEQ_H
#define VECTORALEQ_H
/**
 *  \class eventQueue
 *
 *  \brief A vector-based implementation of a plane sweep event queue
 * 
 *  This is an event queue implemented on top of an STL vector
 *
 *  It is pretty basic.  It simply pops from the front of the vector,
 *  and does an insertion sort-style insert.
 *
 *  This implementation was used to compare the cache behavior of a vector
 *  based plane sweep implementation with an avl tree based implementation.
 *
 *  As expected, the vector based has very good cache behavior (low miss rate),
 *  but has quadratic time complexity.  The AVL tree implementation has a bad 
 *  cache miss rate (often upwards of 20 percent miss rate on L2 cache), but 
 *  a good time complexity
 *
 */
class eventQueue
{
    private:
        /// the vector used for the queue
        vector<halfsegment> eq; 
    
    public:
        /**
         *  insert into the event queue
         *
         *  uses an insertion sort-style insert.
         *  The queue is more like a priority queue, it is always sorted.
         */
        void insert( const halfsegment & h1 ){
            if( eq.empty() || eq[eq.size()-1] < h1 ) {
                eq.push_back( h1 );
                return;
            }
            else {
                for( vector<halfsegment>::iterator it = eq.begin(); ;it++ ){
                    if(  h1 < *it  ){
                        eq.insert( it, h1 );
                        break;
                    }
                }
                
            }
        }
        /**
         * Peek at the head of queue.  return the halfsegment at the head of the queue, but do not remove it
         *
         * \param h1 [in/out]  pass by reference.  Will be set to the values of the halfsegment at the head of the queue.
         * \return False if the queue is empty, True otherwise
         *
         */
        bool peek( halfsegment & h1 ){
            if( eq.empty() ) {
                return false;
            }
            h1 = eq[0];
            return true;
        }
        /**
         *  Pop the element from the head of the queue.
         *
         *  \return True if an element was popped, False if the queue is empty
         */
        bool pop( ){
            if( eq.empty() ) {
                return false;
            }
            eq.erase( eq.begin() );
            return true;
        }
        /**
         * Get the number of halfsegments in the queue
         */
        int size(){
            return eq.size();
        }

        /**
         * Print all elements in the queue (for debugging)
         */
        void print(){
            cerr << "eq:-----"<<endl;
            for( int i = 0; i < eq.size(); i++ )
                cerr << eq[i] << endl;
            cerr << "^^^^^^"<< endl;
        }


};

/**
 * \class activeListVec
 *
 * \brief A vector-based implementation of a plane sweep active list
 *
 * The vector implementation of an activce list. 
 *
 * The active list is always sorted with respect to the current position of the sweep line. In
 *  other words, the segments are sorted vertically with respect to their y intercept at the 
 *  current position of the sweep line.
 *
 *  Like the event queue, this implementation was for cache behavior comparison with the AVL version
 *
 */
class activeListVec
{
    private:
        /// The active list vector 
        vector<halfsegment> al; 
    public:
        /// The current position of the sweep line 
        double xVal; 

        // always assume h1 is the new halfsegment (being entered into the list)
        // returns true if h1 < h2, false otherwise
        /**
         * Active list halfsegment less than.
         *
         * Comparison function for halfsegments based on the current sweep line position.
         *
         * Always assumes the h1 is being added to the list or is the halfsegment being searched for, and h2 is currently in the active lis
         *
         * \param h1 [in] the halfsegment being inserted into the list or being searched for
         * \param h2 [in] a halfsegment in the active list.
         * \return True if the segment is inserted.  False if an equal segment or colinear segment exists in the active list.
         */
        bool alHsegLT( const halfsegment &h1, const halfsegment &h2 )
        {
            // if equal, indicate
            if( h1 == h2 )
                return false;
            // if colinear, then 'first' is greater (due to hseg ordering) 
            // the only time we will have overlapping colinear is when inserting left hsegs
            // so this shouldn't ever come up when removing based on right segs
            if( h1.colinear( h2 ) )
                return false;

            double h1Y = h1.getYvalAtX( xVal );
            double h2Y = h2.getYvalAtX( xVal );
            // at this point, we simply construct hsegs using the sweep line intersect and xval as the dominating point
            // cerr << "xval, h1, h2 :" << xVal << ", " << h1 << ", " << h2 << endl;
            halfsegment newH1;
            halfsegment newH2;
            newH1.dx = newH2.dx = xVal;
            newH1.dy = h1Y;
            newH2.dy = h2Y;
            if( newH1.dx == h1.sx && newH1.dy == h1.sy ) {
                newH1.sx = h1.dx;
                newH1.sy = h1.dy;
            }
            else {
                newH1.sx = h1.sx;
                newH1.sy = h1.sy;
            }
            if( newH2.dx == h2.sx && newH2.dy == h2.sy ) {
                newH2.sx = h2.sx;
                newH2.sy = h2.dy;
            }
            else {
                newH2.sx = h2.sx;
                newH2.sy = h2.sy;
            }
            //cerr << "newh1, newh2: " << newH1 << ", " << newH2 << (newH1 < newH2)<<endl;
            // if Y vals are different
            if( h1Y < h2Y ) return true;
            else if( h1Y > h2Y ) return false;
            // otherwise, the dom point is the same
            // now we just use the halfsgment < operator 
            // for the active list, we want the seg that is above the other one.  
            // special case: when they are both right hsegs, they always share a dom point.  reverse <
            if( !newH1.isLeft() && !newH2.isLeft( ) ) {
                if(  newH1 < newH2 ) return false;
                return true;
            }
            else if( newH1.isLeft() && newH2.isLeft( ) ) { 
                if( newH1 < newH2 ) return true;
                return false;
            }
            else {

                // we have a left one and a right one.  the right hseg is less
                if( !newH1.isLeft() ) return true;
                return false;
                // we should never get 1 left and 1 right hseg

                cerr<< "AL comp.  got a left and right" << endl;
                cerr<< "xval: " << std::setprecision(100)<< xVal << endl;
                cerr << std::setprecision(100) << h1 << endl << h2 << endl << newH1 << endl << newH2 << endl;
                exit( -1 );
            }
            return false;
        }
       
        /**
         * Test if two halfsegments are equal
         *
         * usues the overloaded == operator from the halfsegment class
         */
        inline bool alHsegEQ( const halfsegment &h1, const halfsegment &h2 )
        {
            return (h1 == h2);
        }
        /**
         *  Insert a segment into the active list.  
         *
         *  Segments in the active list are ALWAYS left halfsegments, and are sorted according to thier
         *  y-intercept with the sweep line.  
         *
         *  Do NOT try to insert a right halfsegment.  If you do, undertermined behavior will occur!
         *
         *  If there is a duplicate halfsegment (based only on end points, not labels) in the active list,
         *  the duplicate parameter is assigned the duplicate halfsegment. 
         *
         *  \param h1 [in] the halfsegment to insert into the active list
         *  \param duplicate [out] assigned True if a duplicate of h1 is in the active list.  False otherwise.
         *  \param theDup [out] a copy of the duplicate halfsegment already un the active list
         *  \param segIndex [out] the index of duplicate of the h1, if one is found, or the index of h1 after the insert if no duplicate is found.
         */
        void insert( const halfsegment & h1, bool & duplicate, halfsegment & theDup, int & segIndex )
        {
            duplicate = false;
            if( al.empty() || !alHsegLT( h1, al[al.size()-1])  ) {
                al.push_back( h1 );
                theDup = h1;
                segIndex = al.size()-1;
                return;
            } 
            else {
                int i = 0;
                for( vector<halfsegment>::iterator  it = al.begin(); ; it++ ){
                    if( *it == h1 ){
                        duplicate = true;
                        theDup = *it;
                        segIndex = i;
                        return;
                    } 
                    else if( alHsegLT( h1, *it ) ){
                        al.insert( it, h1 );
                        theDup = h1;
                        segIndex = i;
                        return;
                    }
                    i++;
                }
            }
        }
        
        /**
         *  Chech if a halfsegment exists in the active list
         *
         *  \param h1 [in] the halfsegment to check
         *  \param theCopy [out] a copy of the halfsegment identical to h1 (in structure only, equality ignores label values) that is in the active list
         *  \param index [out] the index of the theCopy, if it is found.  -1 if h1 is not in the active list.
         */
        bool exists( const halfsegment& h1, halfsegment & theCopy, int & index ){
            index = find (h1 );
            if( index != -1 ){
                theCopy = al[index];
                return true;
            }
            return false;
        }

        /**
         *  Find the index of a halfsegment in the active list.
         *
         *  linear time search.  returns -1 if the halfsegment is not in the active list.
         */
        int find( const halfsegment& h1 ){
            for( int i= 0; i < al.size(); i++ ){ 
                if( h1 == al[i] )
                    return i;
            }
            return -1;
        }

        /**
         *  replace a halfsegment in the active list with newH1.  
         *
         *  Used to update labels.
         *
         *  Uses the find function to find h1 in the active list
         */
        void replace( const halfsegment &h1, const halfsegment & newH1 ){
            int index = find( h1 );
            if( index <0 || index >= al.size() ){
                cerr << "replace: did not find halfsegment: " << h1 << endl;
                exit( -1 );
            }   
            al[index] = newH1;
        }
        /**
         * replace a halfsegment in the active list with newH1.  
         *
         * Used to update labels.
         *
         * Does not search for the halfsegment in the active list.
         * Rather, it looks for the halfsegment at the specified index.
         *
         * If the halfsegment at the specified index does not match h1, 
         * the program crashes!
         */
        void replace( const halfsegment &h1, const halfsegment & newH1, const int index ){
            if( index <0 || index >= al.size() ){
                cerr << "replace: did not find halfsegment: " << h1 << endl;
                exit( -1 );
            }
            if( al[index] == h1 ){
                al[index] = newH1;
                return;
            }
            else {
                cerr << "trying to replace a seg by index that does not match" <<endl;
                exit( -1 );
            }
        }

        /**
         * Find the halfsegment direclty above (the neighbor above) h1
         *
         * \param h1 [in] the halfsegment whose neighbor we are finding
         * \param theAbove [out] a copy of the neighbor above h1
         * \param index [in]  the index of h1 in the active list
         */
       bool getAbove( const halfsegment& h1, halfsegment &theAbove, const int index ){
            // cerr << "foundabove index = " << index << endl;
            if( index < 0 || index > al.size() ){
                cerr << "invalid size getAbove AL"<<endl;
                exit(-1);
            }
            if( index >= al.size()-1 ){
                return false;
            }
            theAbove = al[index+1];
            return true;
        }
       
        /**
         * Find the halfsegment direclty above (the neighbor above) h1
         *
         * The same as the other getAbove, but doesn't know the index of h1, so the find function is used to locate h1 in the active list
         *
         * \param h1 [in] the halfsegment whose neighbor we are finding
         * \param theAbove [out] a copy of the neighbor above h1
         */
       bool getAbove( const halfsegment& h1, halfsegment &theAbove ){
            int index = this->find( h1 );
            // cerr << "foundabove index = " << index << endl;
            if( index < 0 || index > al.size() ){
                cerr << "invalid size getAbove AL"<<endl;
                exit(-1);
            }
            if( index >= al.size()-1 ){
                return false;
            }
            theAbove = al[index+1];
            return true;
        }
        /**
         * Find the halfsegment direclty below (the neighbor below) h1
         *
         * \param h1 [in] the halfsegment whose neighbor we are finding
         * \param theBelow [out] a copy of the neighbor below h1
         * \param index [in]  the index of h1 in the active list
         */
        bool getBelow(const halfsegment& h1, halfsegment &theBelow, const int index ){
            if( index < 0 || index >= al.size() ){
                cerr << "invalid size getBelow AL"<<endl;
                exit(-1);
            }
            if( index <= 0 ){
                return false;
            }
            theBelow = al[index-1];
            return true; 
        }
        /**
         * Find the halfsegment direclty below (the neighbor below) h1
         *
         * The same as the other getBelow, but doesn't know the index of h1, so the find function is used to locate h1 in the active list
         *
         * \param h1 [in] the halfsegment whose neighbor we are finding
         * \param theBelow [out] a copy of the neighbor below h1
         */
        bool getBelow(const halfsegment& h1, halfsegment &theBelow ){
            int index = this->find( h1 );
            if( index < 0 || index >= al.size() ){
                cerr << "invalid size getBelow AL"<<endl;
                exit(-1);
            }
            if( index <= 0 ){
                return false;
            }
            theBelow = al[index-1];
            return true; 
        }
                
        void erase( const halfsegment & h1, const int index )
        {
            // cerr << "erase1"<<endl;
            if( index < 0 || index >= al.size() ) 
                return; 
            
            vector<halfsegment>::iterator  it = al.begin() + index;
            if( *it == h1 ){
                al.erase( it );
                // cerr << "erase2"<<endl;
                return;
            } 
            else {
                cerr << "trying to erase a seg by index that does not match" <<endl;
                exit( -1 );
            }
	}

        /**
         *  Find the halfsegment that is equal to h1 (in structure only, equality is not based on labels)
         *  and remove it from the active list.
         */
        void erase( const halfsegment & h1 )
        {
            // cerr << "erase1"<<endl;
            for(vector<halfsegment>::iterator  it = al.begin();it != al.end() ; it++ ){
                if( *it == h1 ){
                    al.erase( it );
                    // cerr << "erase2"<<endl;
                    return;
                } 
	    }
	}

        /**
         *  Print function for debugging
         */
        void print(){
            cerr << "al:-----"<<endl;
            for( int i = 0; i < al.size(); i++ )
                cerr << al[i] << endl;
            cerr << "^^^^^^" << endl;
        }
};
#endif

