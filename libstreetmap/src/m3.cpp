#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include <iostream>
#include  <vector>
#include <cstdlib>
//using namespace std;

#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <queue>
#include <boost/heap/fibonacci_heap.hpp>






//structure to be used as comparator in priority_queue

struct compare_node {

    bool operator()(const queueElement& n1, const queueElement& n2) const {
        return n1.time < n2.time;
    }
};

//Uses Diekstra's algorithm to finding time needed to get to each intersection expanding
//in order of time needed to get to the node. Stops when it reaches the destination node
//and then iterates through to return the path to get to the destination

std::vector<unsigned> find_path_between_intersections(unsigned
        intersect_id_start, unsigned intersect_id_end) {
    unsigned numIntersections = getNumberOfIntersections();

    //return empty path if either input is out of range
    if (intersect_id_start >= numIntersections || intersect_id_end >= numIntersections)
        return {
    };

    //declaring variables
    double time[numIntersections];
    unsigned prevStreetSeg[numIntersections];
    unsigned prevIntersection[numIntersections];
    bool visited[numIntersections];
    unsigned currentIntersectionID = 0;
    std::priority_queue<queueElement, std::vector<queueElement>, std::greater<queueElement> > intersectionQueue, checkQueue;
    std::vector<unsigned> adjacentSegments;
    std::vector<unsigned> potentialAdjacentSegments;
    double alternateTime = 0;
    unsigned currentAdjacent = 0;
    bool firstIteration = true;

    //initializing arrays
    for (currentIntersectionID = 0; currentIntersectionID < numIntersections; currentIntersectionID++) {
        visited[currentIntersectionID] = false;
        prevIntersection[currentIntersectionID] = 10000000;
        prevStreetSeg[currentIntersectionID] = 10000000;
        time[currentIntersectionID] = 10000000;
    }

    //initialize start node time and put in the queue
    time[intersect_id_start] = 0;
    intersectionQueue.push(queueElement(0, intersect_id_start));


    while (!intersectionQueue.empty()) {

        //if you have the the path for the end intersection, break the loop
        if (visited[intersect_id_end]) //breaks while loop if every node is calculated
            break;
        //pop the shortest time intersection off the queue
        queueElement var = intersectionQueue.top();
        currentIntersectionID = var.getIntersectionID();
        intersectionQueue.pop();

        //only do calculations on the intersection if it was not already visited
        if (!visited[currentIntersectionID]) {
            visited[currentIntersectionID] = true;
            potentialAdjacentSegments.clear();
            adjacentSegments.clear();
            potentialAdjacentSegments = find_intersection_street_segments(currentIntersectionID);

            //filter out one way segments that are going the wrong way
            for (unsigned j = 0; j < potentialAdjacentSegments.size(); j++) {
                auto segInfo = getStreetSegmentInfo(potentialAdjacentSegments[j]);
                if (segInfo.from == currentIntersectionID)
                    adjacentSegments.push_back(potentialAdjacentSegments[j]);
                else if (!segInfo.oneWay)
                    adjacentSegments.push_back(potentialAdjacentSegments[j]);
            }


            for (auto currentSegment = adjacentSegments.begin();
                    currentSegment != adjacentSegments.end(); currentSegment++) {
                auto segInfo = getStreetSegmentInfo(*currentSegment);

                //get adjacent Intersection ID for connected segment
                if (segInfo.from == currentIntersectionID)
                    currentAdjacent = segInfo.to;
                else
                    currentAdjacent = segInfo.from;

                //Only edit intersection information if it has not been visited
                if (visited[currentAdjacent] == false) {
                    alternateTime = time[currentIntersectionID] + find_street_segment_travel_time(*currentSegment);
                    //check if there was a turn from the last segment
                    if (firstIteration || getStreetSegmentInfo(prevStreetSeg[currentIntersectionID]).streetID != getStreetSegmentInfo(*currentSegment).streetID)
                        alternateTime += 0.25;

                    //if the new path to the adjacent intersection is faster than the old path
                    if (alternateTime < time[currentAdjacent]) {
                        intersectionQueue.push(queueElement(alternateTime, currentAdjacent));
                        time[currentAdjacent] = alternateTime;
                        prevStreetSeg[currentAdjacent] = *currentSegment;
                        prevIntersection[currentAdjacent] = currentIntersectionID;
                    }

                }
            }

        }
        firstIteration = false;
    }
    currentIntersectionID = intersect_id_end;
    std::vector<unsigned> streetSegmentReturn;
    if (prevIntersection[currentIntersectionID] >= numIntersections || !are_directly_connected(prevIntersection[currentIntersectionID], currentIntersectionID))
        return {
    };
    //iterate backwards structure to find path
    while (currentIntersectionID != intersect_id_start) {
        streetSegmentReturn.insert(streetSegmentReturn.begin(), prevStreetSeg[currentIntersectionID]);
        currentIntersectionID = prevIntersection[currentIntersectionID];
    }
    return streetSegmentReturn;

}


