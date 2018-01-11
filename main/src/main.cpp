#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "m4.h"

using namespace std;

int main() {
    load_map("/cad2/ece297s/public/maps/toronto.streets.bin");
    std::vector<DeliveryInfo> deliveries;
    std::vector<unsigned> depots;
    std::vector<unsigned> result_path;

    deliveries = {DeliveryInfo(9256, 65463)};
    depots = {2};
  //  result_path = traveling_courier(deliveries, depots);
    string cityName;
    do {
        cityName = selectScreen();
        if (cityName != "close") {
            std::cout << "Loading." << std::endl;
            load_map("/cad2/ece297s/public/maps/" + cityName + ".streets.bin");
            loadOSMDatabaseBIN("/cad2/ece297s/public/maps/" + cityName + ".osm.bin");
            initializeMap();
    result_path = traveling_courier(deliveries, depots);

                close_map();
        }
    }while (cityName != "close");
//    string cityName;
//    do {
//        cityName = selectScreen();
//        if (cityName != "close") {
//            std::cout << "Loading." << std::endl;
//            load_map("/cad2/ece297s/public/maps/" + cityName + ".streets.bin");
//            loadOSMDatabaseBIN("/cad2/ece297s/public/maps/" + cityName + ".osm.bin");
//            initializeMap();
//
//            close_map();
//        }
//
//    } while (cityName != "close");
    close_graphics();
    std::cout << "Goodbye." << std::endl;
    return 0;
}