#include "StreetsDatabaseAPI.h"
#include "m1.h"

//Global variables
unsigned *numIntersectionsGlobal;

//Declaring Arrays for all information indexed by intersection ID
std::vector<unsigned> *intersectionIdToStreetSegmentIds;
std::vector<unsigned> *intersectionIdToStreetIds;
std::vector<std::string> *intersectionIdToStreetNames;
std::vector<unsigned> *intersectionIdToAdjacentIntersectionIds;
std::vector<unsigned> *streetIdToSegmentIds;
std::vector<unsigned> *streetIdToIntersectionIds;

std::vector<std::string> *intersectionNames;
std::vector<std::string> *poiNames;
std::unordered_map<std::string, std::vector<unsigned>> *lowerCaseIntersectionNameToID;
std::unordered_map<std::string, std::vector<unsigned>> *poiNameToIds;
int *intersectionGroup;
unsigned *poiToIntersection;

//Declaring Structures for storing information
std::unordered_map<std::string, std::vector<unsigned>> *streetNameToStreetID;
std::unordered_map<std::string, std::vector<unsigned>> *streetNameToIntersectionIds;
std::unordered_map<std::string, std::string> *poiLowerToUpper;


//load the map

bool load_map(std::string map_name) {

    bool load_success = loadStreetsDatabaseBIN(map_name);

    //Assign all total number variables
    unsigned numIntersections = getNumberOfIntersections();
    unsigned numStreets = getNumberOfStreets();
    unsigned numSegments = getNumberOfStreetSegments();
    unsigned numOfPOIs = getNumberOfPointsOfInterest();

    //Set value of globals
    numIntersectionsGlobal = new unsigned (numIntersections);

    //Initializing Arrays that are indexed with intersectionID or streetID
    intersectionIdToStreetSegmentIds = new std::vector<unsigned> [numIntersections];
    intersectionIdToStreetIds = new std::vector<unsigned> [numIntersections];
    intersectionIdToStreetNames = new std::vector<std::string> [numIntersections];
    intersectionIdToAdjacentIntersectionIds = new std::vector<unsigned> [numIntersections];
    streetIdToSegmentIds = new std::vector<unsigned> [numStreets];
    streetIdToIntersectionIds = new std::vector<unsigned> [numStreets];
    intersectionNames = new std::vector<std::string>;
    poiNames = new std::vector<std::string>;

    //Initializing maps
    streetNameToStreetID = new std::unordered_map <std::string, std::vector<unsigned>>;
    streetNameToIntersectionIds = new std::unordered_map <std::string, std::vector<unsigned>>;
    lowerCaseIntersectionNameToID = new std::unordered_map <std::string, std::vector<unsigned>>;
    poiNameToIds = new std::unordered_map <std::string, std::vector<unsigned>>;
    poiLowerToUpper = new std::unordered_map <std::string, std::string>;
    intersectionGroup = new int [numIntersections];
    poiToIntersection = new unsigned [numOfPOIs];

    //Load streetNameToID
    std::string streetName;
    for (unsigned currentStreetID = 0; currentStreetID < numStreets; currentStreetID++) {
        //push back streetID if it's street name matches street_name passed
        streetName = getStreetName(currentStreetID);
        ((*streetNameToStreetID)[streetName]).push_back(currentStreetID);
    }

    //Load all structures with Intersection Key
    StreetSegmentInfo currentSegmentInfo;
    unsigned currentStreetSegmentID, numOfStreetSegments;
    std::vector<unsigned> * adjIntersectionIds;
    std::string currentStreetName;

    //Loop through all intersections
    for (unsigned currentIntersectionID = 0; currentIntersectionID < numIntersections; currentIntersectionID++) {
        intersectionGroup[currentIntersectionID] = -1;
        numOfStreetSegments = getIntersectionStreetSegmentCount(currentIntersectionID);
        bool containsUnknown = false;

        //Loop through all street segments at the intersection
        for (unsigned streetSegCounter = 0; streetSegCounter < numOfStreetSegments; streetSegCounter++) {
            //Get street seg ID and info
            currentStreetSegmentID = getIntersectionStreetSegment(currentIntersectionID, streetSegCounter);
            currentSegmentInfo = getStreetSegmentInfo(currentStreetSegmentID);
            currentStreetName = getStreetName(currentSegmentInfo.streetID);
            if (!currentSegmentInfo.streetID)
                containsUnknown = true;

            intersectionIdToStreetSegmentIds[currentIntersectionID].push_back(currentStreetSegmentID);
            intersectionIdToStreetIds[currentIntersectionID].push_back(currentSegmentInfo.streetID);
            intersectionIdToStreetNames[currentIntersectionID].push_back(currentStreetName);

            //add intersection on the other side of current street segment if direction of travel is valid
            adjIntersectionIds = &intersectionIdToAdjacentIntersectionIds[currentIntersectionID];
            if (currentSegmentInfo.from == currentIntersectionID) {
                adjIntersectionIds->push_back(getStreetSegmentInfo(currentStreetSegmentID).to);
            } else if (!currentSegmentInfo.oneWay) {
                adjIntersectionIds->push_back(getStreetSegmentInfo(currentStreetSegmentID).from);
            }
            streetIdToIntersectionIds[currentSegmentInfo.streetID].push_back(currentIntersectionID);
            (*streetNameToIntersectionIds)[currentStreetName].push_back(currentIntersectionID);
        }

        std::string intersectionName = getIntersectionName(currentIntersectionID);
        if (!containsUnknown) {
            std::transform(intersectionName.begin(), intersectionName.end(),
                    intersectionName.begin(), [](unsigned char c) {
                        return std::tolower(c); });
            ((*lowerCaseIntersectionNameToID)[intersectionName]).push_back(currentIntersectionID);
            (*intersectionNames).push_back(intersectionName);
        }
    }

    //Loops through all street segments
    for (unsigned currentSegmentID = 0; currentSegmentID < numSegments; currentSegmentID++) {
        currentSegmentInfo = getStreetSegmentInfo(currentSegmentID);
        streetIdToSegmentIds[currentSegmentInfo.streetID].push_back(currentSegmentID);
    }


    for (unsigned currPOIID = 0; currPOIID < numOfPOIs; currPOIID++) {
        poiToIntersection[currPOIID] = numIntersections + 1;
        std::string poiName = getPointOfInterestName(currPOIID);
        (*poiNameToIds)[poiName].push_back(currPOIID);
        std::string originalPoiName = poiName;
        std::transform(poiName.begin(), poiName.end(), poiName.begin(),
                [](unsigned char c) {
                    return std::tolower(c); });
        (*poiNames).push_back(poiName);
        (*poiLowerToUpper)[poiName] = originalPoiName;
    }

    std::vector<unsigned> depthFirstQueue;
    depthFirstQueue.push_back(0);
    int currentGroup = -1;
    unsigned currentIntersection;
    bool done;
    while (true) {
        currentGroup++;
        done = true;
        for (int i = 0; i < *numIntersectionsGlobal && done; i++) {
            if (intersectionGroup[i] == -1) {
                done = false;
                intersectionGroup[i] = currentGroup;
                depthFirstQueue.push_back(i);
            }
        }

        if (done)
            break;

        while (!depthFirstQueue.empty()) {
            currentIntersection = depthFirstQueue.back();
            depthFirstQueue.pop_back();
            for (auto nextIntersection = intersectionIdToAdjacentIntersectionIds[currentIntersection].begin();
                    nextIntersection != intersectionIdToAdjacentIntersectionIds[currentIntersection].end(); nextIntersection++) {
                if (intersectionGroup[*nextIntersection] == -1) {
                    intersectionGroup[*nextIntersection] = currentGroup;
                    depthFirstQueue.push_back(*nextIntersection);
                }
            }
        }
    }
    int mod = 0;
    int counter = 0;
    for (unsigned currPOIID = 0; currPOIID < numOfPOIs; currPOIID++) {
        mod++;
        if((*poiNameToIds)[getPointOfInterestName(currPOIID)].size()>20){
            counter++;
            poiToIntersection[currPOIID] = find_closest_intersection(getPointOfInterestPosition(currPOIID));
        }
    }






    return load_success;
}