// Returns the time required to travel along the path specified. The path
// is passed in as a vector of street segment ids, and this function can 
// assume the vector either forms a legal path or has size == 0.
// The travel time is the sum of the length/speed-limit of each street 
// segment, plus 15 seconds per turn implied by the path. A turn occurs
// when two consecutive street segments have different street names.

double compute_path_travel_time(const std::vector<unsigned>& path) {
    double totalTime = 0;
    if (path.size() == 0) {
        return totalTime;
    }
    if (path.size() == 1) {
        return find_street_segment_travel_time(*path.begin());
    }

    //iterates through all segments and add time per segments factor in turns
    auto nextSegment = ++path.begin(), currentSegment = path.begin();
    for (; nextSegment != path.end(); currentSegment++, nextSegment++) {
        totalTime += find_street_segment_travel_time(*currentSegment);
        if (getStreetSegmentInfo(*currentSegment).streetID != getStreetSegmentInfo(*nextSegment).streetID)
            totalTime += 0.25;

    }
    totalTime += find_street_segment_travel_time(*currentSegment);

    return totalTime;
}


// Returns the shortest travel time path (vector of street segments) from 
// the start intersection to a point of interest with the specified name.
// If no such path exists, returns an empty (size == 0) vector.

std::vector<unsigned> find_path_to_point_of_interest(unsigned
        intersect_id_start, std::string point_of_interest_name) {
    //variable declaration
    unsigned numIntersections = getNumberOfIntersections();
    double time[numIntersections]; //std::vector<double> time (numIntersections, 10000000);
    unsigned prevStreetSeg[numIntersections];
    unsigned prevIntersection[numIntersections];
    bool visited[numIntersections]; //std::vector<bool> visited (numIntersections, false);
    unsigned currentIntersectionID = 0;
    std::priority_queue<queueElement, std::vector<queueElement>, std::greater<queueElement> > intersectionQueue, checkQueue;
    std::vector<unsigned> adjacentSegments;
    std::vector<unsigned> potentialAdjacentSegments;
    std::vector<unsigned> poiIntersections;
    double alternateTime = 0;
    unsigned currentAdjacent = 0;
    bool firstIteration = true;
    bool done = false;
    LatLon position;
    unsigned closestIntersection;

    //find closest intersection for each POI
    std::vector<unsigned> poiIDs = getPoiIdsFromName(point_of_interest_name);
    for (auto currentPoi = poiIDs.begin(); currentPoi != poiIDs.end(); currentPoi++) {
        //grab intersection if it was preloaded
        closestIntersection = getPoiIntersection(*currentPoi);
        //calculated it if no preloaded value exists
        if (closestIntersection == numIntersections + 1) {
            position = getPointOfInterestPosition(*currentPoi);
            closestIntersection = find_closest_intersection(position);
        }
        if (getIntersectionGroup(closestIntersection) == getIntersectionGroup(intersect_id_start)) {
            poiIntersections.push_back(closestIntersection);
        }
    }
    if (poiIntersections.empty())
        return {
    };

    //initialize arrays
    for (currentIntersectionID = 0; currentIntersectionID < numIntersections; currentIntersectionID++) {
        time[currentIntersectionID] = 10000000;
        visited[currentIntersectionID] = false;
        prevIntersection[currentIntersectionID] = 10000000;
        prevStreetSeg[currentIntersectionID] = 100000000;
    }

    time[intersect_id_start] = 0;
    intersectionQueue.push(queueElement(0, intersect_id_start));
    while (!intersectionQueue.empty()) {

        //search through all poi intersections and break as soon as one of them is found
        //since we traverse in order of time, this will be the shortest Time POI
        for (auto currentPoiIntersection = poiIntersections.begin();
                currentPoiIntersection != poiIntersections.end() && !done; currentPoiIntersection++) {
            if (visited[*currentPoiIntersection]) { //breaks while loop if every poiNode is found{
                done = true;
                //keep final intersection before breaking
                currentIntersectionID = *currentPoiIntersection;
            }
        }

        if (done)
            break;

        queueElement var = intersectionQueue.top();
        currentIntersectionID = var.getIntersectionID();
        intersectionQueue.pop();
        //don't process intersection if it was already visited
        if (!visited[currentIntersectionID]) {
            visited[currentIntersectionID] = true;
            potentialAdjacentSegments.clear();
            adjacentSegments.clear();
            potentialAdjacentSegments = find_intersection_street_segments(currentIntersectionID);

            //eliminate segments which are one-way in the opposite direction
            for (auto j = potentialAdjacentSegments.begin(); j != potentialAdjacentSegments.end(); j++) {
                StreetSegmentInfo segInfo = getStreetSegmentInfo(*j);
                if (segInfo.from == currentIntersectionID)
                    adjacentSegments.push_back(*j);
                else if (!segInfo.oneWay)
                    adjacentSegments.push_back(*j);
            }

            for (auto currentSegment = adjacentSegments.begin();
                    currentSegment != adjacentSegments.end(); currentSegment++) {
                StreetSegmentInfo segInfo = getStreetSegmentInfo(*currentSegment);
                if (segInfo.from == currentIntersectionID)
                    currentAdjacent = segInfo.to;
                else
                    currentAdjacent = segInfo.from;
                //only calculate adjacents which were not visited
                if (visited[currentAdjacent] == false) {
                    alternateTime = time[currentIntersectionID] + find_street_segment_travel_time(*currentSegment);
                    //check if there was a turn from the last segment
                    if (firstIteration || getStreetSegmentInfo(prevStreetSeg[currentIntersectionID]).streetID != getStreetSegmentInfo(*currentSegment).streetID)
                        alternateTime += 0.25;

                    //if path to intersection is shorter than current saved path, overwrite it
                    if (alternateTime < time[currentAdjacent]) {
                        intersectionQueue.push(queueElement(alternateTime, currentAdjacent));
                        time[currentAdjacent] = alternateTime;
                        prevStreetSeg[currentAdjacent] = *currentSegment;
                        prevIntersection[currentAdjacent] = currentIntersectionID;
                    }
                }
            }
        }
        firstIteration = false;
    }
    double closestTime = 1000000000;
    std::vector<unsigned> streetSegmentReturn;
    if (prevIntersection[currentIntersectionID] >= numIntersections || !are_directly_connected(prevIntersection[currentIntersectionID], currentIntersectionID))
        return {
    };

    //iterate backwards to find path
    while (currentIntersectionID != intersect_id_start) {
        streetSegmentReturn.insert(streetSegmentReturn.begin(), prevStreetSeg[currentIntersectionID]);
        currentIntersectionID = prevIntersection[currentIntersectionID];
    }
    return streetSegmentReturn;
}



