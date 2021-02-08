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
/* Code edited by Eric Lehmann will be marked ELEHMANN for ease of identification. */

#include <iomanip>
#include "parPlaneSweep.h"
#include "vectorAlEq.h"
#include <limits>
#include <numeric>
#include <execution>
#include <algorithm>
#include <thread>

/**
 * A binary search function
 */
int binarySearchExists( vector< halfsegment > &region, double x, int hi=-1, int lo=0 ) 
{
	if( hi < 0 )
		hi = region.size();
	while( lo < hi ) {
		int mid = (lo+hi)/2;
		if( region[mid].dx < x ) lo = mid+1;
		else if( region[mid].dx > x ) hi = mid;
		else return mid;
	}
	return -1;
}

/**
 * A binary search function
 */
int binarySearchSmallestGreater( vector< halfsegment > &region, double x, int hi=-1, int lo=-1 ) 
{
	if( hi < 0 )
		hi = region.size();
	while( lo < hi ) {
		int mid = (lo+hi)/2;
		if( region[mid].dx <= x ) lo = mid+1;
		else if( region[mid].dx > x ) {
			if( mid > 0 && region[mid-1].dx <= x ) return mid;
			hi = mid-1;
		}
		//else return mid;
	}
	return hi;
}

/**
 * A binary search function
 */
bool binarySearchHalfsegment(vector< halfsegment > &region, 
														const halfsegment &h, int & mid, int hi=-1, int lo=0 )  
{
	if( hi < 0 )
		hi = region.size();
	while( lo < hi ) {
		mid = (lo+hi)/2;
		if( region[mid] < h ) lo = mid+1;
		else if( region[mid] == h ) return true;
		else hi = mid;
	}
	mid = lo;
	return false;
}

/**
 *  Find the isolation boundaries.  Isolation boundaries are vertical lines
 *  that do not intersect any halfsegment end points in r1 or r2 that form the 
 *  strip boundaries
 *
 *  \param r1  halfsegments defining one input region
 *  \param r2 halfsegments definining the second input region
 *  \param isoBounds [in/out] the x values indicating vertical lines that form strip boundaries.
 *
 */
void findIsoBoundaries( vector<halfsegment> &r1, vector<halfsegment> &r2,
												vector< double> & isoBounds );


/**
 *  Break an input region up into strips.  Strip boundaries are isoBounds
 *
 *  \param region the region to split into strips
 *  \param isoBounds  the strip boundaries
 *  \param rStrips [out] the region broken into strips.  Each halfsegment will have a strip ID indicating the strip to which it belongs.  stripIDs start at 0 and increment.
 *  \param stripStopIndex [out] halfsegments in rStrips are sorted by strip ID then halfsegment ordering.  This vector marks the positions in the rStrips array where the last halfsegment in each strip is located.
 */
void createStrips(unsigned int rThreadNum, vector< halfsegment> & region, vector<double> &isoBounds, 
									 vector<halfsegment> & rStrips, 	vector< int > &stripStopIndex );



/**
 * This function calls the individual overlay functions.  It simply sets up calls
 * to overlayPlaneSweep().
 *
 * It is expected that the overlayPlaneSweep() function splits up the input into 
 * strips, and then passes those strips to this function.
 *
 * The start and stop indexes indcate where each strip starts in the r1Strips and r2Strips vectors.
 */
void partialOverlay( 	const vector<halfsegment> & r1Strips, const vector<halfsegment> & r2Strips,
											vector<halfsegment> & result, 
											vector< int > &r1StripStopIndex, vector< int > &r2StripStopIndex, 
											const int stripID );




/**
 *  Find the intersection point between two halfsegments.  Also indicate if they are colinear.  
 */
bool findIntersectionPoint( const halfsegment & h1, const  halfsegment & h2,
														double & X, double & Y, bool & colinear );


/**
 *  break halfsegments if they intersect.  returns a vector of the resulting halfsegments, since various numbers of halfsegments are returned based on the confguration of the input halfsegments.
 */
bool breakHsegs( const halfsegment &alSeg, halfsegment & origCurr,
								 vector< halfsegment> & brokenSegs, bool & colinear,
								 const bool includeCurrSegInBrokenSegs );

/**
 *  Remove breaks in halfsegments that are only introduced to create strips.
 *  
 *  This is not strictly necessary, but removes breaks in halfsegemnts 
 *  that were introduced solely for the purpose of createing strips
 */
void createFinalOverlay( vector<halfsegment> & finalResult,
												 vector< vector<halfsegment> > &resultStrips, 
												 const vector<double> &isoBounds );


/**
 *  Once two intersecting halfsegments have been broken up based on their intersection such that the result halfsegments only intersect at end points, we need to put those halfsegments in the event queue, and possible the active list.  This function does that.
 *
 *  eventX and eventY indicate the current event point (where the sweep line is).
 */
