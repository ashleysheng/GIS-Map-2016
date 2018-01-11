/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "m2.h"
#include "m3.h"

std::vector<unsigned> highlightedStreets;
std::vector<unsigned> unhighlightedStreets;

t_point *highlightedIntersection = NULL;
static bool nightMode = false;
static bool showPOIs = true;
static bool featuresNotSorted = true;
static std::string selectedCity = "close";
static std::string textForDisplay = "Currently Hovering Over: ";
bool ifHighlightStreets = true;

const t_color GREENSPACE(176, 251, 129);
const t_color PARK(155, 246, 179);
const t_color GOLFCOURSE(53, 210, 93);
const t_color LAKE(155, 246, 245);
const t_color RIVER(50, 234, 231);
//const t_color BACKGROUND(248, 243, 197); //Yellow
//const t_color BACKGROUND(84, 84, 84); //Grey
const t_color BACKGROUND(119, 136, 153); //Slate Grey

const unsigned int MaxStrings = 5, MaxIntersections = 2;
unsigned int currentString, currentStringMax, currentIntersection;
unsigned int savedIntersection[MaxIntersections], savedIntersectionIds[MaxStrings];
unsigned int hoverAutocomplete = 10;
std::string typingString[MaxStrings] = "";
bool selectGPS = false;
bool typing = false;
bool selectIntersection = false;
bool singleCap = false, capLock = false;
unsigned int cursor = 0;
bool error = false, help = false;
bool drawingPath = false;
bool drawPOI = false;
std::vector<unsigned> path;
std::string errorMessage = "Error: Error should not be displayed";
std::string *textUI = NULL;
std::string instructionsGPS[] = {"What are you looking for (intersection or point of interest)",
    "Where are you starting?",
    "Where would you like to go?"};
std::string inputHelper[] = {"options", "intersection"};
std::string *gpsInputType[] = {&inputHelper[0], &inputHelper[1], &typingString[0]};
std::string hoveringOverChoice = "";
const unsigned helpLines = 6;
std::string helpMessage[] = {"Help",
    "Directions: Find directions from an intersection to another intersection or point of interest.",
    "Night On/Off: Enable or disable night mode. Night mode gives a navy blue background to reduce eye strain at night.",
    "Hide/Show POIs: Dictates whether points of interest are drawn on screen.",
    "Find: Find an intersection, point of interest, or latitude longitude position on the current map.",
    "City Select: Pick another city to display."};
std::vector<std::string> autocompleteOptions;
std::string routeForDisplay = "BEGIN";
unsigned int routeNum = 0;
std::vector<unsigned> streetsOfRoute;
double currentPathTime = 0, currentPathDistance = 0;

//Initialize global variables and draw map

void initializeMap() {
    drawingPath = false;
    selectGPS = false;
    typing = false;
    selectIntersection = false;
    featuresNotSorted = true;
    drawCityMap();
    selectedCity = "close";
    textUI = NULL;
}

//Gets the initial frame for the map

t_bound_box getInitialCoordinates() {
    //Finds furthest feature in each direction
    float lonMin = getFeaturePoint(0, 0).lon, lonMax = getFeaturePoint(0, 0).lon,
            latMin = getFeaturePoint(0, 0).lat, latMax = getFeaturePoint(0, 0).lat;
    for (unsigned i = 0; i < getNumberOfFeatures(); i++) {
        for (unsigned ii = 0; ii < getFeaturePointCount(i); ii++) {
            LatLon ll = getFeaturePoint(i, ii);
            if (ll.lon < lonMin) lonMin = ll.lon;
            if (ll.lon > lonMax) lonMax = ll.lon;
            if (ll.lat < latMin) latMin = ll.lat;
            if (ll.lat > latMax) latMax = ll.lat;
        }
    }
    return t_bound_box(lonMin, latMin, lonMax, latMax);
}

//Creates the screen to choose a city

std::string selectScreen() {
    init_graphics("Maps Simple", BACKGROUND);
    t_bound_box selectScreen(0, 0, 60, 60);
    set_visible_world(selectScreen);
    update_message("Select a city and press proceed.");
    event_loop(mousePressSelectScreen, NULL, NULL, drawSelectScreen);
    //returns the url segment of the city the user selects
    return selectedCity;
}

//Creates the city map

void drawCityMap() {
    init_graphics("Maps Simple", BACKGROUND);
    set_visible_world(getInitialCoordinates());
    update_message("Map of " + selectedCity + ".");
    create_button("Window", "Find", findButtonFunc);

    //Checks if showPOIs is active o decide which button text to use
    if (showPOIs)
        create_button("Window", "Hide POIs", togglePOIFunc);
    else
        create_button("Window", "Show POIs", togglePOIFunc);

    //Checks night mode to decide which button text to use
    if (nightMode)
        create_button("Window", "Night Off", nightModeFunc);
    else
        create_button("Window", "Night On", nightModeFunc);

    //Allows for typing
    create_button("Window", "Directions", gpsButtonFunc);

    create_button("Window", "Help", helpButtonFunc);

    set_mouse_move_input(true);
    std::cout << "Done." << std::endl;
    event_loop(mousePressCityMap, mouseMoveCityMap, keyPressCityMap, drawScreen);
    std::cout << "Map closed." << std::endl;

    //Checks which mode map is currently in order to know which buttons to destroy
    if (nightMode)
        destroy_button("Night Off");
    else
        destroy_button("Night On");
    if (showPOIs)
        destroy_button("Hide POIs");
    else
        destroy_button("Show POIs");
    destroy_button("Directions");
    destroy_button("Help");
    destroy_button("Find");

}

//Highlights or un-highlights every street in a given vector

void drawHighlightedStreets(bool ifHighlightStreets, std::vector<unsigned> highlightedStreets) {
    bool wasDone = false;
    t_bound_box currCoords = get_visible_world();
    bool ifDrawMinorStreet = false, ifDrawMediumStreet = false, ifMajor = false, ifMedium = false, ifMinor = false;
    if (currCoords.area() / getInitialCoordinates().area() < 0.005)
        ifDrawMinorStreet = true;
    if (currCoords.area() / getInitialCoordinates().area() < 0.05)
        ifDrawMediumStreet = true;


    int size = highlightedStreets.size();
    std::vector<unsigned> streetSegs[size];
    int i = 0;
    for (std::vector<unsigned>::iterator iter = highlightedStreets.begin(); iter != highlightedStreets.end(); iter++, i++) {
        streetSegs[i] = find_street_street_segments(*iter);
        if (*iter != 0) {
            for (std::vector<unsigned>::iterator currentStreetSeg = streetSegs[i].begin(); currentStreetSeg != streetSegs[i].end(); currentStreetSeg++) {
                auto streetSegInfo = getStreetSegmentInfo(*currentStreetSeg);
                //highlighting style for major street
                if (streetSegInfo.speedLimit >= 70) {
                    setlinestyle(SOLID);
                    setlinewidth(3);
                    ifMajor = true;
                    ifMedium = false;
                    ifMinor = false;
                    //highlighting style for medium streets    
                } else if (streetSegs[i].size() >= 30) {
                    setlinestyle(SOLID);
                    setlinewidth(2);
                    ifMedium = true;
                    ifMajor = false;
                    ifMinor = false;
                    //highlighting style for minor streets
                } else {
                    setlinestyle(DASHED);
                    setlinewidth(2);
                    ifMinor = true;
                    ifMajor = false;
                    ifMedium = false;
                }

                unsigned numOfCurvePoints = getStreetSegmentInfo(*currentStreetSeg).curvePointCount;
                std::vector<t_point> tp;
                //get the position of the from end of the segment
                tp.push_back({getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).from).lon, getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).from).lat});
                //get the positions of all curve points of that segment
                for (unsigned ii = 0; ii < numOfCurvePoints; ii++) {
                    tp.push_back({getStreetSegmentCurvePoint(*currentStreetSeg, ii).lon, getStreetSegmentCurvePoint(*currentStreetSeg, ii).lat});
                }

                //get the position of the to end of the segment
                tp.push_back({getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).to).lon, getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).to).lat});
                for (std::vector<t_point>::iterator currPoint = tp.begin(); currPoint != tp.end() - 1; currPoint++) {

                    //if selecting, set colour to orange
                    if (ifMajor || (ifMedium && ifDrawMediumStreet) || (ifMinor && ifDrawMinorStreet)) {
                        wasDone = true;
                        if (ifHighlightStreets) {
                            setcolor(ORANGE);
                            drawline(*currPoint, *(currPoint + 1));


                            //if deselecting, set colour to white
                        } else {
                            setcolor(WHITE);
                            drawline(*currPoint, *(currPoint + 1));
                        }
                    }
                }
            }
        }
    }
    if (wasDone && drawingPath)
        drawPath(path);
    if (wasDone)
        drawDisplayInfo();
    if (wasDone && selectIntersection)
        drawSelectIntersectionInfo();
}