//close the map

void close_map() {
    closeStreetDatabase();

    // destroy/clear any data structures you created in load_map
    delete streetNameToStreetID;
    delete [] intersectionIdToStreetIds;
    delete [] intersectionIdToStreetNames;
    delete [] intersectionIdToAdjacentIntersectionIds;
    delete [] streetIdToSegmentIds;
    delete [] streetIdToIntersectionIds;
    delete streetNameToIntersectionIds;
    delete [] intersectionIdToStreetSegmentIds;
    delete intersectionNames;
    delete poiNames;
    delete lowerCaseIntersectionNameToID;
    delete [] intersectionGroup;
    delete poiNameToIds;
    //Delete globals
    delete numIntersectionsGlobal;
    delete [] poiToIntersection;

    //Set pointers to Null
    streetNameToStreetID = NULL;
    intersectionIdToStreetIds = NULL;
    intersectionIdToStreetNames = NULL;
    intersectionIdToAdjacentIntersectionIds = NULL;
    streetIdToSegmentIds = NULL;
    streetIdToIntersectionIds = NULL;
    streetNameToIntersectionIds = NULL;
    intersectionIdToStreetSegmentIds = NULL;
    numIntersectionsGlobal = NULL;
    intersectionNames = NULL;
    poiNames = NULL;
    lowerCaseIntersectionNameToID = NULL;
    intersectionGroup = NULL;
    poiNameToIds = NULL;


}