void insertBrokenSegsToActiveListAndDiscoveredQueue( const vector<halfsegment> & brokenSegs,
																										 vector<halfsegment> & result,
																										 eventQueue& discoveredSegs,
																										 activeListVec& activeList,
																										 const double eventX,
																										 const double eventY );

/**
 * See the prototype in parPlaneSweep.h
 */
void parallelOverlay( vector<halfsegment> &r1, vector<halfsegment> &r2, vector<halfsegment> &result, 
							int numStrips, int numWorkerThreads )
{
	vector<halfsegment> r1Strips, r2Strips;
	vector< double > isoBounds;
	vector< vector< halfsegment> > resultStrips;
	vector< int > r1StripStopIndex, r2StripStopIndex;
	result.clear();  // make sure the result vec is clear

		// set default parallel values
if ( numWorkerThreads > 0 ) {
	}
	else {
	}
	if (numStrips < 0) {
		numStrips = 1;
	}


	int numIsoBounds = numStrips+1;

	// set up vectors for split points, return strips
	for( int i = 0; i < numStrips; i++ ) {
		resultStrips.push_back( vector<halfsegment>() );
	}
	for( int i = 0; i < numIsoBounds; i++ ) {
		isoBounds.push_back( 0 );
	} 
	
	// find split points
	findIsoBoundaries( r1, r2, isoBounds );

	// split up the regions at the iso boundaries
// ELEHMANN
	int track_regions[] = {0,1};
	unsigned int thread0 = 0;
	unsigned int thread1 = 1;
	std::for_each( std::execution::par, std::begin(track_regions), std::end(track_regions), [&] (int i){
		if( i == 0 ) createStrips(thread0, r1, isoBounds, r1Strips, r1StripStopIndex );
		else  createStrips( thread1, r2, isoBounds, r2Strips, r2StripStopIndex );
		});
	std::vector<int> track_strips(numStrips);
	std::iota( track_strips.begin(), track_strips.end(), 0);
	std::for_each( std::execution::par, track_strips.begin(), track_strips.end(), [&] (int i) {
	partialOverlay( r1Strips, r2Strips, resultStrips[i], r1StripStopIndex, r2StripStopIndex, i );
		});
//END

	createFinalOverlay(  result,
										 resultStrips, 
												isoBounds );

}

// call the individual overlay functions.
// this function just sets up the function calls to overlayPlaneSweep
/**
 * See the prototype in parPlaneSweep.h
 */
void partialOverlay( 	const vector<halfsegment> & r1Strips, const vector<halfsegment> & r2Strips,
											vector<halfsegment> & result, 
											vector< int > &r1StripStopIndex, vector< int > &r2StripStopIndex, 
											const int stripID )
{
	
	int r1Start, r2Start, r1Stop, r2Stop;
	if( stripID == 0 ) {
		r1Start = r2Start = 0;
	}
	else {
		r1Start = r1StripStopIndex[stripID-1];
		r2Start = r2StripStopIndex[stripID-1];
	}
	r1Stop = r1StripStopIndex[stripID];
	r2Stop = r2StripStopIndex[stripID];

	overlayPlaneSweep( &(r1Strips[r1Start]), r1Stop-r1Start, 
										&(r2Strips[r2Start]), r2Stop-r2Start,
										 result);
}

/**
 * See the prototype in parPlaneSweep.h
 */