std::string getRouteForDisplay() {
    if (!streetsOfRoute.empty()) {
        currentPathTime = 0;
        currentPathDistance = 0;
        for (std::vector<unsigned>::iterator currentStreetSeg = path.begin(); currentStreetSeg != path.end(); currentStreetSeg++) {
            if (getStreetSegmentInfo(*currentStreetSeg).streetID == *(streetsOfRoute.begin() + routeNum - 1)) {
                currentPathTime += find_street_segment_travel_time(*(currentStreetSeg));
                currentPathDistance += find_street_segment_length(*(currentStreetSeg));
            }
        }
    }
    int intCurrentPathTime1 = floor(currentPathTime); 
    int intCurrentPathTime2 = floor((currentPathTime - intCurrentPathTime1)*100);
    int intCurrentDistanceTime1 = floor(currentPathDistance); 
    int intCurrentDistanceTime2 = floor((currentPathDistance - intCurrentDistanceTime1)*100);
    return "Turn on " + getStreetName(*(streetsOfRoute.begin() + routeNum - 1)) + " drive for " + std::to_string(intCurrentPathTime1) + "." + std::to_string(intCurrentPathTime2) + " mins (" + std::to_string(intCurrentDistanceTime1) + "." + std::to_string(intCurrentDistanceTime2) + "m)";
}
// draw the path

void drawPath(std::vector<unsigned> pathSegs) {
    if (!streetsOfRoute.empty()) {
        for (std::vector<unsigned>::iterator currentStreetSeg = pathSegs.begin(); currentStreetSeg != pathSegs.end(); currentStreetSeg++) {
            if (getStreetSegmentInfo(*currentStreetSeg).streetID == *(streetsOfRoute.begin() + routeNum - 1)) {
                setcolor(GREEN);
            } else
                setcolor(RED);
            std::vector<t_point> tp;
            unsigned numOfCurvePoints = getStreetSegmentInfo(*currentStreetSeg).curvePointCount;
            //get the position of the from end of the segment
            tp.push_back({getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).from).lon, getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).from).lat});
            //get the positions of all curve points of that segment
            for (unsigned ii = 0; ii < numOfCurvePoints; ii++) {
                tp.push_back({getStreetSegmentCurvePoint(*currentStreetSeg, ii).lon, getStreetSegmentCurvePoint(*currentStreetSeg, ii).lat});
            }
            //get the position of the to end of the segment
            tp.push_back({getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).to).lon, getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).to).lat});

            for (std::vector<t_point>::iterator currPoint = tp.begin(); currPoint != tp.end() - 1; currPoint++) {
                setlinewidth(1000);
                setlinestyle(SOLID);
                drawline(*currPoint, *(currPoint + 1));
            }
        }
    }
    t_bound_box bounds = get_visible_world();
    float height = bounds.get_height();
    float width = bounds.get_width();
    setcolor(PURPLE);
    setlinewidth(5);
    float x = getIntersectionPosition(savedIntersection[1]).lon;
    float y = getIntersectionPosition(savedIntersection[1]).lat;
    t_point tp1(x, y);
    t_point tp2(x, y + height * 0.06);
    drawline(tp1, tp2);

    t_point *flagPolyPoints;
    flagPolyPoints = makeRectangle(y + height * 0.06, x + width * 0.04, y + height * 0.03, x);
    fillpoly(flagPolyPoints, 4);

    x = getIntersectionPosition(savedIntersection[0]).lon;
    y = getIntersectionPosition(savedIntersection[0]).lat;
    t_point one1, two1, three1, one2, two2, three2;
    one1.x = x;
    one1.y = y;
    two1.x = x + width * 0.05;
    two1.y = y;
    three1.x = x + width * 0.025;
    three1.y = y + height * 0.043;
    one2.x = x;
    one2.y = y + height * 0.0286;
    two2.x = x + width * 0.025;
    two2.y = y - height * 0.0144;
    three2.x = x + width * 0.05;
    three2.y = y + height * 0.0286;
    t_point star1PolyPoints[] = {one1, two1, three1};
    t_point star2PolyPoints[] = {one2, two2, three2};
    setcolor(YELLOW);
    fillpoly(star1PolyPoints, 3);
    fillpoly(star2PolyPoints, 3);


    createTextBox(0.1, 0.10, 1, routeForDisplay, "all", WHITE, "directions");
    //void createTextBox(double positionPercent, double percentHeight, double percentWidth,
    //    std::string text, std::string dividingLine, int boxColour, std::string special)
}

//Draws all streets on the map

