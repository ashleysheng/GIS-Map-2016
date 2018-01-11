#pragma once //protects against multiple inclusions of this header file

#include <string>
#include <vector>
#include <math.h> 
#include <algorithm>
#include "StreetsDatabaseAPI.h"
#include <unordered_map>

//use these defines if you need earth radius or conversion from degrees to radians
#define EARTH_RADIUS_IN_METERS 6372797.560856
#define DEG_TO_RAD 0.017453292519943295769236907684886

//function to load map streets.bin file
bool load_map(std::string map_name);

//close the loaded map
void close_map();

//function to return street id(s) for a street name
//return a 0-length vector if no street with this name exists.
std::vector<unsigned> find_street_ids_from_name(std::string street_name);//1

//function to return the street segments for a given intersection 
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id);//2

//function to return street names at an intersection (include duplicate street names in returned vector)
std::vector<std::string> find_intersection_street_names(unsigned intersection_id);//3

std::vector<unsigned> find_intersection_street_ids(unsigned intersection_id);

//can you get from intersection1 to intersection2 using a single street segment (hint: check for 1-way streets too)
//corner case: an intersection is considered to be connected to itself
bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2);//1

//find all intersections reachable by traveling down one street segment 
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections
std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id);//2

//for a given street, return all the street segments
std::vector<unsigned> find_street_street_segments(unsigned street_id);//3

//for a given street, find all the intersections
std::vector<unsigned> find_all_street_intersections(unsigned street_id);//1

//function to return all intersection ids for two intersecting streets
//this function will typically return one intersection id between two street names
//but duplicate street names are allowed, so more than 1 intersection id may exist for 2 street names
std::vector<unsigned> find_intersection_ids_from_street_names(std::string street_name1, std::string street_name2);//2

//find distance between two coordinates
double find_distance_between_two_points(LatLon point1, LatLon point2);//3

double find_distance_between_point_and_line(LatLon linepoint1, LatLon linepoint2, LatLon point);

//find the length of a given street segment
double find_street_segment_length(unsigned street_segment_id);//1

//find the length of a whole street
double find_street_length(unsigned street_id);//2

//find the travel time to drive a street segment (time(minutes) = distance(km)/speed_limit(km/hr) * 60
double find_street_segment_travel_time(unsigned street_segment_id);//3

//find the nearest point of interest to a given position
unsigned find_closest_point_of_interest(LatLon my_position);//1

//find the nearest intersection (by ID) to a given position
unsigned find_closest_intersection(LatLon my_position);//2

//find all intersections within a certain radius
std::vector<unsigned> find_nearby_intersections(LatLon my_position, double radius);

//Returns a vector identical to the one given but without repeating values
std::vector<unsigned> copyVectorWithoutRepeats(std::vector<unsigned> &vectorWithCopies);

//Get a list which stores the names of all intersections
//Does not hold repeats or those with <unknown> in the name
std::vector<std::string> getIntersectionNames();

//Get a list of all POI names
//Does not hold repeats
std::vector<std::string> getPOINames();

//Get the intersection ID of an intersection from its lowercase name
std::vector<unsigned> intersectionIdFromLowerCaseName(std::string intersectionName);

std::vector<unsigned> getPoiIdsFromName(std::string poiName);

std::string getProperPoiName (std::string poi);

int getIntersectionGroup(unsigned intersectionID);

unsigned getPoiIntersection (unsigned poiID);