void overlayPlaneSweep( const halfsegment r1[], int r1Size, 
												const halfsegment r2[], int r2Size, 
												vector<halfsegment>& result )
{
	halfsegment currSeg, maxSeg, tmpSeg, *tmpSegPtr;
	maxSeg.dx = maxSeg.dy = maxSeg.sx = maxSeg.sy = std::numeric_limits<double>::max();
	double eventX, eventY;
	activeListVec activeList;
        eventQueue discoveredSegs;
	vector< halfsegment > brokenSegs;
	bool colinearIntersection;
	int r1Pos = 0;
	int r2Pos = 0;
	int segSource;
        int segIndex;
	while( discoveredSegs.size() > 0 || r1Pos < r1Size || r2Pos < r2Size ) {
		// get the next seg
		// next seg is the least seg from r1, r2, and the discoveredSeg tree (event queue)
		currSeg = maxSeg;
		if( r1Pos < r1Size ) {
			currSeg = r1[r1Pos];
			segSource = 1;
		}
		if( r2Pos < r2Size &&  r2[r2Pos]< currSeg ) {
			currSeg = r2[r2Pos];
			segSource = 2;
		}
                if(discoveredSegs.peek( tmpSeg  ) ) {
                    if( tmpSeg < currSeg || tmpSeg == currSeg  ){
                        currSeg = tmpSeg;
                        segSource = 3;
                    }
                }
		// remove the next seg from its source
		if( segSource == 3 ) {
			discoveredSegs.pop();
		}
		else if( segSource == 2 ){
			r2Pos++;
		}
		else {
			r1Pos++;
		}
		
		// set current event point.
		// the activeList compare function uses eventX as its |param| argument
		eventX = currSeg.dx;
                activeList.xVal = eventX;
		eventY = currSeg.dy;
                // cerr << "========================="<<endl;
                // cerr << "segSrc: " << segSource<< "  currSeg: " << currSeg << endl;
		// If curr is a left seg, insert it and check for intersections with neighbors
		// Else it is a right seg, remove it and check its neighbors for intersections
		if( currSeg.isLeft( ) ) {
			// initialize the overlap labels
			currSeg.ola = currSeg.olb = -1;
			// insert the left seg
			// use avl_t_insert so we can get its neighbors.  
			
                        bool dup;
                        halfsegment segInAL;
                        activeList.insert( currSeg, dup, segInAL, segIndex );
                        // record the active list seg for replacement later (if there are intersections with it)
                         //activeList.print();
                        // cerr << "al seg: "<<segInAL << "segIndex: " << segIndex << endl; 
			// if a duplicate is in the active list, we get a pointer to the duplicate. So we need to update labels
			// if insert is successful, we get a pointer to the inserted item
			if( dup ) { 
                             // cerr << "dup"<<endl;
                                // activeList.print();
				// We found a duplicate in active list.  update labels
				// NOTE: overlapping segs are a special case for labels.
				// NOTE: overlap labels are altered in the |breakHsegs()| as well,
				//       but that function is not called here (no need to break up segs
				//       if they are equal)
				segInAL.ola = currSeg.la;  
				segInAL.olb = currSeg.lb;
                                activeList.replace( segInAL, segInAL, segIndex );
			} 
			else {
				bool needToRemoveCurr = false;
			        bool hasAbove, hasBelow;
                                // inserted successfully.  Need to get neighbors
				halfsegment belowSegCopy;
				halfsegment aboveSegCopy;
				hasBelow =activeList.getBelow( currSeg, belowSegCopy, segIndex );
				hasAbove = activeList.getAbove( currSeg, aboveSegCopy, segIndex );
			        // if( hasBelow) cerr << "below: "<< belowSegCopy<<endl;
                                
                                // if( hasAbove ) cerr << "above: " << aboveSegCopy<<endl;
                                
				// do intersections with above and below.  Update currSeg along the way
				brokenSegs.clear();
			
				// We have to deal with the below seg first because currSegs labels get changed
				// based on the below seg.  Once we begin dealing with the above seg, the labels
				// for the currSeg are already computed and will carry over into those calaculations
				if( hasBelow ) {
					// update labels:
					// NOTE: overlapping segs are a special case for labels.
					// NOTE: overlap labels are altered in the |breakHsegs()| as well,
					if( currSeg.regionID != belowSegCopy.regionID ) { 
						// if below seg is from opposing region, set overlap labels
						if( belowSegCopy.dx != belowSegCopy.sx ) { // if seg is not vertical, use la (label above)
							currSeg.ola = currSeg.olb = belowSegCopy.la;
						} else { // if below seg is vertical, use lb (the label to the right)
							currSeg.ola = currSeg.olb = belowSegCopy.lb;
						}
					} 
					else if(currSeg.regionID == belowSegCopy.regionID ) { // if below seg is from same region, just extend overlap labels
						currSeg.ola = currSeg.olb = belowSegCopy.ola;
						// commented code is for checking against vertical seg below from same regions
						// this should never happen for well formed regions based on hseg order
						// if( belowSegCopy.dx != belowSegCopy.sx ) { // if seg is not vertical, use ola (label above)
						//		currSeg.ola = currSeg.olb = belowSegCopy.ola;
						//  } else { // if below seg is vertical, use lb (the label to the right)
						//	currSeg.ola = currSeg.olb = belowSegCopy.olb;
						//	}
					}
				
					// Labels are now computed
					// Compute the segment intersections:
					if(breakHsegs(  belowSegCopy, currSeg, brokenSegs, colinearIntersection, false ) ){
					        // cerr << "attempt to erase below: " << belowSegCopy << " " <<segIndex << endl;
                                                needToRemoveCurr = true;
						// remove below seg
						activeList.erase( belowSegCopy, segIndex-1 );
                                                segIndex--;
					}
									
				}
				// compute intersections with above seg:
				if( hasAbove ) {
					if( breakHsegs(  aboveSegCopy, currSeg, brokenSegs, colinearIntersection, false ) ) {
					        //cerr << "attempt to erase above: " << aboveSegCopy << " " <<segIndex << endl;
						
                                            needToRemoveCurr = true;
						// remove above seg
						activeList.erase( aboveSegCopy, segIndex +1 );
					}
				}

				// Update the seg inserted into the active list this round
				// currSeg is the result of intersecting that seg with its neighbors
                                // cerr << "about to rep: " << segInAL << endl <<currSeg << endl;
                                activeList.replace( segInAL, currSeg, segIndex );
				// Insert all the broken up segs into thier various data structures
				insertBrokenSegsToActiveListAndDiscoveredQueue( brokenSegs,result,
																													discoveredSegs, activeList,
																													eventX, eventY );
				
			}	
		}
		else {
			// This is a right halfsegment. 
			// find its brother (left halfsegment) in the active list,
			//      remove it, and check its neighbors for intersections.  
			currSeg = currSeg.getBrother();
		        // cerr <<"brother: " << currSeg << endl;
                        halfsegment theALseg;
                        int segIndex;
                        if( activeList.exists( currSeg, theALseg, segIndex ) ){
                            // cerr << "found brother"<<endl;
				// We found the halfsegment in the active list.
				// Its possible we don't find one, since a right halfsegment may be in r1 or r2
				//   whose brother (left halfsegment) was broken due to an intersection
				result.push_back( theALseg );
				result.push_back( theALseg.getBrother( ) );
				// The copy of the seg in the active list has the appropriate labels, so copy it over
				currSeg = theALseg;
				// find the neighbors
                                halfsegment belowCopy, aboveCopy;   
				bool isAbove, isBelow;
                                isBelow = activeList.getBelow( currSeg, belowCopy, segIndex );
			        isAbove = activeList.getAbove( currSeg, aboveCopy, segIndex );
                                halfsegment origAbove(aboveCopy);
				// delete the segment
				if( isAbove && isBelow ) {				
                                    // cerr << "check above/below for inters"<<endl;
					brokenSegs.clear();
					if(breakHsegs(  belowCopy, aboveCopy, brokenSegs, colinearIntersection, true ) ){
                                                // if we got here, we are done updating labels, so we
                                                // can kill iterators in the active list by deleting
                                                //activeList.replace( tmpSeg, aboveCopy);
                                                //activeList.erase( currSeg );
                                                //alHSegPtr = NULL;
						// remove below seg and above seg
						activeList.erase( belowCopy, segIndex-1  );
                                                segIndex--;
						activeList.erase( origAbove, segIndex+1  );
                                                activeList.erase( currSeg, segIndex );
						// add broken segs to output and discovered list
						insertBrokenSegsToActiveListAndDiscoveredQueue( brokenSegs,result,
																														discoveredSegs, activeList,
																														eventX, eventY );
                                        } else {
                                            // delete the currseg
                                            activeList.erase( currSeg, segIndex );
                                        }
                                } else {
                                    // jsut delete the seg from the active list
                                    // cerr << "just erase: " << currSeg << endl;
                                    activeList.erase( currSeg, segIndex );
                                }
                                // activeList.print();

                            
				
			}
		}

	}
		// sort the result
	
		std::sort( result.begin(), result.end() );
	//	cerr << "ps result: " <<endl;
	//for( int i = 0; i <result.size(); i++ )
	//	if( result[i].isLeft() )
	//		cerr << result[i]<<endl;

}