void drawAllStreets() {
    //depending on the zoom in factor, print major/minor streets accordingly
    t_bound_box currCoords = get_visible_world();
    bool ifPrintMajorStreetName = false, ifPrintMinorStreetName = false, ifDrawMinorStreet = false, ifDrawMediumStreet = false, ifPrintOneWay = false;
    //Make else if
    if (currCoords.area() / getInitialCoordinates().area() < 0.0003)
        ifPrintOneWay = true;
    if (currCoords.area() / getInitialCoordinates().area() < 0.0005)
        ifPrintMinorStreetName = true;
    if (currCoords.area() / getInitialCoordinates().area() > 0.008) {
        ifPrintMajorStreetName = false;
        ifPrintMinorStreetName = false;
    }
    if (currCoords.area() / getInitialCoordinates().area() < 0.005)
        ifDrawMinorStreet = true;
    if (currCoords.area() / getInitialCoordinates().area() < 0.05) {
        ifDrawMediumStreet = true;
        ifPrintMajorStreetName = true;
    }

    bool major = false, minor = false, medium = false;
    unsigned numOfStreets = getNumberOfStreets();

    //looping through all streets
    for (unsigned currStreetId = 0; currStreetId < numOfStreets; currStreetId++) {
        //get all the segments of this street

        std::vector<unsigned> streetSegs = find_street_street_segments(currStreetId);

        //looping through all the segments
        int StreetSegmentCount = -1; //segment index
        for (std::vector<unsigned>::iterator currentStreetSeg = streetSegs.begin(); currentStreetSeg != streetSegs.end(); currentStreetSeg++) {
            StreetSegmentCount++;

            StreetSegmentInfo streetSeg = getStreetSegmentInfo(*currentStreetSeg);
            if (streetSeg.speedLimit >= 70) {
                setcolor(WHITE);
                setlinestyle(SOLID);
                setlinewidth(3);
                major = true;
                minor = false;
                medium = false;
            } else if (streetSegs.size() >= 30 && streetSeg.streetID != 0) {
                setcolor(WHITE);
                setlinestyle(SOLID);
                setlinewidth(2);
                major = false;
                minor = false;
                medium = true;
            } else {
                setcolor(WHITE);
                setlinestyle(DASHED);
                setlinewidth(2);
                minor = true;
                major = false;
                medium = false;
            }
            unsigned numOfCurvePoints = getStreetSegmentInfo(*currentStreetSeg).curvePointCount;
            std::vector<t_point> tp;
            //get the position of the from end of the segment
            tp.push_back({getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).from).lon, getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).from).lat});
            //get the positions of all curve points of that segment
            for (unsigned ii = 0; ii < numOfCurvePoints; ii++) {
                tp.push_back({getStreetSegmentCurvePoint(*currentStreetSeg, ii).lon, getStreetSegmentCurvePoint(*currentStreetSeg, ii).lat});
            }
            //get the position of the to end of the segment
            tp.push_back({getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).to).lon, getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).to).lat});
            //drawing the street segment
            if (ifDrawMinorStreet || (ifDrawMediumStreet && medium) || major) {
                for (std::vector<t_point>::iterator currPoint = tp.begin(); currPoint != tp.end() - 1; currPoint++) {
                    drawline(*currPoint, *(currPoint + 1));
                }
            }

            //print One Way signs if sufficiently zoomed in and street is oneway
            if (ifPrintOneWay && getStreetSegmentInfo(*currentStreetSeg).oneWay) {
                //setcolor(DARKGREY);
                setcolor(ORANGE);
                t_point tp1, tp2;
                tp1 = *tp.begin();
                tp2 = *(tp.begin() + 1);
                double theta = atan((tp2.y - tp1.y) / (tp2.x - tp1.x));
                settextrotation(theta * 180 / 3.14);
                LatLon from = getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).from);
                LatLon to = getIntersectionPosition(getStreetSegmentInfo(*currentStreetSeg).to);

                //use coordinates of from and to points and angle to determine direction of arrows
                if (from.lon < to.lon)
                    drawtext((tp1.x + tp2.x) / 2, (tp1.y + tp2.y) / 2, ">", 150, FLT_MAX);
                else if (from.lon > to.lon)
                    drawtext((tp1.x + tp2.x) / 2, (tp1.y + tp2.y) / 2, "<", 150, FLT_MAX);
                else {
                    settextrotation(90);
                    if (from.lat < to.lat)
                        drawtext((tp1.x + tp2.x) / 2, (tp1.y + tp2.y) / 2, ">", 150, FLT_MAX);
                    else
                        drawtext((tp1.x + tp2.x) / 2, (tp1.y + tp2.y) / 2, "<", 150, FLT_MAX);
                }
            }

            // if zoomed in, print the street names on every 30th segments
            if (currStreetId && ((ifPrintMajorStreetName && major) || (ifPrintMinorStreetName && (minor || medium))) && (StreetSegmentCount % 60 == 0)) {
                t_point tp1, tp2;
                tp1 = *tp.begin();
                tp2 = *(tp.begin() + 1);
                double theta = atan((tp2.y - tp1.y) / (tp2.x - tp1.x));
                settextrotation(theta * 180 / 3.14);
                //setcolor(DARKGREY);
                setcolor(ORANGE);
                drawtext((tp1.x + tp2.x) / 2, (tp1.y + tp2.y) / 2, getStreetName(currStreetId), 150, FLT_MAX);
            }

        }
    }
}

//Draws each individual feature with given colour

void drawFeature(t_color colour, std::vector<unsigned> featureIds) {
    t_bound_box currCoords = get_visible_world();
    float left = currCoords.left() - currCoords.get_width()*0.2;
    float right = currCoords.right() + currCoords.get_width()*0.2;
    float top = currCoords.top() - currCoords.get_height()*0.2;
    float bottom = currCoords.bottom() + currCoords.get_height()*0.2;
    setlinestyle(SOLID);
    setlinewidth(2);

    //looping through all features
    for (std::vector<unsigned>::iterator currentFeatureId = featureIds.begin(); currentFeatureId != featureIds.end(); currentFeatureId++) {
        unsigned numOfPoints = getFeaturePointCount(*currentFeatureId);
        unsigned ii;
        t_point polypts[numOfPoints];
        bool ifOffScreen = true;
        //looping through all points
        for (ii = 0; ii < numOfPoints; ii++) {
            polypts[ii].x = getFeaturePoint(*currentFeatureId, ii).lon;
            polypts[ii].y = getFeaturePoint(*currentFeatureId, ii).lat;
            if (polypts[ii].x < right && polypts[ii].x > left && polypts[ii].y > bottom && polypts[ii].y < top) {
                ifOffScreen = false;
            }
        }
        if (ifOffScreen == false) {
            setcolor(colour);
            if (polypts[0].x == polypts[numOfPoints - 1].x && polypts[0].y == polypts[numOfPoints - 1].y) { //closed feature
                fillpoly(polypts, numOfPoints);
            } else {
                for (unsigned iii = 0; iii < numOfPoints - 1; iii++)
                    drawline(polypts[iii], polypts[iii + 1]);
            }
        }
    }
}

//Goes through all features to match type with colour

void drawAllFeatures() {
    //                            0        1         2      3          4          5        6          7      8          9        10      
    static t_color colour[11] = {LAKE, DARKKHAKI, CORAL, LIGHTGREY, GOLFCOURSE, PARK, MEDIUMPURPLE, KHAKI, GREENSPACE, RIVER, TURQUOISE};
    static std::vector<unsigned> arrayOfFeatureIds[11];
    //separating different types of features
    if (featuresNotSorted) {
        for (unsigned currFeatureType = 0; currFeatureType < 11; currFeatureType++)
            arrayOfFeatureIds[currFeatureType].clear();
        unsigned numOfFeatures = getNumberOfFeatures();
        for (unsigned currFeatureId = 0; currFeatureId < numOfFeatures; currFeatureId++) {
            FeatureType featureType = getFeatureType(currFeatureId);
            if (featureType == Lake) arrayOfFeatureIds[0].push_back(currFeatureId);
            if (featureType == Island) arrayOfFeatureIds[1].push_back(currFeatureId);
            if (featureType == Beach) arrayOfFeatureIds[2].push_back(currFeatureId);
            if (featureType == Unknown) arrayOfFeatureIds[3].push_back(currFeatureId);
            if (featureType == Golfcourse) arrayOfFeatureIds[4].push_back(currFeatureId);
            if (featureType == Park) arrayOfFeatureIds[5].push_back(currFeatureId);
            if (featureType == Shoreline) arrayOfFeatureIds[6].push_back(currFeatureId);
            if (featureType == Building) arrayOfFeatureIds[7].push_back(currFeatureId);
            if (featureType == Greenspace) arrayOfFeatureIds[8].push_back(currFeatureId);
            if (featureType == River) arrayOfFeatureIds[9].push_back(currFeatureId);
            if (featureType == Stream) arrayOfFeatureIds[10].push_back(currFeatureId);
        }
        featuresNotSorted = false;
    }
    t_bound_box currCoords = get_visible_world();
    bool drawBuildings = false;
    if (currCoords.area() / getInitialCoordinates().area() < 0.01)
        drawBuildings = true;
    for (unsigned currFeatureType = 0; currFeatureType < 11; currFeatureType++) { //0-10 feature types
        if (currFeatureType != 7 || drawBuildings)
            drawFeature(colour[currFeatureType], arrayOfFeatureIds[currFeatureType]);
    }
}

//Draws all POIs on the map and their names