//function to return street id(s) for a street name
//return a 0-length vector if no street with this name exists.

std::vector<unsigned> find_street_ids_from_name(std::string street_name) {
    return (*streetNameToStreetID)[street_name];
}

//Returns the street segment ids for a given intersection id

std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id) {
    return intersectionIdToStreetSegmentIds[intersection_id];
}

std::vector<unsigned> find_intersection_street_ids(unsigned intersection_id) {
    return intersectionIdToStreetIds[intersection_id];
}
//function to return street names at an intersection (include duplicate street names in returned vector)

std::vector<std::string> find_intersection_street_names(unsigned intersection_id) {
    return intersectionIdToStreetNames[intersection_id];
}

//Checks if both intersection are identical
//Then checks if second intersection is in the list of adjacent intersections to
//intersection 1

bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2) {
    //test if two intersections are the same
    if (intersection_id1 == intersection_id2)
        return true;
    std::vector<unsigned> adjacentIntersections = intersectionIdToAdjacentIntersectionIds[intersection_id1];
    if (std::find(adjacentIntersections.begin(), adjacentIntersections.end(),
            intersection_id2) != adjacentIntersections.end())
        return true;
    return false;
};

//returns all adjacent intersections without duplicates

std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id) {
    return copyVectorWithoutRepeats(intersectionIdToAdjacentIntersectionIds[intersection_id]);
}

//for a given street, return all the street segments

std::vector<unsigned> find_street_street_segments(unsigned street_id) {
    return streetIdToSegmentIds[street_id];
}

//for a given street, return all the intersections without duplicates

std::vector<unsigned> find_all_street_intersections(unsigned street_id) {
    return copyVectorWithoutRepeats(streetIdToIntersectionIds[street_id]);
}

//function to return all intersection ids for two intersecting streets
//this function will typically return one intersection id between two street names
//but duplicate street names are allowed, so more than 1 intersection id may exist for 2 street names

std::vector<unsigned> find_intersection_ids_from_street_names(std::string street_name1,
        std::string street_name2) {

    //Get a vector of intersections for each street
    std::vector<unsigned> streetOneIntersections = (*streetNameToIntersectionIds)[street_name1],
            streetTwoIntersections = (*streetNameToIntersectionIds)[street_name2],
            intersectionIDs;

    //Compare the two vectors for identical intersections
    for (std::vector<unsigned>::iterator currentIntersection = streetOneIntersections.begin();
            currentIntersection != streetOneIntersections.end(); currentIntersection++)
        if (std::find(streetTwoIntersections.begin(), streetTwoIntersections.end(),
                *currentIntersection) != streetTwoIntersections.end())
            intersectionIDs.push_back(*currentIntersection);

    //Return a vector without identical values
    return copyVectorWithoutRepeats(intersectionIDs);
}