void insertBrokenSegsToActiveListAndDiscoveredQueue( const vector<halfsegment> & brokenSegs,
																										 vector<halfsegment> & result,
																										 eventQueue& discoveredSegs,
																										 activeListVec& activeList,
																										 const double eventX,
																										 const double eventY )
{
	for( int i = 0; i < brokenSegs.size(); i++ ){
		if( (brokenSegs[i].dx != brokenSegs[i].sx && (  brokenSegs[i].dx <= eventX &&  brokenSegs[i].sx <= eventX))
				|| (brokenSegs[i].dx == brokenSegs[i].sx && (  brokenSegs[i].dy <= eventY &&  brokenSegs[i].sy <= eventY)) ){
			// If the seg is behind sweep line, just put it in the output.
			// The seg is behind teh sweep line if it is not vertical and dx,sx <= eventX
			// OR the seg is vertical and dy,sy <= eventY
			result.push_back( brokenSegs[i] );
		}
		else if ( !brokenSegs[i].isLeft()
							|| brokenSegs[i].dx > eventX 
							|| ( brokenSegs[i].dx == eventX && brokenSegs[i].dy > eventY ) ) {
			// If the seg is ahead of the sweep line, or a right halfegment,
			// it goes in the event queue (discovered list)
			// The seg is ahead of the sweep line if dx > eventX or dx = eventx and dy > eventY
                        discoveredSegs.insert( brokenSegs[i] );
		}
		else {
			// If we get here, the seg spans the sweep line and is a left halfsegment
			// The seg goes back into the active list.
			bool tmp;
                        halfsegment tmph;
                        int tmpi;
		        activeList.insert( brokenSegs[i], tmp,tmph,tmpi );
		}
	}
}