void drawPOIs() {
    unsigned numOfPOIs = getNumberOfPointsOfInterest();
    t_bound_box currCoords = get_visible_world();
    bool ifPrintText = false;
    double iconFactor = 0;

    if (currCoords.area() / getInitialCoordinates().area() < 0.00002) {
        ifPrintText = true;
        iconFactor = 0.01;
    } else if (currCoords.area() / getInitialCoordinates().area() < 0.0002)
        iconFactor = 0.01;
    else if (currCoords.area() / getInitialCoordinates().area() < 0.015)
        iconFactor = 0.005;

    std::vector<std::string> poiTypes;
    for (unsigned i = 0; i < numOfPOIs; i++) {

        float x = getPointOfInterestPosition(i).lon;
        float y = getPointOfInterestPosition(i).lat;

        //For certain POIs will write a letter in the circle
        if (getPointOfInterestType(i) == "restaurant") {
            setcolor(PURPLE);
            fillarc(x, y, iconFactor * currCoords.get_width(), 0, 360);
            setcolor(WHITE);
            drawtext(x, y, "R", 150, FLT_MAX);
        } else if (getPointOfInterestType(i) == "fast_food") {
            setcolor(RED);
            fillarc(x, y, iconFactor * currCoords.get_width(), 0, 360);
            setcolor(WHITE);
            drawtext(x, y, "F", 150, FLT_MAX);
        } else if (getPointOfInterestType(i) == "bank") {
            setcolor(GREEN);
            fillarc(x, y, iconFactor * currCoords.get_width(), 0, 360);
            setcolor(WHITE);
            drawtext(x, y, "$", 150, FLT_MAX);
        } else if (getPointOfInterestType(i) == "bicycle_rental") {
            setcolor(BLACK);
            fillarc(x, y, iconFactor * currCoords.get_width(), 0, 360);
            setcolor(WHITE);
            drawtext(x, y, "B", 150, FLT_MAX);
        } else if (getPointOfInterestType(i) == "fuel") {
            setcolor(BLUE);
            fillarc(x, y, iconFactor * currCoords.get_width(), 0, 360);
            setcolor(WHITE);
            drawtext(x, y, "G", 150, FLT_MAX);
        } else if (getPointOfInterestType(i) == "cafe") {
            setcolor(DARKGREY);
            fillarc(x, y, iconFactor * currCoords.get_width(), 0, 360);
            setcolor(WHITE);
            drawtext(x, y, "C", 150, FLT_MAX);
        } else {
            setcolor(TURQUOISE);
            fillarc(x, y, iconFactor * currCoords.get_width(), 0, 360);
        }

        //Prints names if zoomed in enough
        if (ifPrintText) {
            setcolor(DARKGREY);
            settextrotation(0);
            drawtext(x, y - 0.015 * currCoords.get_width(), getPointOfInterestName(i), 150, FLT_MAX);
        }
    }

}

//Draws all intersections on the map

void drawIntersections() {
    if (drawPOI) {
        unsigned numOfIntersections = getNumberOfIntersections();
        for (unsigned currIntersectionId = 0; currIntersectionId < numOfIntersections; currIntersectionId++) {
            float x = getIntersectionPosition(currIntersectionId).lon;
            float y = getIntersectionPosition(currIntersectionId).lat;
            setcolor(PLUM);
            setlinestyle(SOLID);
            drawarc(x, y, 0.00004, 0, 360);
            setcolor(PURPLE);
        }
    }
    t_bound_box bounds = get_visible_world();
    float width = bounds.get_width();
    if (highlightedIntersection != NULL) {
        setcolor(PURPLE);
        fillarc(highlightedIntersection->x, highlightedIntersection->y, width * 0.01, 0, 360);
    }
}

//Creates white box at bottom of screen to display information

void drawDisplayInfo() {
    t_bound_box bounds = get_visible_world();
    t_bound_box displayBoxbBounds(bounds.left(), bounds.bottom(), bounds.right(), bounds.bottom()+(bounds.top() - bounds.bottom()) / 10);
    t_point *polyPoints = makeRectangle(bounds.bottom()+(bounds.top() - bounds.bottom()) / 10, bounds.right(), bounds.bottom(), bounds.left());
    setcolor(WHITE);
    fillpoly(polyPoints, 4);

    settextrotation(0);

    setfontsize(20);
    setcolor(DARKGREY);
    if (typing)
        drawtext(displayBoxbBounds.get_xcenter(), displayBoxbBounds.get_ycenter(),
            typingString[currentString], 150, FLT_MAX);
    else
        drawtext(displayBoxbBounds.get_xcenter(), displayBoxbBounds.get_ycenter(),
            textForDisplay, 150, FLT_MAX);


}

//Draws the screen for the city map

void drawScreen() {
    set_draw_mode(DRAW_NORMAL); // Should set this if your program does any XOR drawing in callbacks.
    clearscreen(); /* Should precede drawing for all drawscreens */
    if (selectGPS) {
        int typeColour = WHITE, selectColour = WHITE;
        if (hoveringOverChoice == "type")
            typeColour = ORANGE;
        else if (hoveringOverChoice == "press")
            selectColour = ORANGE;
        createTextBox(0, 0.40, 1, "Type start and end locations", "none", typeColour);
        createTextBox(0.40, 0.40, 1, "Click points on the map", "bottom", selectColour);
        createTextBox(0.80, 0.20, 1, "How would you like to select your points", "bottom");
    } else if (typing) {
        std::string special = "";
        autocompleteOptions = autoCompletion(*(gpsInputType[currentString]),
                typingString[currentString]);
        createTextBox(0.50, 0.20, 1, typingString[currentString], "bottom");
        if (currentString > 0)
            special = "back";
        else
            special = "none";
        createTextBox(0.70, 0.30, 1, instructionsGPS[currentString], "bottom", WHITE, special);
        for (unsigned int i = 0; i < autocompleteOptions.size(); i++) {
            if (i == hoverAutocomplete)
                createTextBox(0.40 - 0.10 * i, 0.10, 1, autocompleteOptions[i], "bottom", ORANGE);
            else
                createTextBox(0.40 - 0.10 * i, 0.10, 1, autocompleteOptions[i], "bottom");
        }
    } else {
        //If night mode is activated create a navy polygon to fill the background
        if (nightMode) {
            t_bound_box bounds = getInitialCoordinates();
            t_point *polyPoints = makeRectangle(bounds.top(), bounds.right(), bounds.bottom(), bounds.left());
            setcolor(DARKSLATEBLUE);
            fillpoly(polyPoints, 4);
        }

        t_bound_box currCoords = get_visible_world();
        drawAllFeatures();
        drawAllStreets();
        if (currCoords.area() / getInitialCoordinates().area() < 0.009) drawPOI = true;
        else drawPOI = false;
        if (drawPOI) {
            //only show POIs if showPOIs is on
            if (showPOIs)
                drawPOIs();
        }
        drawIntersections();
        if (drawingPath)
            drawPath(path);
        textForDisplay = ("Hovering over: ");
        drawDisplayInfo();
        if (selectIntersection) {
            drawSelectIntersectionInfo();
        }
    }
    if (help) {
        createTextBox(0.15, 0.65, 0.90, "", "all");
        createTextBox(0.80, 0.10, 0.60, helpMessage[0], "all", WHITE, "black");
        for (unsigned i = 1; i < helpLines; i++)
            createTextBox(0.70 - i * .10, 0.10, 0.60, helpMessage[i], "none", WHITE, "help");
    }
    if (error)
        createTextBox(0.40, 0.20, 0.80, errorMessage, "all", ORANGE, "error");
}

//Draws the screen for selecting a city

void drawSelectScreen() {
    set_draw_mode(DRAW_NORMAL);
    clearscreen();
    //Array to hold selection names and the corresponding url segment
    std::string names[] = {"Saint Helena", "Cairo", "Moscow", "New York", "Hamilton", "Toronto"},
    active[] = {"saint_helena", "cairo_egypt", "moscow", "newyork", "hamilton_canada", "toronto"};
    //Array to hold the colour of each box
    t_color colour[] = {LAKE, DARKKHAKI, CORAL, LIGHTGREY, GOLFCOURSE, PARK};

    setfontsize(20);
    t_point *polyPoints;

    for (int currCityId = 0; currCityId < 6; currCityId++) {
        //If the box has been selected
        if (selectedCity == active[currCityId]) {
            //Draw surrounding black box
            setcolor(BLACK);
            polyPoints = makeRectangle(0 + currCityId * 10, 60, 10 + currCityId * 10, 0);
            fillpoly(polyPoints, 4);
            //Draw inside box with text
            setcolor(colour[currCityId]);
            polyPoints = makeRectangle(1 + currCityId * 10, 58, 9 + currCityId * 10, 2);
            fillpoly(polyPoints, 4);
            setcolor(BLACK);
            drawtext(30, 10 * currCityId + 5, names[currCityId], 150, FLT_MAX);
        } else {
            //Otherwise draw one large box with text
            setcolor(colour[currCityId]);
            polyPoints = makeRectangle(0 + currCityId * 10, 60, 10 + currCityId * 10, 0);
            fillpoly(polyPoints, 4);
            setcolor(BLACK);
            drawtext(30, 10 * currCityId + 5, names[currCityId], 150, FLT_MAX);
        }
    }
}