//Converts co-ordinates in polar form and then calculates the distance between them.

double find_distance_between_two_points(LatLon point1, LatLon point2) {

    //Convert latitude and longitude in polar co-ordinates
    double latAverage, point1X, point2X, point1Y, point2Y, cosOfLatAverage,
            length, displacementX, displacementY;
    point1Y = point1.lat * DEG_TO_RAD;
    point2Y = point2.lat * DEG_TO_RAD;
    latAverage = (point1Y + point2Y) / 2;

    //Calculate cos using a taylor series
    cosOfLatAverage = 1 - latAverage * latAverage / 2 +
            latAverage * latAverage * latAverage * latAverage / 24
            - latAverage * latAverage * latAverage * latAverage * latAverage * latAverage / 720;
    point1X = cosOfLatAverage * point1.lon * DEG_TO_RAD;
    point2X = cosOfLatAverage * point2.lon * DEG_TO_RAD;

    //Calculate length of point
    displacementX = point2X - point1X;
    displacementY = point2Y - point1Y;
    length = EARTH_RADIUS_IN_METERS * sqrt(displacementY * displacementY +
            displacementX * displacementX);
    return length;
}

double find_distance_between_point_and_line(LatLon a, LatLon b, LatLon point) {

    double magnitude_AB = (b.lon - a.lon)*(b.lon - a.lon) + (b.lat - a.lat)*(b.lat - a.lat);
    double dotProduct = (b.lon - a.lon)*(point.lon - a.lon) + (b.lat - a.lat)*(point.lat - a.lat);
    double closestx, closesty;


    if ((dotProduct / magnitude_AB) < 0) {
        closestx = a.lon;
        closesty = a.lat;
    } else if ((dotProduct / magnitude_AB) > 1) {
        closestx = b.lon;
        closesty = b.lat;
    } else {
        closestx = a.lon + (b.lon - a.lon)*(dotProduct / magnitude_AB);
        closesty = a.lat + (b.lat - a.lat)*(dotProduct / magnitude_AB);
    }


    return find_distance_between_two_points(LatLon(closesty, closestx), point);



}



//add start point, curve points and end point of street segment to a vector
//calculates distance between each point in the vector and adds to total distance

double find_street_segment_length(unsigned street_segment_id) {
    StreetSegmentInfo streetSegment = getStreetSegmentInfo(street_segment_id);
    LatLon startCoords = getIntersectionPosition(streetSegment.from);
    LatLon endCoords = getIntersectionPosition(streetSegment.to);
    std::vector<LatLon> allCoords;

    //add start coordinate to allCoors
    allCoords.push_back(startCoords);

    //add all curve points to allCoors
    for (unsigned currentCurvePoint = 0; currentCurvePoint < streetSegment.curvePointCount;
            currentCurvePoint++)
        allCoords.push_back(getStreetSegmentCurvePoint(street_segment_id, currentCurvePoint));

    //add end coordinate to allCoors
    allCoords.push_back(endCoords);
    double distance = 0;
    std::vector<LatLon>::iterator nextCurvePoint = ++allCoords.begin();

    //iterates through every point in the vector and adds the distance to the next one
    for (std::vector<LatLon>::iterator currentCurvePoint = allCoords.begin();
            nextCurvePoint != allCoords.end(); ++currentCurvePoint, ++nextCurvePoint) {
        distance += find_distance_between_two_points(*currentCurvePoint, *nextCurvePoint);
    }
    return distance;
}

//find the length of a whole street by adding length of each segment on that street

double find_street_length(unsigned street_id) {
    double streetLength = 0;

    //find all the street segments that belong to that street
    std::vector<unsigned> streetSegments = find_street_street_segments(street_id);
    for (std::vector<unsigned>::iterator currentSegment = streetSegments.begin();
            currentSegment != streetSegments.end(); currentSegment++) {
        streetLength += find_street_segment_length(*currentSegment);
    }
    return streetLength;
}