bool breakHsegs( const halfsegment &alSeg, halfsegment & origCurr, 
								 vector< halfsegment> & brokenSegs, bool & colinear,
								 const bool includeCurrSegInBrokenSegs)
{
	halfsegment tmpSeg;
	halfsegment curr = origCurr; // copy
	const halfsegment h2 = alSeg; // rename
	bool foundIntersection;
	// get the intersecion point
	double X,Y;
	if( foundIntersection = findIntersectionPoint( h2, curr, X, Y, colinear ) ) {
		if( colinear ) {
			// If the segs are colinear, their intersection can have at most 3 components
			//  1) one seg begins to the left (or below) the other. (non overlapping part)
			//  2) the portion of the segs that overlap
			//  3) one seg ends to the right (or above) the other. (non overlapping part)
			// at most, all three portions exist. At the least, portion (2) will exist
			if( h2.dx < curr.dx ||  h2.dy < curr.dy ) { // build the first part (1)
				tmpSeg = h2;
				tmpSeg.sx = curr.dx;
				tmpSeg.sy = curr.dy;
				brokenSegs.push_back( tmpSeg ); // THIS IS NEW Neighbor SEG from AVL tree
				brokenSegs.push_back( tmpSeg.getBrother() );
			}
			// build the middle part (2)
			tmpSeg = curr;
			if( curr.sx > h2.sx || (curr.sx == h2.sx && curr.sy > h2.sy )) {
				// curr extends beyond h2
				tmpSeg.sx = h2.sx;
				tmpSeg.sy = h2.sy;
			}
			tmpSeg.ola = h2.la; // Set the overlapping labels. 
			tmpSeg.olb = h2.lb;
			brokenSegs.push_back( tmpSeg.getBrother() );
			if( includeCurrSegInBrokenSegs ) { // don't need to return currSeg for inserting left hseg
				brokenSegs.push_back( tmpSeg ); 
			}
			origCurr = tmpSeg;  // UPDATE |origCurr| so that the overlap labels get updated
			                    // it is pass by reference
			// build last part (3)
			if( curr.sx != h2.sx || curr.sy != h2.sy ) {
				// h2 extends past curr
				tmpSeg = h2;
				tmpSeg.dx = curr.sx;
				tmpSeg.dy = curr.sy;
				if( curr.sx > h2.sx || (curr.sx == h2.sx && curr.sy > h2.sy )) {
					// curr extends past h2
					tmpSeg = curr;
					tmpSeg.dx = h2.sx;
					tmpSeg.dy = h2.sy;
				}
				brokenSegs.push_back( tmpSeg ); // some future seg
				brokenSegs.push_back( tmpSeg.getBrother() );
			}
		}
		else {
			// regular intersection
			// split up curr
			if( (X == curr.dx && Y == curr.dy ) ||
					(X == curr.sx && Y == curr.sy ) ) {
				// If this is an end point intersection, do not break the seg
				// we do not remove the curr seg from the active list, so no need
				// to add to broken segs to get reinserted.  Active list curr seg
				// gets updated in the plane sweep function after all intersections
				// with currSegs neighbors have been computed
			}
			else {
				// If the intersection is on the interior of this seg, break it up
				tmpSeg = curr;
				tmpSeg.sx = X;
				tmpSeg.sy = Y;
				brokenSegs.push_back( tmpSeg.getBrother() );
				if( includeCurrSegInBrokenSegs ) { // don't need to return currSeg for inserting left hseg
					brokenSegs.push_back( tmpSeg ); 
				}
				origCurr = tmpSeg;  // UPDATE |origCurr|.  it is passed by reference
				tmpSeg = curr;
				tmpSeg.dx = X;
				tmpSeg.dy = Y;
				brokenSegs.push_back( tmpSeg );
				brokenSegs.push_back( tmpSeg.getBrother() );	
			}
			// split up h2.  Identical code to above. see those comments
			if( (X == h2.dx && Y == h2.dy ) ||
					(X == h2.sx && Y == h2.sy ) ) {
				// If this is an end point intersection, do not break the seg
				// we will reinsert this seg, but its brother remains the same,
				// so we don't need to put the brother in
				// reinsertion is just for convienience
				// we remove the seg in the plane sweep function so 
				// we don't have to keep track of the aboves belows.
				// In the case of colinears, seg must be removed anyway, so we just
				// always remove the above/below segs and reinsert them if needed.
				brokenSegs.push_back(  h2 );
			}
			else {
				// interior intersection, split
				tmpSeg = h2;
				tmpSeg.sx = X;
				tmpSeg.sy = Y;
				brokenSegs.push_back( tmpSeg ); // THIS IS THE UPDATED H2
				brokenSegs.push_back( tmpSeg.getBrother() );
				tmpSeg = h2;
				tmpSeg.dx = X;
				tmpSeg.dy = Y;
				brokenSegs.push_back( tmpSeg );
				brokenSegs.push_back( tmpSeg.getBrother() );	
			}
		}
	}
        // cerr << "------- broken segs -----" << endl;
        // for( int i =0; i < brokenSegs.size(); i++ )
        //     cerr << brokenSegs[i]<<endl;
        // cerr << "-------  -----" << endl;

	return foundIntersection;
}