//Function called with using find

void findButtonFunc(void (*drawscreen_ptr) (void)) {
    std::string type;
    std::cout << "What feature would you like to find?" << std::endl
            << "(street, intersection, point)" << std::endl;
    std::getline(std::cin, type);

    if (type == "intersection") {
        std::string street1, street2;
        std::cout << "Please enter the names of the first street which intersects."
                << std::endl;
        std::getline(std::cin, street1);
        std::cout << "Please enter the second street" << std::endl;
        std::getline(std::cin, street2);
        auto intersectionIDs = find_intersection_ids_from_street_names(street1, street2);
        if (intersectionIDs.size() == 0) {
            std::cout << "No intersections found." << std::endl;
            return;
        }


        float x = getIntersectionPosition(intersectionIDs[0]).lon;
        float y = getIntersectionPosition(intersectionIDs[0]).lat;

        //set graphics to highlight specific POI    
        if (highlightedIntersection != NULL) delete highlightedIntersection;
        highlightedIntersection = new t_point(x, y);
        update_message("Selected Intersection: " + getIntersectionName(intersectionIDs[0]));
        t_bound_box newScreen(x - 0.001, y - 0.001, x + 0.001, y + 0.001);
        set_visible_world(newScreen);
        if (intersectionIDs.size() > 1) {
            std::cout << "Other intersection(s) at: " << std::endl;
            for (auto iter = intersectionIDs.begin() + 1;
                    iter != intersectionIDs.end(); iter++) {
                LatLon point = getIntersectionPosition(*iter);
                std::cout << "(" << point.lon << ", " << point.lat << ")"
                        << std::endl;
            };

        }

    } else if (type == "street") {
        std::string street;
        std::cout << "Please enter the name of the street." << std::endl;
        std::getline(std::cin, street);
        auto streetIDs = find_street_ids_from_name(street);
        if (streetIDs.size() == 0) {
            std::cout << "No streets found." << std::endl;
            return;
        }
        auto streetSegments = find_street_street_segments(streetIDs[0]);
        unsigned selectedSegmentID = streetSegments[(streetSegments.size() / 2)];
        StreetSegmentInfo selectedSegment = getStreetSegmentInfo(selectedSegmentID);
        unsigned intersectionID = selectedSegment.from;
        float x = getIntersectionPosition(intersectionID).lon;
        float y = getIntersectionPosition(intersectionID).lat;
        t_bound_box newScreen(x - 0.001, y - 0.001, x + 0.001, y + 0.001);
        set_visible_world(newScreen);
        if (streetIDs.size() > 1) {
            std::cout << "Other street(s) at: " << std::endl;
            for (auto iter = streetIDs.begin() + 1;
                    iter != streetIDs.end(); iter++) {
                streetSegments = find_street_street_segments(*iter);
                selectedSegmentID = streetSegments[(streetSegments.size() / 2)];
                selectedSegment = getStreetSegmentInfo(selectedSegmentID);
                intersectionID = selectedSegment.from;
                LatLon point = getIntersectionPosition(intersectionID);
                std::cout << "(" << point.lon << ", " << point.lat << ")"
                        << std::endl;
            }
        }

    } else if (type == "point") {
        double latitude, longitude;
        std::cout << "Please enter the latitude." << std::endl;
        std::cin >> latitude;
        std::cout << "Please enter the longitude." << std::endl;
        std::cin >> longitude;
        t_bound_box newScreen(longitude - 0.001, latitude - 0.001, longitude + 0.001, latitude + 0.001);
        set_visible_world(newScreen);

    } else {
        std::cout << "Input invalid." << std::endl;
    }
}

//Function called when toggling night mode

void nightModeFunc(void (*drawscreen_ptr) (void)) {
    if (!nightMode) {
        change_button_text("Night On", "Night Off");
        nightMode = true;
    } else {

        change_button_text("Night Off", "Night On");
        nightMode = false;
    }
    drawScreen();
}

//Function called when toggling POIs

void togglePOIFunc(void (*drawscreen_ptr) (void)) {
    if (!showPOIs) {
        change_button_text("Show POIs", "Hide POIs");
        showPOIs = true;
    } else {

        change_button_text("Hide POIs", "Show POIs");
        showPOIs = false;
    }
    drawScreen();
}

//Called whenever the mouse is clicked on the city map

void mousePressCityMap(float x, float y, t_event_buttonPressed event) {
    if (error) {
        if (hoverAutocomplete == 12) {
            error = false;
            errorMessage = "Error: There should be no error";
        }
    } else if (selectGPS) {
        if (hoveringOverChoice == "press") {
            selectGPS = false;
            selectIntersection = true;
            currentIntersection = 0;
            update_message("Map of " + selectedCity + ".");
            highlightedIntersection = NULL;
            drawScreen();
        } else if (hoveringOverChoice == "type") {
            selectGPS = false;
            set_keypress_input(true);
            typing = true;
            currentString = 0;
            currentStringMax = 3;
            textUI = instructionsGPS;
            cursor = 0;
            for (unsigned int i = 0; i < MaxStrings; i++)
                typingString[i] = "";
            drawScreen();
        }
    } else if (typing) {
        if (hoverAutocomplete < autocompleteOptions.size()) {
            typingString[currentString] = autocompleteOptions[hoverAutocomplete];
            if (*(gpsInputType[currentString]) == "intersection")
                savedIntersectionIds[currentString] =
                    (intersectionIdFromLowerCaseName(typingString[currentString]))[0];
            currentString++;
            cursor = 0;
            if (currentString >= currentStringMax) {
                doneTyping();
            }
            drawScreen();
            //For back button
        } else if (hoverAutocomplete == 11) {
            typingString[currentString] = "";
            currentString--;
            typingString[currentString] = "";
            drawScreen();
        }
    } else {
        t_bound_box bounds = get_visible_world();
        if (selectIntersection && currentIntersection > 0 &&
                x > bounds.get_xcenter() - (bounds.left() - bounds.right())*0.42 &&
                y > bounds.get_ycenter() + (bounds.top() - bounds.bottom())*0.42) {
            if (currentIntersection == 1)
                currentIntersection--;
            else {
                savedIntersection[0] = savedIntersection[1];
                currentIntersection--;
            }
            drawScreen();
        }
        if (selectIntersection && currentIntersection > 1 &&
                x > bounds.get_xcenter() - (bounds.left() - bounds.right())*0.42 &&
                y > bounds.get_ycenter() + (bounds.top() - bounds.bottom())*0.32) {
            currentIntersection--;
            drawScreen();
        }
        if (selectIntersection && currentIntersection == MaxIntersections &&
                x > bounds.get_xcenter() + (bounds.left() - bounds.right())*0.125 &&
                x < bounds.get_xcenter() - (bounds.left() - bounds.right())*0.125 &&
                y > bounds.get_ycenter() + (bounds.top() - bounds.bottom())*0.20 &&
                y < bounds.get_ycenter() + (bounds.top() - bounds.bottom())*0.30) {
            doneSelectIntersection();
        }
        //if the user clicks on an intersection, print out the intersection name
        //and highlight the intersection
        LatLon mousePressedPosition(y, x); //y lat; x lon
        // get the intersection closet to the mouse position 
        unsigned closestIntersectionId = find_closest_intersection(mousePressedPosition);
        // get the distance
        double distance = find_distance_between_two_points(mousePressedPosition, getIntersectionPosition(closestIntersectionId));
        if (distance < 3) {
            if (selectIntersection && currentIntersection < MaxIntersections) {
                savedIntersection[currentIntersection] = closestIntersectionId;
                currentIntersection++;
            }
            update_message("Selected Intersection: " + getIntersectionName(closestIntersectionId));
            if (highlightedIntersection != NULL)
                delete highlightedIntersection;
            highlightedIntersection = new t_point(getIntersectionPosition(closestIntersectionId).lon, getIntersectionPosition(closestIntersectionId).lat);
        } else {
            update_message("Map of " + selectedCity + ".");
            highlightedIntersection = NULL;
        }
        float yCenter = bounds.get_ycenter(),
                height = bounds.get_height(),
                xCenter = bounds.get_xcenter(),
                width = bounds.get_width();
        if (x < xCenter - 0.40 * width &&
                y > yCenter - 0.4 * height && y < yCenter - 0.3 * height) { //going back
            if (routeNum > 0) {
                routeNum--;
                if (routeNum == 0)
                    routeForDisplay = "BEGIN";
                else {
                    routeForDisplay = getRouteForDisplay();
                }
            }
        } else if (x > xCenter + 0.40 * width &&
                y > yCenter - 0.4 * height && y < yCenter - 0.3 * height) { //going forward
            if (routeNum <= streetsOfRoute.size()) {
                routeNum++;
                if (routeNum > streetsOfRoute.size()) {
                    routeForDisplay = "You have reached your destination. :)";
                } else {
                    routeForDisplay = getRouteForDisplay();
                }
            }
        }
        drawScreen();
    }
}

