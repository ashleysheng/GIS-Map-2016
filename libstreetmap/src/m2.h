#pragma once
#include <string>
#include "graphics.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "m1.h"
#include <iostream>
#include <cfloat> // for FLT_MAX
#include <chrono>
#include <thread>
#include <cstdlib>
#include <vector>
#include <math.h>

//Creates the screen to choose a city
std::string selectScreen();

//Draws the screen for selecting a city
void drawSelectScreen();

//Called whenever mouse is clicked when selecting a city
void mousePressSelectScreen(float x, float y, t_event_buttonPressed event);

//Initialize global variables and draw city map
void initializeMap() ;

//Creates the city map
void drawCityMap();

//Draws the screen for the city map
void drawScreen();

//Draws the travel path
void drawPath(std::vector<unsigned> pathSegs);

//Draws all streets on the map
void drawAllStreets();

//Goes through all features to match type with colour
void drawAllFeatures();

//Draws each individual feature with given colour
void drawFeature(t_color colour, std::vector<unsigned> featureIds);

//Draws all POIs on the map and their names
void drawPOIs();

//Draws all intersections on the map
void drawIntersections();

//Creates white box at bottom of screen to display information
void drawDisplayInfo();

//Called whenever the mouse is clicked on the city map
void mousePressCityMap(float x, float y, t_event_buttonPressed event);

//Called when mouse moves in city map
void mouseMoveCityMap(float x, float y);

//Function called with using find
void findButtonFunc(void (*drawscreen_ptr) (void));

//Function called when toggling night mode
void nightModeFunc(void (*drawscreen_ptr) (void));

//Function called when toggling POIs
void togglePOIFunc(void (*drawscreen_ptr) (void));

//This function selects whether the user is hovering over a street and returns 
// a vector contains its streetID
std::vector<unsigned> chooseHighlightedStreet(float x, float y);

//Highlights or un-highlights every street in a given vector
void drawHighlightedStreets(bool ifHighlightStreets, std::vector<unsigned> highlightedStreets);

//Gets the initial frame for the map
t_bound_box getInitialCoordinates();

//Creates a t_point array with four points to create a rectangle with the given
//dimensions
t_point* makeRectangle(float top, float right, float bottom, float left);

//Called when a keyboard key is pressed
void keyPressCityMap (char c, int keysym);

//Function called when type button is pressed
void gpsButtonFunc (void (*drawscreen_ptr) (void));

//Used to create text boxes
void createTextBox(double positionPercent, double percentHeight, double percentWidth, 
        std::string text, std::string dividingLine, int boxColour = WHITE, std::string special = "");

//Feedback for user telling them which intersections have been selected for pathfinding
void drawSelectIntersectionInfo();

//Called when finished typing to use values input
void doneTyping();

//Called when done selecting intersections
void doneSelectIntersection();

//Called when Help button clicked
void helpButtonFunc (void (*drawscreen_ptr) (void));