bool findIntersectionPoint( const halfsegment & h1, const  halfsegment & h2, double & X, double & Y, bool & colinear )
{
	// if colinear, intersection point is h2 dominating (since h2 will be curr seg from overlay)
	colinear = false;
	if( h1.colinear( h2 ) ) {
		X = h2.dx;
		Y = h2.dy;
		colinear = true;
		return true;
	}
	// if they share an end point, there is nothing to do
	if( (h1.dx == h2.dx && h1.dy == h2.dy ) ||
			(h1.sx == h2.sx && h1.sy == h2.sy ) ) {
		X = std::numeric_limits<double>::max();
		Y = std::numeric_limits<double>::max();
		return false;
	}

	// find intersection point
	double x1 = h1.dx;
	double y1 = h1.dy;
	double x2 = h1.sx;
	double y2 = h1.sy;
	double x3 = h2.dx;
	double y3 = h2.dy;
	double x4 = h2.sx;
	double y4 = h2.sy;

	double denom = ((y4-y3)*(x2-x1)) - ((x4-x3)*(y2-y1));
	double ua = ((x4-x3)*(y1-y3)) - ((y4-y3)*(x1-x3));
	double ub = ((x2-x1)*(y1-y3)) - ((y2-y1)*(x1-x3));
	
	ua = ua/denom;
	ub = ub/denom;
	// if ua and ub are between 0 and 1 inclusive, we have an intersection
	// in at least 1 interior.  
	// end point intersections are handled above
	if( 0.0 <= ua && ua <= 1.0 && 0.0 <= ub && ub <= 1.0 ) {
		X = x1 + ( ua*(x2-x1) );
		Y = y1 + ( ua*(y2-y1) );
		return true;
	}
	else {
		X = std::numeric_limits<double>::max();
		Y = std::numeric_limits<double>::max();
	}
	return false;
}

bool hsegIDSort( const halfsegment & h1, const halfsegment & h2 ) 
{
		return h1.stripID < h2.stripID || (h1.stripID == h2.stripID && h1 < h2 );
}

