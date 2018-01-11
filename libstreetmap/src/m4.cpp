#include "m4.h"
#include "m2.h"
#include <time.h>

unsigned deliveryIndex;
unsigned deliveryIntersection;
float globalTime;
unsigned possibleIntersection[3];

#include <stdlib.h>     /* srand, rand */
#include <map>
#include <utility>
//typedef std::pair<unsigned, std::vector<unsigned> > myPair;
//typedef std::pair<std::vector<unsigned>, unsigned> myPair2;

//std::map<myPair, myPair2 > myMap;
std::map<std::vector<unsigned>, std::vector<unsigned> >myMap;

std::vector<unsigned> traveling_courier(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots) {

    unsigned a = 0;
    //    unsigned b = 0;

    srand(time(NULL));
    std::vector<unsigned> finalReturnSegments;
    double bestTime = 100000000, time;
    std::vector<unsigned> dijikstraReturn;
    clock_t startTime = clock();
    bool randomise = false;
    unsigned sizeOfDeliveries = deliveries.size();
    if (sizeOfDeliveries > 10) randomise = true;
    unsigned randomIndex, randomIndex2, randomIndex3;
    unsigned timeLimit = 25;
    // if (sizeOfDeliveries > 12) timeLimit = 25;
    // if (sizeOfDeliveries > 100) timeLimit = 22;
    unsigned counter;
    std::vector<unsigned> bestDepots;
    std::vector<unsigned> allDepots = depots;

    bool ifFound;
    bool firstIteration = false;

    bool done [sizeOfDeliveries];
    unsigned numIntersections = getNumberOfIntersections();
    unsigned switching = 1;
    std::vector<unsigned> duplicatedDropOffDeliveries, temp;
    bool multi = false;
    for (unsigned i = 0; i < sizeOfDeliveries; i++) {
        for (unsigned ii = 0; ii < sizeOfDeliveries; ii++) {
            if (i != ii && deliveries[i].dropOff == deliveries[ii].dropOff) {
                multi = true;
                duplicatedDropOffDeliveries.push_back(i);
            }
        }
    }
    unsigned sizeOfDuplicatedDropOffDeliveries = duplicatedDropOffDeliveries.size();
    std::vector<unsigned> initialDestinations(sizeOfDeliveries);

    for (unsigned i = 0; i < sizeOfDeliveries; i++) {
        initialDestinations[i] = deliveries[i].pickUp;
    }




    do {
        for (unsigned depotNum = 0; depotNum < allDepots.size(); depotNum++) {
            a++;
            if (randomise) {
                if (multi) {
                    randomIndex = 1 + rand() % (sizeOfDeliveries - 3);
                    randomIndex2 = 1 + rand() % (sizeOfDeliveries - 3);
                    //      randomIndex2 = 1 + rand() % (static_cast<unsigned int> (sizeOfDeliveries * 0.3) - 3);

                    randomIndex3 = 1 + rand() % (sizeOfDeliveries - 3);
                } else {
                    randomIndex = 1 + rand() % (sizeOfDeliveries - 3);
                    randomIndex2 = 0;
                    randomIndex3 = 0;
                }
            }
            unsigned prevIntersection = allDepots[depotNum];
            std::vector<unsigned> returnSegments;
            std::vector<unsigned> possibleDestinations = initialDestinations;
            bool done[sizeOfDeliveries] = {false};
            bool finished = false;
            counter = -1;


            while (!finished) {
                counter++;

                if (firstIteration == true && randomise && (counter == randomIndex || (randomIndex2 != 0 && counter == randomIndex2))) {

                    unsigned nextIntersection;
                    //        unsigned i = -1;
                    //   unsigned i = 1 + rand() % (sizeOfDeliveries - 3);

                    //                    if (multi) {
                    //                        unsigned initialI = i;
                    //                        do {
                    //                            //get a random intersection from possibleDestinations
                    //                            //if its a pick up for multi
                    //                            ifFound = false;
                    //                            if (i == sizeOfDeliveries - 1)
                    //                                i = -1;
                    //                            i++;
                    //                            if (i == initialI) {
                    //                                do {
                    //                                    if (i == sizeOfDeliveries - 1)
                    //                                        i = -1;
                    //                                    i++;
                    //                                    nextIntersection = possibleDestinations[i];
                    //                                } while (nextIntersection >= numIntersections);
                    //                                break;
                    //                            }
                    //                            for (unsigned a = 0; a < sizeOfDuplicatedDropOffDeliveries; a++) {
                    //                                if (i == duplicatedDropOffDeliveries[a]) {
                    //                                    if (possibleDestinations[i] == deliveries[i].pickUp) {
                    //                                        nextIntersection = possibleDestinations[i];
                    //                                        ifFound = true;
                    //                                        break;
                    //                                    }
                    //                                }
                    //                            }
                    //                            if (ifFound) break;
                    //                        } while (true);
                    //                    } else {
                    //                        do {
                    //                            if (i == sizeOfDeliveries - 1)
                    //                                i = -1;
                    //                            i++;
                    //                            nextIntersection = possibleDestinations[i];
                    //                        } while (nextIntersection >= numIntersections);
                    //                    }
                    //                    do {
                    //                        i++;
                    //                        nextIntersection = possibleDestinations[i];
                    //                    } while (nextIntersection >= numIntersections);


                    //std::vector<std::vector<unsigned>> dijkstraThree(unsigned
                    //        intersect_id_start, std::vector<unsigned> possibleDestinations) {

                    std::vector<std::vector<unsigned>> dijkstraThreeReturn = dijkstraThree(prevIntersection, possibleDestinations);
                    std::vector<unsigned> currSegs;
                    if (switching == 2) {
                        currSegs = dijkstraThreeReturn[2];
                        nextIntersection = possibleIntersection[2];
                        switching = 1;
                    } else {
                        currSegs = dijkstraThreeReturn[1];
                        nextIntersection = possibleIntersection[1];
                        switching = 2;
                    }

                    //                    dijikstraReturn = dijkstra(prevIntersection, possibleDestinations);
                    //                    unsigned prevDeliveryIntersection = deliveryIntersection;
                    //                    temp.clear();
                    //                    for (unsigned i = 0; i < sizeOfDeliveries; i++) {
                    //                        if (possibleDestinations[i] == deliveryIntersection) {
                    //                            possibleDestinations[i] = numIntersections + 1;
                    //                            temp.push_back(i);
                    //                        }
                    //                    }
                    //                    dijikstraReturn = dijkstra(prevIntersection, possibleDestinations);
                    //                    nextIntersection = deliveryIntersection;
                    //                    for (unsigned i = 0; i < temp.size(); i++) {
                    //                        possibleDestinations[temp[i]] = prevDeliveryIntersection;
                    //                    }

                    //   std::vector<unsigned> currSegs = find_path_between_intersections(prevIntersection, nextIntersection);



                    for (auto temp = currSegs.begin();
                            temp != currSegs.end(); temp++)
                        returnSegments.push_back(*temp);
                    //update prevIntersection each iteration
                    prevIntersection = nextIntersection;
                    //if an item is being picked up, put the drop off in the possible destinations, else done
                    for (unsigned i = 0; i < sizeOfDeliveries; i++) {
                        if (nextIntersection == possibleDestinations[i]) {
                            if (nextIntersection == deliveries[i].pickUp) {
                                if (deliveries[i].pickUp != deliveries[i].dropOff)
                                    possibleDestinations[i] = deliveries[i].dropOff;
                                else {
                                    done [i] = true;
                                    possibleDestinations[i] = numIntersections + 1;
                                }
                            } else if (nextIntersection == deliveries[i].dropOff) {
                                done [i] = true;
                                possibleDestinations[i] = numIntersections + 1;
                            }
                        }
                    }
                    finished = true;
                    for (unsigned i = 0; i < sizeOfDeliveries; i++) {
                        if (!done[i])
                            finished = false;
                    }

                } else {

                    //      dijikstraReturn.clear();
                    //                             myPair mp = std::make_pair(prevIntersection, possibleDestinations);

                    std::vector<unsigned> temp1 = possibleDestinations;
                    temp1.push_back(prevIntersection);
                    auto search = myMap.find(temp1);
                    if (search != myMap.end()) {
                        // myPair2 mp2 = search->second;
                        std::vector<unsigned> temp2 = search->second;

                        deliveryIntersection = temp2[temp2.size() - 1];
                        temp2.pop_back();
                        dijikstraReturn = temp2;
                        //                        dijikstraReturn = mp2.first;
                        //                        deliveryIntersection = mp2.second;
                    } else {
                        dijikstraReturn = dijkstra(prevIntersection, possibleDestinations);

                        std::vector<unsigned> temp3 = dijikstraReturn;
                        temp3.push_back(deliveryIntersection);
                        myMap[temp1] = temp3;

                        //                        myPair2 mp2 = std::make_pair(dijikstraReturn, deliveryIntersection);
                        //    myMap.insert(std::pair<myPair, myPair2 >(mp, mp2));
                        //                        myMap[mp] = mp2;
                    }



                    for (auto temp = dijikstraReturn.begin();
                            temp != dijikstraReturn.end(); temp++)
                        returnSegments.push_back(*temp);

                    //update prevIntersection each iteration
                    prevIntersection = deliveryIntersection;
                    for (unsigned i = 0; i < sizeOfDeliveries; i++) {
                        if (prevIntersection == possibleDestinations[i]) {
                            if (prevIntersection == deliveries[i].pickUp) {
                                if (deliveries[i].pickUp != deliveries[i].dropOff)
                                    possibleDestinations[i] = deliveries[i].dropOff;
                                else {
                                    done [i] = true;
                                    possibleDestinations[i] = numIntersections + 1;
                                }
                            } else if (prevIntersection == deliveries[i].dropOff) {
                                done [i] = true;
                                possibleDestinations[i] = numIntersections + 1;
                            }
                        }
                    }
                    finished = true;
                    for (unsigned i = 0; i < sizeOfDeliveries; i++) {
                        if (!done[i])
                            finished = false;
                    }
                }
            }
            std::vector<unsigned> dijikstraReturn = dijkstra(prevIntersection, depots);
            for (auto temp = dijikstraReturn.begin();
                    temp != dijikstraReturn.end(); temp++)
                returnSegments.push_back(*temp);

            time = compute_path_travel_time(returnSegments);
            if (time < bestTime) {
                finalReturnSegments = returnSegments;
                bestTime = time;
                clock_t currentTime = clock();
                float timeSecs = ((float) (currentTime - startTime)) / CLOCKS_PER_SEC;
                std::cout << "At " << timeSecs << " s, a better path is generated with time: " << bestTime << std::endl;
                if (firstIteration == false) {
                    bestDepots.push_back(allDepots[depotNum]);
                    if (bestDepots.size() > 12)
                        bestDepots.erase(bestDepots.begin());
                }
            }
            clock_t currentTime = clock();
            float timeSecs = ((float) (currentTime - startTime)) / CLOCKS_PER_SEC;
            globalTime = timeSecs;
            //   std::cout << globalTime << std::endl;
            if (globalTime >= timeLimit)
                break;

        }
        firstIteration = true;
        allDepots = bestDepots;

        if (globalTime >= timeLimit)
            break;
    } while (true);
    myMap.clear();

    std::cout << a << std::endl;
    //   std::cout << b << std::endl;
    drawPath(finalReturnSegments);
    return finalReturnSegments;
}