// takes in a string, does autocompletion and return an array of strings (max 5)

std::vector<std::string> autoCompletion(std::string requestType, std::string requestToBeCompleted) {
    int stringSize = requestToBeCompleted.size();
    //switch cases
    std::transform(requestToBeCompleted.begin(), requestToBeCompleted.end(), requestToBeCompleted.begin(),
            [](unsigned char c) {
                return std::tolower(c); });
    std::transform(requestType.begin(), requestType.end(), requestType.begin(),
            [](unsigned char c) {
                return std::tolower(c); });
    std::vector<std::string> results;
    std::vector<std::string> intersectionNames = getIntersectionNames();
    std::vector<std::string> poiNames = getPOINames();
    std::vector<std::string> optionNames = {"intersection", "point of interest"};
    if (requestType == "intersection") {
        //going through all the intersection names
        for (std::vector<std::string>::iterator iter = intersectionNames.begin(); iter != intersectionNames.end(); iter++) {
            //compare the first [stringSize] characters of each intersection names with the string sent in
            if (strncmp((*iter).c_str(), requestToBeCompleted.c_str(), stringSize) == 0 &&
                    std::find(results.begin(), results.end(), *iter) == results.end()) {
                //push back if they are the same
                results.push_back(*iter);
            }
            //break if reach max
            if (results.size() == 5) break;
        }
    } else if (requestType == "point of interest") {
        //going through all the poi names
        for (std::vector<std::string>::iterator iter = poiNames.begin(); iter != poiNames.end(); iter++) {
            //compare the first [stringSize] characters of each poi names with the string sent in
            if (strncmp((*iter).c_str(), requestToBeCompleted.c_str(), stringSize) == 0 &&
                    std::find(results.begin(), results.end(), *iter) == results.end()) {
                results.push_back(*iter);
            }
            if (results.size() == 5) break;
        }
    } else if (requestType == "options") {
        for (std::vector<std::string>::iterator iter = optionNames.begin(); iter != optionNames.end(); iter++) {
            if (strncmp((*iter).c_str(), requestToBeCompleted.c_str(), stringSize) == 0) {
                results.push_back(*iter);
            }
            if (results.size() == 5) break;
        }
    }
    return results;
}

bool operator>(const queueElement& LHS, const queueElement& RHS) {
    if (LHS.time > RHS.time)
        return true;
    else
        return false;
}

bool operator<(const queueElement& LHS, const queueElement& RHS) {
    if (LHS.time < RHS.time)
        return true;
    else
        return false;
}