//ELEHMANN
void threadedStrips(unsigned int threadnum, unsigned int threadcount, std::vector<halfsegment> *myVector, vector<halfsegment> &region, vector<double> &isoBounds,
									vector<halfsegment> &rStrips, vector<int> &stripStopIndex){
	halfsegment workSeg;
	int startBound = 0;
	int threadMin = threadnum * (region.size()/threadcount);
	int threadMax;
	if (threadnum == threadcount -1){
		threadMax = region.size();
	}
	else{
		threadMax = threadMin + (region.size()/threadcount);
	}
	for ( int i = threadMin; i < threadMax; i++){
		if (region[i].isLeft()){
			workSeg = region[i];
			for (int j = startBound; j < isoBounds.size()-1; j++){
				if (workSeg.dx > isoBounds[j+1]){
					startBound++;
					continue;
				}
				else if (workSeg.dx >= isoBounds[j] && workSeg.sx < isoBounds[j+1]){
					workSeg.stripID = j;
					myVector->push_back(workSeg);
					myVector->push_back(workSeg.getBrother());
					break;
				}
				else{
					halfsegment lhs = workSeg;
					lhs.sy = workSeg.dy = workSeg.getYvalAtX(isoBounds[j+1]);
					lhs.sx = workSeg.dx = isoBounds[j+1];
					lhs.stripID = j;
					myVector->push_back(lhs);
					myVector->push_back(lhs.getBrother());
				}
			}
		}
	}

}
void createStrips( unsigned int rThreadNum,  vector< halfsegment> & region, vector<double> &isoBounds, 
									 vector<halfsegment> & rStrips, 	vector< int > &stripStopIndex )
{
	std::chrono::time_point<std::chrono::system_clock> preprocess_start = std::chrono::system_clock::now();
	unsigned int nthread = std::thread::hardware_concurrency() / 2;
	std::vector<std::vector<halfsegment>*> tempVectors;
	for (unsigned int i = 0; i < nthread; i++){
			tempVectors.push_back(new std::vector<halfsegment>);
	}
	std::vector<int> track_threads;
	for (unsigned int i = 0; i < nthread; i++){track_threads.push_back(i);}
	std::for_each( std::execution::par, track_threads.begin(), track_threads.end(), [&] (int i) {
		threadedStrips(i, nthread, tempVectors[i], region, isoBounds, rStrips, stripStopIndex);
	});

	for (unsigned int i = 0; i < nthread; i++){
		for (unsigned int j = 0; j < tempVectors[i]->size(); j++){
			rStrips.push_back(tempVectors[i]->at(j));
		}
	}
	std::sort( rStrips.begin(), rStrips.end(), hsegIDSort);
// END
        // deallocate input segments (to save memory!)
        // use the swap to local var trick!
        //{
          //  vector< halfsegment > localOne;
           // region.swap(localOne);
        //}

	// sort the strips by stripID, then by hseg order

	// find the startIndex for each strip
	for( int i = 0; i < isoBounds.size()-1; i++ )
		stripStopIndex.push_back( std::numeric_limits<int>::min() ); 
	for( int i = 0; i < rStrips.size(); i++ ){
		if( i >= stripStopIndex[ rStrips[i].stripID ] )
			stripStopIndex[ rStrips[i].stripID ] = i+1;
	}
	// remove any remaining min vals
	int prevVal = 0;
	for( int i = 0; i <  stripStopIndex.size(); i++ ) {
		if( stripStopIndex[i] == std::numeric_limits<int>::min()  ) {
			stripStopIndex[i] = prevVal;
		}
		prevVal = stripStopIndex[i];
	}
	
	std::chrono::time_point<std::chrono::system_clock> preprocess_end = std::chrono::system_clock::now();
	std::chrono::duration<double> preprocess_duration = preprocess_end - preprocess_start;
	std::ofstream csv;
	csv.open("preprocessing.csv", std::ofstream::out | std::ofstream::app);
	csv << "Serial Vector," << isoBounds.size() - 1 << "," << preprocess_duration.count() << std::endl;
	csv.close();
#ifdef DEBUG_PRINT
#pragma omp critical
	{
		/*		cerr << "strips: "<<endl;
		for( int i = 0; i < rStrips.size(); i++ ) {
			cerr <<"[(" << rStrips[i].dx << "," << rStrips[i].dy << ")(" << rStrips[i].sx << "," << rStrips[i].sy << ") " << rStrips[i].stripID << ", " << rStrips[i].la << ", " <<  rStrips[i].lb << "]" << endl;
			}*/
		cerr<< "stopIndex: " ;
		for( int i = 0; i < stripStopIndex.size(); i++ )
			cerr << stripStopIndex[i] << " ";
		cerr << endl;
	}
#endif
}

void findIsoBoundaries( vector<halfsegment> &r1, vector<halfsegment> &r2,
											vector< double> & isoBounds )
{

	// set extrema for isobounds
	isoBounds[0] = std::numeric_limits<double>::max() *-1;
	isoBounds[ isoBounds.size()-1 ] =  std::numeric_limits<double>::max();
	
	// find min/max X
	double minX =  std::numeric_limits<double>::max();
	double maxX =  std::numeric_limits<double>::max()*-1;
	if( isoBounds.size() == 2){
		return;
	}
	for( int i = 0; i < r1.size(); i++ ){
		if( r1[i].dx < minX ) minX = r1[i].dx;
		if( r1[i].dx > maxX ) maxX = r1[i].dx;
		if( r1[i].sx < minX ) minX = r1[i].sx;
		if( r1[i].sx > maxX ) maxX = r1[i].sx;
	}
	for( int i = 0; i < r2.size(); i++ ){
		if( r2[i].dx < minX ) minX = r2[i].dx;
		if( r2[i].dx > maxX ) maxX = r2[i].dx;
		if( r2[i].sx < minX ) minX = r2[i].sx;
		if( r2[i].sx > maxX ) maxX = r2[i].sx;
	}
	// calc the middle iso bounds (not the extrema). 
	// need numIsoBounds-2 values spaced evenly between minX and maxX
	double prevIsoVal = minX;
	double stripWidth = (maxX-minX) / (isoBounds.size()-1);
	for( int i = 1; i < isoBounds.size()-1; i++ ) {
		prevIsoVal += stripWidth;
		isoBounds[i] = prevIsoVal;
	} 

#ifdef DEBUG_PRINT
	cerr<< "iso Bounds: " << endl;
	cerr << minX <<","<< maxX<<","<< r1.size() << ","<<r2.size() <<endl;
	for( int i= 0; i < isoBounds.size(); i++ ){
		cerr << isoBounds[i] << ", ";
	}
	cerr << endl;
#endif
	
	// make sure we don't have an iso boundary on an endpoint
	for(  int i = 1; i < isoBounds.size()-1; i++ ) {
		int r1Index, r2Index;
		r1Index = binarySearchExists( r1, isoBounds[i] );
		r2Index = binarySearchExists( r2, isoBounds[i] );
		// move forward to find highest segwith same domX
		if( r1Index >= 0 || r2Index >= 0 ){
			// get smallest greater
			r1Index = binarySearchSmallestGreater( r1, isoBounds[i] );
			r2Index = binarySearchSmallestGreater( r2, isoBounds[i] );
			// cerr << "riInd, r2Ind " << r1Index << ", " << r2Index << endl;
			double xVal = isoBounds[i+1];
			if( r1Index < r1.size() && r1[r1Index].dx < xVal ) {
				xVal =  r1[r1Index].dx;
			}
			if( r2Index < r2.size() && r2[r2Index].dx < xVal ) {
				xVal =  r2[r2Index].dx;
			}
			if( xVal == std::numeric_limits<double>::max() ){
				cerr << "Did not find xVal in splits" << endl;
				exit(-1 );
			}
			// find point between isoX and xVal
			isoBounds[i] = (isoBounds[i]+xVal)/2.0;
		}
	}
	
#ifdef DEBUG_PRINT
	cerr<< "iso Bounds: " << endl;
	for( int i= 0; i < isoBounds.size(); i++ ){
		cerr << isoBounds[i] << ", ";
	}
	cerr << endl;
#endif


}