// Returns the shortest travel time path (vector of street segments) from 
// the start intersection to a point of interest with the specified name.
// If no such path exists, returns an empty (size == 0) vector.

std::vector<unsigned> dijkstra(unsigned
        intersect_id_start, std::vector<unsigned> possibleDestinations) {

    //  vecOfIntersections.clear();

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
    double alternateTime = 0;
    unsigned currentAdjacent = 0;
    bool firstIteration = true;
    bool done = false;

    if (possibleDestinations.empty())
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
        for (unsigned i = 0; i < possibleDestinations.size(); i++) {
            if (possibleDestinations[i] != numIntersections + 1 && visited[possibleDestinations[i]]) { //breaks while loop if every poiNode is found{
                done = true;
                //keep final intersection before breaking
                currentIntersectionID = possibleDestinations[i];
                deliveryIndex = i;
                deliveryIntersection = possibleDestinations[i];
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
    std::vector<unsigned> streetSegmentReturn;
    if (prevIntersection[currentIntersectionID] >= numIntersections || !are_directly_connected(prevIntersection[currentIntersectionID], currentIntersectionID))
        return {
    };

    //iterate backwards to find path
    while (currentIntersectionID != intersect_id_start) {
        streetSegmentReturn.insert(streetSegmentReturn.begin(), prevStreetSeg[currentIntersectionID]);
        //  vecOfIntersections.insert(vecOfIntersections.begin(), prevIntersection[currentIntersectionID]);
        currentIntersectionID = prevIntersection[currentIntersectionID];
    }
    return streetSegmentReturn;
}

std::vector<std::vector<unsigned>> dijkstraThree(unsigned
        intersect_id_start, std::vector<unsigned> possibleDestinations) {
    //variable declaration
    unsigned numIntersections = getNumberOfIntersections();
    double time[numIntersections]; //std::vector<double> time (numIntersections, 10000000);
    unsigned prevStreetSeg[numIntersections];
    unsigned prevIntersection[numIntersections];
    bool visited[numIntersections]; //std::vector<bool> visited (numIntersections, false);
    unsigned currentIntersectionID, savedIntersection[3];
    std::priority_queue<queueElement, std::vector<queueElement>, std::greater<queueElement> > intersectionQueue, checkQueue;
    std::vector<unsigned> adjacentSegments;
    std::vector<unsigned> potentialAdjacentSegments;
    double alternateTime = 0;
    unsigned currentAdjacent = 0;
    bool firstIteration = true;
    unsigned counter = 0;
    std::vector<unsigned> chosen3, maxDestinations;
    for (auto iter = possibleDestinations.begin(); iter != possibleDestinations.end(); iter++)
        if (std::find(maxDestinations.begin(), maxDestinations.end(), *iter) == maxDestinations.end()
                && *iter != numIntersections + 1)
            maxDestinations.push_back(*iter);

    if (possibleDestinations.empty())
        return {
    };

    //initialize arrays
    for (unsigned i = 0; i < numIntersections; i++) {
        time[i] = 10000000;
        visited[i] = false;
        prevIntersection[i] = 10000000;
        prevStreetSeg[i] = 100000000;
    }

    time[intersect_id_start] = 0;
    intersectionQueue.push(queueElement(0, intersect_id_start));
    while (!intersectionQueue.empty()) {

        //search through all poi intersections and break as soon as one of them is found
        //since we traverse in order of time, this will be the shortest Time POI
        for (unsigned i = 0; i < possibleDestinations.size(); i++) {
            if (possibleDestinations[i] != numIntersections + 1 && visited[possibleDestinations[i]]) { //breaks while loop if every poiNode is found{
                if (std::find(chosen3.begin(), chosen3.end(), possibleDestinations[i]) == chosen3.end()) {
                    savedIntersection[counter] = possibleDestinations[i];
                    possibleIntersection[counter] = possibleDestinations[i];
                    chosen3.push_back(possibleDestinations[i]);
                    counter++;
                }
                if (chosen3.size() == 3)
                    break;
            }
        }

        if (chosen3.size() == 3 || chosen3.size() == maxDestinations.size())
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
    std::vector<std::vector<unsigned>> streetSegmentReturn;
    for (int i = 0; i < 3; i++) {
        std::vector<unsigned> temp;
        streetSegmentReturn.push_back(temp);
    }
    if (prevIntersection[savedIntersection[0]] >= numIntersections)
        return {
    };

    //iterate backwards to find path
    for (unsigned i = 0; i < counter; i++) {
        while (savedIntersection[i] != intersect_id_start) {
            streetSegmentReturn[i].insert(streetSegmentReturn[i].begin(), prevStreetSeg[savedIntersection[i]]);
            savedIntersection[i] = prevIntersection[savedIntersection[i]];
        } //Try reverse
    }
    return streetSegmentReturn;
}