//Called whenever mouse is clicked when selecting a city

void mousePressSelectScreen(float x, float y, t_event_buttonPressed event) {
    //Checks bounds to see which city is pressed and assigns a string
    if (x >= 0 && x <= 60) {
        if (y >= 0 && y <= 10) {
            selectedCity = "saint_helena";
        } else if (y >= 10 && y <= 20) {
            selectedCity = "cairo_egypt";
        } else if (y >= 20 && y <= 30) {
            selectedCity = "moscow";
        } else if (y >= 30 && y <= 40) {
            selectedCity = "newyork";
        } else if (y >= 40 && y <= 50) {
            selectedCity = "hamilton_canada";
        } else if (y >= 50 && y <= 60) {
            selectedCity = "toronto";
        }
        //Updates message at the bottom of screen
        update_message(selectedCity + " selected.");
    }
    drawSelectScreen();
}

//This function selects whether the user is hovering over a street and returns 
// a vector contains its streetID

std::vector<unsigned> chooseHighlightedStreet(float x, float y) {


    std::vector<unsigned> nearbyIntersections = find_nearby_intersections(LatLon(y, x), 1000);
    std::vector<unsigned> nearbyStreetSegs;

    //find all nearby intersections and returns all street segments connected to them
    for (std::vector<unsigned>::iterator currentIntersection = nearbyIntersections.begin(); currentIntersection != nearbyIntersections.end(); currentIntersection++) {
        std::vector<unsigned> streetSegsAtIntersection = find_intersection_street_segments(*currentIntersection);
        for (std::vector<unsigned>::iterator currentStreetSeg = streetSegsAtIntersection.begin(); currentStreetSeg != streetSegsAtIntersection.end(); currentStreetSeg++)
            nearbyStreetSegs.push_back(*currentStreetSeg);
    }

    unsigned closestStreetID = 0;
    double closestDistance = 1000000;
    double currentDistance;

    //finds distance from every straight section of every segment to find closest
    // street segment curve section
    for (unsigned i = 0; i < nearbyStreetSegs.size(); i++) {
        StreetSegmentInfo streetSegment = getStreetSegmentInfo(nearbyStreetSegs[i]);
        LatLon startCoords = getIntersectionPosition(streetSegment.from);
        LatLon endCoords = getIntersectionPosition(streetSegment.to);
        std::vector<LatLon> allCoords;
        //add start coordinate to allCoors
        allCoords.push_back(startCoords);

        //add all curve points to allCoors
        for (unsigned currentCurvePoint = 0; currentCurvePoint < streetSegment.curvePointCount;
                currentCurvePoint++)
            allCoords.push_back(getStreetSegmentCurvePoint(nearbyStreetSegs[i], currentCurvePoint));

        //add end coordinate to allCoors
        allCoords.push_back(endCoords);

        //iterates through every point in the vector and adds the distance to the next one
        for (auto currentCurvePoint = allCoords.begin(), nextCurvePoint = ++allCoords.begin();
                nextCurvePoint != allCoords.end();
                ++currentCurvePoint, ++nextCurvePoint) {
            currentDistance = find_distance_between_point_and_line(*currentCurvePoint,
                    *nextCurvePoint, LatLon(y, x));

            if (currentDistance < closestDistance) {
                closestStreetID = getStreetSegmentInfo(nearbyStreetSegs[i]).streetID;
                closestDistance = currentDistance;
            }
        }
    }

    t_bound_box currCoords = get_visible_world();
    float proximityRequired;
    if (currCoords.area() / getInitialCoordinates().area() < 0.005)
        proximityRequired = 5;
    else if (currCoords.area() / getInitialCoordinates().area() < 0.05)
        proximityRequired = 10;
    else
        proximityRequired = 20;

    //returns the closest street if any part of it is closer than 3 units away
    if (closestDistance < proximityRequired && closestStreetID != 0)
        return {
        closestStreetID
    };
    else
        return {
    };
}

//Called when mouse moves in city map

void mouseMoveCityMap(float x, float y) {
    if (error) {
        t_bound_box bounds = get_visible_world();
        unsigned int previousHover = hoverAutocomplete;
        float yCenter = bounds.get_ycenter(),
                height = bounds.get_height(),
                xCenter = bounds.get_xcenter(),
                width = bounds.get_width();
        if (x > xCenter - 0.08 * width && x < xCenter + 0.08 * width &&
                y > yCenter - 0.08 * height && y < yCenter - 0.04 * height)
            // 0.4 + 0.2 * 0.10, 0.2 * 0.20,
            //   0.8 * 0.20)
            hoverAutocomplete = 12;
        else
            hoverAutocomplete = 10;
        if (previousHover != hoverAutocomplete)
            drawScreen();
    } else if (selectGPS) {
        t_bound_box bounds = get_visible_world();
        std::string previousHover = hoveringOverChoice;
        float yCenter = bounds.get_ycenter(),
                height = bounds.get_height();
        if ((y < yCenter - height * .09 && y > yCenter - height * .11) ||
                (y > yCenter + height * .29 && y < yCenter + height * .31)) {
            //Avoid flickering by adding a space buffer
        } else if (y < yCenter + height * .30 && y > yCenter - height * .10) {
            hoveringOverChoice = "press";
        } else if (y < yCenter - height * .10 && y > yCenter - height * .50) {
            hoveringOverChoice = "type";
        } else {
            hoveringOverChoice = "";
        }
        if (previousHover != hoveringOverChoice)
            drawScreen();
    } else if (typing) {
        t_bound_box bounds = get_visible_world();
        unsigned int previousHover = hoverAutocomplete;
        float yCenter = bounds.get_ycenter(),
                height = bounds.get_height();
        if (y < yCenter)
            hoverAutocomplete = (int) (((yCenter - y) / height)*10);
        else if (x > bounds.get_xcenter() - (bounds.left() - bounds.right())*0.42 &&
                y > bounds.get_ycenter() + (bounds.top() - bounds.bottom())*0.42 &&
                currentString > 0)
            hoverAutocomplete = 11;
        else
            hoverAutocomplete = 10;
        if (previousHover != hoverAutocomplete)
            drawScreen();
    } else {
        t_bound_box bounds = get_visible_world();
        float yCenter = bounds.get_ycenter(),
                height = bounds.get_height(),
                xCenter = bounds.get_xcenter(),
                width = bounds.get_width();
        unsigned int previousHover = hoverAutocomplete;
        if (x < xCenter - 0.40 * width &&
                y > yCenter - 0.4 * height && y < yCenter - 0.3 * height) {
            hoverAutocomplete = 13;
        } else if (x > xCenter + 0.40 * width &&
                y > yCenter - 0.4 * height && y < yCenter - 0.3 * height) {
            hoverAutocomplete = 14;
        } else {
            hoverAutocomplete = 10;
            LatLon mouseHoveredPosition(y, x); //y lat; x lon
            // get the intersection closet to the mouse position 
            unsigned closestIntersectionId = find_closest_intersection(mouseHoveredPosition);
            // get the distance
            double distance = find_distance_between_two_points(mouseHoveredPosition, getIntersectionPosition(closestIntersectionId));

            //if intersection is getting hovered
            if (distance < 8) {
                textForDisplay = ("Hovering over: " + getIntersectionName(closestIntersectionId));
                unhighlightedStreets = highlightedStreets;
                highlightedStreets = find_intersection_street_ids(closestIntersectionId);
                highlightedStreets = copyVectorWithoutRepeats(highlightedStreets);
            }//if street is getting hovered or nothing is getting hovered
            else {
                unhighlightedStreets = highlightedStreets;
                highlightedStreets.clear();
                highlightedStreets = chooseHighlightedStreet(x, y);
                if (highlightedStreets.size() > 0) {
                    textForDisplay = ("Hovering over: " + getStreetName(highlightedStreets[0]));
                } else {

                    textForDisplay = ("Hovering over: ");
                }
            }
            drawHighlightedStreets(false, unhighlightedStreets);
            drawHighlightedStreets(true, highlightedStreets);
        }
        if (previousHover != hoverAutocomplete)
            drawScreen();
    }
}