void createFinalOverlay( vector<halfsegment> & finalResult,
												 vector< vector<halfsegment> > &resultStrips, 
												 const vector<double> &isoBounds )
{
	halfsegment curr;
	int broIndex;
	int currBound;
	int currStrip;
	int currIndex;
	// each strip is sorted.  Join segs with artifical breaks
	// easiest way:  for each isobound
	// 1) grab a non-invalidated seg
	// 2) if it ends on an iso bound, check if it is the only one at that point
	// 3) if so, find the seg and its brother from the next strip, join, invalidate all others
	//    keep the earliest left seg as the valid one
	// 4) if the new seg ends on the next iso bound, repeat, building the seg across all bounds.
	// 5) once seg is finished, put it and its brother in results
	for( int i = 0; i <resultStrips.size(); i++ ) {
		for( int j = 0; j < resultStrips[i].size(); j++ ) {
			curr = resultStrips[i][j];
			// only process left hsegs that are valid;
			if( curr.isLeft() && curr.la != curr.lb ) {
				// if the left end point ends on an iso boundary, merge it
				currIndex = j;
				currBound = i+1;
				currStrip = i;
				if( curr.sx == isoBounds[currBound] ) {
					// find its brother
					if( ! binarySearchHalfsegment(resultStrips[ currStrip ],  curr.getBrother(), broIndex ) ) {
						cerr << "did not find brother 1" <<endl; exit( -1 );
					}
				}
				bool invalidateLast=false;
				while( curr.sx == isoBounds[currBound] ) {
					// check the brother's neighbors
					if( (broIndex+1 < resultStrips[currStrip].size() 
							 && resultStrips[currStrip][broIndex+1].dx == curr.sx
							 && resultStrips[currStrip][broIndex+1].dy == curr.sy )
							|| (broIndex > 0
									&& resultStrips[currStrip][broIndex-1].dx == curr.sx
									&& resultStrips[currStrip][broIndex-1].dy == curr.sy ) ) {
						// multiple segs cross here.  We are done with this seg
						break;
					} else {
						invalidateLast = true; // record that we need to invalidate the last part
						// we have to join this seg with its counterpart in the next strip
						// invalidate the brother
						resultStrips[currStrip][broIndex].la = resultStrips[currStrip][broIndex].lb = -1;
						// invalidate the curr
						resultStrips[currStrip][currIndex].la = resultStrips[currStrip][currIndex].lb = -1;
						// find the seg in the next strip
						binarySearchHalfsegment(resultStrips[ currStrip+1 ],  resultStrips[currStrip][broIndex], currIndex ); 
						// find its brother
						binarySearchHalfsegment(resultStrips[ currStrip+1 ],  resultStrips[currStrip+1][currIndex].getBrother(), broIndex ); 
						// update curr with that segs sub point
						curr.sx = resultStrips[ currStrip+1 ][currIndex].sx;
						curr.sy = resultStrips[ currStrip+1 ][currIndex].sy;
						currStrip++; // we are now in the next strip
						currBound++; // we are now looking at the next bound
					}
				}
				if( invalidateLast ) {
					// invalidate the brother
					resultStrips[currStrip][broIndex].la = resultStrips[currStrip][broIndex].lb = -1;
					// invalidate the curr
					resultStrips[currStrip][currIndex].la = resultStrips[currStrip][currIndex].lb = -1;

				}
				// add the seg and its brother to the output
				finalResult.push_back( curr );
				finalResult.push_back( curr.getBrother() );
			}
		}
	}

}


