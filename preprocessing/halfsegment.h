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
#ifndef HALFSEGMENT_H
#define HALFSEGMENT_H

//#define DEBUG_PRINT

using namespace std;

/**
 * Left hand turn test
 *
 * returns true if when you start at p1, and walk towards p2, you have to make a left turn
 * at p2 in order to continue walking to p3.
 *
 * Takes 3 points.  The parameters are the x and y values for point p1, the x and y values for point p2, 
 * and the x and y values for point p3.
 */
inline bool leftHandturn( double p1x, double p1y, double p2x, double p2y, double p3x, double p3y )
{
	// returns true if when you start at p1, and walk towards p2, you have to make a left turn
	// at p2 in order to continue walking to p3
	return( ( ((p3y - p1y) * (p2x - p1x)) - ((p2y - p1y) * (p3x - p1x)) ) > 0 );
}


/** 
 * \class halfsegment
 * 
 * \brief holds a single halfsegment, along with labeling information and information indicating if the opposing region's interior lies above and/or below the halfsegment.
 *
 *  holds a halfsegment.  
 *
 *  Contains the x and y values for the dominating and submissive points.
 *
 *  Contains integer labels:  la = label above and lb = label below
 *
 *  Contains the ID of the strip that this halfsegment is assigned to.
 *
 *  Contains the region to which the halfsegment belongs (there are two input regions)
 *
 *  Contains the overlap lables.  Overlap labels indicate if the interior of the 
 *  opposing region lies above and/or below this halfsegment.
 */
struct halfsegment {
	/// dominating and submissive points
	double dx, dy, sx, sy; 
	///label above, label below 
        int la, lb; 
	/// strip ID
        int stripID; 
        // the region this seg nelongs to
	int regionID;
        /// overlap labels
	int ola, olb;
	
        /**
         * Default constructor
         */
halfsegment():dx( 0 ), dy(0), sx(0), sy(0), la(-1), lb(-1),
		stripID(-1), regionID( -1 ), ola( -1 ), olb( -1 )
	{ }

    /**
     * Copy constructor
     */
halfsegment( const halfsegment &rhs ): dx(rhs.dx), dy(rhs.dy),
		sx(rhs.sx), sy(rhs.sy), 
		la(rhs.la), lb(rhs.lb), 
		stripID( rhs.stripID), regionID( rhs.regionID ),
		ola( rhs.ola ), olb( rhs.olb )
			{ }


	/**
         *  Returns true if this is a left halfsegment, right otherwise.
         */
	bool isLeft( ) const {
		return( dx < sx || (dx == sx && dy < sy ) );
	}

        /**
         * Compute the brother of this halfsegment. (switch the dominating and submissive points).
         */
        halfsegment getBrother( ) const {
		halfsegment tmp( *this );
		tmp.dx = sx;
		tmp.dy = sy;
		tmp.sx = dx;
		tmp.sy = dy;
		return tmp;
	}

        /**
         *  Overloaded equivalence test.
         *
         *  Computes equivalence based ONLY on the end points of the halfsegments.
         *
         *  Segments are equal if their dominating and submissive points match.
         */
	bool operator==( const halfsegment &rhs ) const {
		return dx == rhs.dx && dy == rhs.dy && sx == rhs.sx && sy == rhs.sy;
	}

        /**
         *  Overloaded less than operator.
         *
         *  Computes based ONLY on the end points of the halfsegment.
         *  That is,  not nased on labels
         *
         *  Uses halfsegment ordering to determine less than
         */
	bool operator<( const halfsegment &rhs ) const {
		if( dx < rhs.dx || (dx == rhs.dx && dy < rhs.dy ) ) return true;
		else if( dx == rhs.dx && dy == rhs.dy ) {
			// dom points the same
			if( isLeft() != rhs.isLeft( ) ) return !isLeft();
			// both left or right hsegs
			else if( this->colinear( rhs ) && (sx < rhs.sx || (sx == rhs.sx && sy < rhs.sy)) ) return true;
			else if( leftHandturn( dx, dy, sx, sy, rhs.sx, rhs.sy ) ) return true;
		}
		return false;
	}
	
        /**
         *  Test if two halfsegments are colinear.
         *
         *  Tests for exact colnearity.  SENSITIVE TO ROUNDING ERRORS.
         */
	inline bool colinear( const halfsegment &rhs) const {
		return 0 ==	( ((rhs.dy - dy) * (sx - dx)) - ((sy - dy) * (rhs.dx - dx)) ) &&
			0 == ( ((rhs.sy - dy) * (sx - dx)) - ((sy - dy) * (rhs.sx - dx)) ) ;
	}
	
        /**
         *  Compute the y value on a linesegment (halfsegment in this case) at a given x value.
         *
         *  Note that if the x value is beyond the end of the line segment, this still returns
         *  a y value.  In otherwords, it is based only upon the equation of a line.  The caller
         *  needs to ensure x is either within the line segment, or check after the function returns
         *
         *  Note that this is sensitive to divide by 0! and to floating point rounding errors
         */
	double getYvalAtX( const double x ) const 
	{
		if( x == dx )	return dy;
		else if( x == sx ) return sy;
		return( ( (sy*x - sy*dx - dy*x + dy*dx) / float((sx-dx))) + dy );
	}

        /**
         *  Overloaded ostream operator.
         */
	friend ostream & operator<<( ostream & out, const halfsegment & rhs ) {
		cerr <<"[(" << rhs.dx << "," << rhs.dy << ")(" << rhs.sx << "," << rhs.sy << ") " << rhs.la << ", " <<  rhs.lb << ", " << rhs.regionID << " <" << rhs.ola <<","<<rhs.olb<<">"<< "]";
		return out;
	}

};
#endif