//Find street segment length in kilometres and speed of the segment, then divide 
//the two and convert to minutes

double find_street_segment_travel_time(unsigned street_segment_id) {
    const double MetresToKM = 1000, HoursToMinutes = 60;
    double time, distanceKM = find_street_segment_length(street_segment_id) / MetresToKM;
    StreetSegmentInfo streetSegment = getStreetSegmentInfo(street_segment_id);
    double speed = streetSegment.speedLimit;
    time = distanceKM / speed * HoursToMinutes;
    return time;
}

//find the nearest point of interest to a given position
//iterates through all the POIs to find which one is closest to my_position

unsigned find_closest_point_of_interest(LatLon my_position) {
    unsigned numPOI = getNumberOfPointsOfInterest();
    unsigned currentPOI = 0, closestPOI = 0;
    double shortestDistance = find_distance_between_two_points(
            getPointOfInterestPosition(currentPOI), my_position);
    double currentDistance;

    for (currentPOI = 1; currentPOI < numPOI; currentPOI++) {
        currentDistance = find_distance_between_two_points(
                getPointOfInterestPosition(currentPOI), my_position);

        //compares current calculated distance to shortest distance so far
        if (currentDistance < shortestDistance) {
            closestPOI = currentPOI;
            shortestDistance = currentDistance;
        }
    }
    return closestPOI;
}

//Goes through all intersections and compares the distance from current position
//Returns the intersection which was closest

unsigned find_closest_intersection(LatLon my_position) {
    unsigned closestIntersection = 0;
    //Set first distance from intersection 0
    double closestDistance = find_distance_between_two_points(my_position,
            getIntersectionPosition(0)), intersectionDistance;
    LatLon intersectionPosition;
    for (unsigned currentIntersection = 1; currentIntersection < *numIntersectionsGlobal; currentIntersection++) {
        intersectionPosition = getIntersectionPosition(currentIntersection);
        intersectionDistance = find_distance_between_two_points(my_position, intersectionPosition);
        if (intersectionDistance < closestDistance) {
            closestDistance = intersectionDistance;
            closestIntersection = currentIntersection;
        }
    }
    return closestIntersection;
}

std::vector<unsigned> find_nearby_intersections(LatLon my_position, double radius) {
    LatLon intersectionPosition;
    double intersectionDistance;
    std::vector<unsigned> nearbyIntersections;
    for (unsigned currentIntersection = 0; currentIntersection < *numIntersectionsGlobal; currentIntersection++) {
        intersectionPosition = getIntersectionPosition(currentIntersection);
        intersectionDistance = find_distance_between_two_points(my_position, intersectionPosition);
        if (intersectionDistance < radius) {
            nearbyIntersections.push_back(currentIntersection);
        }
    }
    return nearbyIntersections;
}


//Returns a vector identical to the one given but without repeating values

std::vector<unsigned> copyVectorWithoutRepeats(std::vector<unsigned> &vectorWithCopies) {
    std::vector<unsigned> vectorWithoutRepeats;
    for (std::vector<unsigned>::iterator iter = vectorWithCopies.begin(); iter != vectorWithCopies.end(); iter++)
        if (std::find(vectorWithoutRepeats.begin(), vectorWithoutRepeats.end(), *iter) == vectorWithoutRepeats.end())
            vectorWithoutRepeats.push_back(*iter);
    return vectorWithoutRepeats;
}

std::vector<std::string> getIntersectionNames() {
    return (*intersectionNames);
}

std::vector<std::string> getPOINames() {
    return (*poiNames);
}

std::vector<unsigned> intersectionIdFromLowerCaseName(std::string intersectionName) {
    return (*lowerCaseIntersectionNameToID)[intersectionName];
}

std::vector<unsigned> getPoiIdsFromName(std::string poiName) {
    return (*poiNameToIds)[poiName];

}

std::string getProperPoiName(std::string poi) {
    return (*poiLowerToUpper)[poi];
}

int getIntersectionGroup(unsigned intersectionID) {
    return intersectionGroup[intersectionID];

}

unsigned getPoiIntersection(unsigned poiID) {
    return poiToIntersection[poiID];

}