//Creates a t_point array with four points to create a rectangle with the given
//dimensions

t_point * makeRectangle(float top, float right, float bottom, float left) {
    t_point one, two, three, four;
    one.x = left;
    one.y = bottom;
    two.x = right;
    two.y = bottom;
    three.x = right;
    three.y = top;
    four.x = left;
    four.y = top;
    t_point rectPoints[] = {one, two, three, four};
    return rectPoints;
}

//Called when a keyboard key is pressed

void keyPressCityMap(char c, int keysym) {
    //Check if special character is used
#ifdef X11
    switch (keysym) {
        case XK_Shift_L:
        case XK_Shift_R:
            if (singleCap)
                singleCap = false;
            else
                singleCap = true;
            break;
        case XK_Caps_Lock:
            if (capLock)
                capLock = false;
            else
                capLock = true;
            singleCap = false;
            break;
        case XK_Left:
            if (cursor > 0)
                cursor--;
            break;
        case XK_Right:
            if (cursor < typingString[currentString].size())
                cursor++;
            break;
    }
#endif

    if (c == '\r') {
        //Go to next string
        std::vector<unsigned int> intersectionID = intersectionIdFromLowerCaseName(typingString[currentString]);
        if (intersectionID.size() && *(gpsInputType[currentString]) == "intersection") {
            savedIntersectionIds[currentString] = intersectionID[0];
            currentString++;
        } else if (*(gpsInputType[currentString]) == "point of interest") {
            currentString++;
        } else if (*(gpsInputType[currentString]) == "options" &&
                (typingString[currentString] == "intersection" ||
                typingString[currentString] == "point of interest")) {
            currentString++;
        } else {
            typingString[currentString] = "";
            error = true;
            if (*(gpsInputType[currentString]) == "intersection")
                errorMessage = "Error: Intersection not found";
            else if (*(gpsInputType[currentString]) == "point of interest")
                errorMessage = "Error: Point of interest not found";
            else
                errorMessage = "Error: Invalid input";
        }
        cursor = 0;
    } else if (c == '\t') {
        //auto complete
        if (autocompleteOptions.size()) {
            typingString[currentString] = autocompleteOptions[0];
            cursor = autocompleteOptions[0].size();
            //currentString++;
        }//Else error message?
    } else if (c) {
        //If backspace check to make sure string is not empty
        if (c == '\b') {
            if (typingString[currentString].length()) {
                cursor -= 1;
                typingString[currentString].erase(typingString[currentString].end() - 1);
            }
            //Otherwise add character to string
        } else {
            if ((singleCap || capLock) && (c >= 'a' && c <= 'z'))
                c -= 32;
            //            typingString[currentString] += c;
            typingString[currentString].insert(cursor, 1, c);
            cursor++;
        }
        singleCap = false;
    }
    //Check to see if all words have been input
    if (currentString >= currentStringMax) {
        doneTyping();
    }
    drawScreen();
}

//Function called when type button is pressed

void gpsButtonFunc(void (*drawscreen_ptr) (void)) {
    if (selectGPS || typing || selectIntersection) {
        selectGPS = false;
    } else {
        selectGPS = true;
        hoveringOverChoice = "";
    }
    typing = false;
    selectIntersection = false;
    drawingPath = false;
    streetsOfRoute.clear();
    routeNum = 0;
    routeForDisplay = "BEGIN";
    drawScreen();
}

//Used to create text boxes

void createTextBox(double positionPercent, double percentHeight, double percentWidth,
        std::string text, std::string dividingLine, int boxColour, std::string special) {
    //Special features to the box
    bool includeX = false, intersectionHeading = false, back = false,
            errorPop = false, small = false, black = false, directionsBox = false;
    if (special == "includeX")
        includeX = true;
    else if (special == "selectIntersectionHeading") {
        intersectionHeading = true;
        includeX = true;
    } else if (special == "back")
        back = true;
    else if (special == "error")
        errorPop = true;
    else if (special == "small")
        small = true;
    else if (special == "black")
        black = true;
    else if (special == "help") {
        black = true;
        small = true;
    } else if (special == "directions") {
        directionsBox = true;
    }

    //Creates a box with the correct position, colour, and size
    t_bound_box bounds = get_visible_world();
    t_bound_box displayBoxbBounds(bounds.left() + (bounds.right() - bounds.left())*(1 - percentWidth)*0.5,
            bounds.bottom() + (bounds.top() - bounds.bottom()) * positionPercent,
            bounds.right() - (bounds.right() - bounds.left())*(1 - percentWidth)*0.5,
            bounds.bottom() + (bounds.top() - bounds.bottom()) * positionPercent +
            (bounds.top() - bounds.bottom()) * percentHeight);
    t_point *polyPoints = makeRectangle(bounds.bottom() + (bounds.top() - bounds.bottom()) *
            positionPercent + (bounds.top() - bounds.bottom()) * percentHeight,
            bounds.right() - (bounds.right() - bounds.left())*(1 - percentWidth)*0.5,
            bounds.bottom() + (bounds.top() - bounds.bottom()) * positionPercent,
            bounds.left() + (bounds.right() - bounds.left())*(1 - percentWidth)*0.5);
    setcolor(boxColour);
    fillpoly(polyPoints, 4);

    //Border on the top or bottom of the box
    setcolor(BLACK);
    setlinestyle(SOLID);
    setlinewidth(4);
    if (dividingLine == "top" || dividingLine == "all") {
        drawline(bounds.left() + (bounds.right() - bounds.left())*(1 - percentWidth)*0.5,
                bounds.bottom() + (bounds.top() - bounds.bottom()) * positionPercent +
                (bounds.top() - bounds.bottom()) * percentHeight,
                bounds.right() - (bounds.right() - bounds.left())*(1 - percentWidth)*0.5,
                bounds.bottom() + (bounds.top() - bounds.bottom()) * positionPercent +
                (bounds.top() - bounds.bottom()) * percentHeight);
    }
    if (dividingLine == "bottom" || dividingLine == "all") {
        drawline(bounds.left() + (bounds.right() - bounds.left())*(1 - percentWidth)*0.5,
                bounds.bottom() + (bounds.top() - bounds.bottom()) * positionPercent,
                bounds.right() - (bounds.right() - bounds.left())*(1 - percentWidth)*0.5,
                bounds.bottom() + (bounds.top() - bounds.bottom()) * positionPercent);
    }
    if (dividingLine == "left" || dividingLine == "all") {
        drawline(bounds.left() + (bounds.right() - bounds.left())*(1 - percentWidth)*0.5,
                bounds.bottom() + (bounds.top() - bounds.bottom()) * positionPercent,
                bounds.left() + (bounds.right() - bounds.left())*(1 - percentWidth)*0.5,
                bounds.bottom() + (bounds.top() - bounds.bottom()) * positionPercent +
                (bounds.top() - bounds.bottom()) * percentHeight);
    }
    if (dividingLine == "right" || dividingLine == "all") {
        drawline(bounds.right() - (bounds.right() - bounds.left())*(1 - percentWidth)*0.5,
                bounds.bottom() + (bounds.top() - bounds.bottom()) * positionPercent,
                bounds.right() - (bounds.right() - bounds.left())*(1 - percentWidth)*0.5,
                bounds.bottom() + (bounds.top() - bounds.bottom()) * positionPercent +
                (bounds.top() - bounds.bottom()) * percentHeight);
    }

    //Write the text
    settextrotation(0);
    if (small)
        setfontsize(15);
    else
        setfontsize(20);
    if (black)
        setcolor(BLACK);
    else
        setcolor(DARKGREY);
    drawtext(displayBoxbBounds.get_xcenter(), displayBoxbBounds.get_ycenter(),
            text, 150, FLT_MAX);

    if (includeX) {
        setcolor(RED);
        drawtext(displayBoxbBounds.get_xcenter() - (bounds.left() - bounds.right())*0.45,
                displayBoxbBounds.get_ycenter(), "X", 150, FLT_MAX);
    }
    if (intersectionHeading) {
        setcolor(BLACK);
        setfontsize(10);
        drawtext(displayBoxbBounds.get_xcenter() + (bounds.left() - bounds.right())*0.45,
                displayBoxbBounds.get_ycenter() + (bounds.top() - bounds.bottom()) * percentHeight * 0.2,
                "Directions", 150, FLT_MAX);
    }
    if (back) {
        if (hoverAutocomplete == 11)
            setcolor(ORANGE);
        else
            setcolor(BLACK);
        drawtext(displayBoxbBounds.get_xcenter() - (bounds.left() - bounds.right())*0.45,
                displayBoxbBounds.get_ycenter() + (bounds.top() - bounds.bottom()) * percentHeight * 0.35,
                "<-", 150, FLT_MAX);
    }
    if (errorPop) {
        int buttonColour;
        if (hoverAutocomplete == 12)
            buttonColour = YELLOW;
        else
            buttonColour = WHITE;
        createTextBox(positionPercent + percentHeight * 0.10, percentHeight * 0.20,
                percentWidth * 0.20, "Dismiss", "all", buttonColour);
    }
    if (directionsBox) {
        if (hoverAutocomplete == 14)
            setcolor(ORANGE);
        else
            setcolor(BLACK);
        drawtext(displayBoxbBounds.get_xcenter() - (bounds.left() - bounds.right())*0.45,
                displayBoxbBounds.get_ycenter(),
                "->", 150, FLT_MAX);
        if (hoverAutocomplete == 13)
            setcolor(ORANGE);
        else
            setcolor(BLACK);
        drawtext(displayBoxbBounds.get_xcenter() + (bounds.left() - bounds.right())*0.45,
                displayBoxbBounds.get_ycenter(),
                "<-", 150, FLT_MAX);
    }
}

//Feedback for user telling them which intersections have been selected for pathfinding

void drawSelectIntersectionInfo() {
    std::string text, special;
    for (unsigned int i = 0; i < currentIntersection; i++) {
        if (i == 0) {
            text = "From: " + getIntersectionName(savedIntersection[i]);
            special = "selectIntersectionHeading";
        } else if (i == 1) {
            text = "To: " + getIntersectionName(savedIntersection[i]);
            special = "includeX";
        }
        createTextBox(0.90 - 0.10 * i, 0.10, 1, text, "bottom", WHITE, special);
    }
    if (currentIntersection == MaxIntersections)
        createTextBox(0.70, 0.10, 0.25, "Find Path", "all", GREEN);
}

void centerScreen() {
    if (!path.empty()) {
        float left = (getIntersectionPosition(getStreetSegmentInfo(*path.begin()).from)).lon;
        float right = (getIntersectionPosition(getStreetSegmentInfo(*path.begin()).from)).lon;
        float top = (getIntersectionPosition(getStreetSegmentInfo(*path.begin()).from)).lat;
        float bottom = (getIntersectionPosition(getStreetSegmentInfo(*path.begin()).from)).lat;
        for (auto currentSeg = path.begin(); currentSeg != path.end(); currentSeg++) {
            auto currentSegInfo = getStreetSegmentInfo(*currentSeg);
            float x1 = (getIntersectionPosition(getStreetSegmentInfo(*currentSeg).from)).lon;
            float y1 = (getIntersectionPosition(getStreetSegmentInfo(*currentSeg).from)).lat;
            float x2 = (getIntersectionPosition(getStreetSegmentInfo(*currentSeg).to)).lon;
            float y2 = (getIntersectionPosition(getStreetSegmentInfo(*currentSeg).to)).lat;
            if (x1 < left) left = x1;
            if (x1 > right) right = x1;
            if (x2 < left) left = x2;
            if (x2 > right) right = x2;
            if (y1 < bottom) bottom = y1;
            if (y1 > top) top = y1;
            if (y2 < bottom) bottom = y2;
            if (y2 > top) top = y2;
            if (std::find(streetsOfRoute.begin(), streetsOfRoute.end(), currentSegInfo.streetID) == streetsOfRoute.end())
                streetsOfRoute.push_back(currentSegInfo.streetID);
        }
        float height = top - bottom;
        float width = right - left;
        t_bound_box newScreen(left - width * 0.1, bottom - height * 0.5, right + width * 0.1, top + height * 0.2);
        set_visible_world(newScreen);
    }
}
//Called when finished typing to directions

void doneTyping() {
    typing = false;
    if (typingString[0] == "intersection") {
        drawingPath = true;
        path = find_path_between_intersections(savedIntersectionIds[1],
                savedIntersectionIds[2]);
        savedIntersection[0] = savedIntersectionIds[1];
        savedIntersection[1] = savedIntersectionIds[2];
    } else if (typingString[0] == "point of interest") {
        drawingPath = true;
        path = find_path_to_point_of_interest(savedIntersectionIds[1],
                getProperPoiName(typingString[2]));
        savedIntersection[0] = savedIntersectionIds[1];
        if (path.size() >= 2) {
            auto secondLastSeg = path.end() - 2;
            auto lastSeg = path.end() - 1;
            unsigned intersectionId1 = getStreetSegmentInfo(*secondLastSeg).from;
            unsigned intersectionId2 = getStreetSegmentInfo(*secondLastSeg).to;
            unsigned intersectionId3 = getStreetSegmentInfo(*lastSeg).from;
            unsigned intersectionId4 = getStreetSegmentInfo(*lastSeg).to;
            if (intersectionId3 == intersectionId1 || intersectionId3 == intersectionId2)
                savedIntersection[1] = intersectionId4;
            else
                savedIntersection[1] = intersectionId3;
        } else if (!path.empty()) {
            auto startIntersection = savedIntersectionIds[1];
            auto lastSeg = path.end() - 1;
            unsigned intersectionId1 = getStreetSegmentInfo(*lastSeg).from;
            unsigned intersectionId2 = getStreetSegmentInfo(*lastSeg).to;
            if (startIntersection == intersectionId1)
                savedIntersection[1] = intersectionId2;
            else
                savedIntersection[1] = intersectionId1;
        }
    }
    //Center Map
    centerScreen();
}

//Called when done selecting intersections

void doneSelectIntersection() {
    selectIntersection = false;
    drawingPath = true;
    path = find_path_between_intersections(savedIntersection[0],
            savedIntersection[1]);
    centerScreen();
}

void helpButtonFunc(void (*drawscreen_ptr) (void)) {
    if (help)
        help = false;
    else
        help = true;
    drawScreen();
}
