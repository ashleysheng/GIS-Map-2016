#include "queueElement.h"
    
    queueElement::queueElement(){
        this->time = 0;
        this->intersectionID = 0;
    };
    
    queueElement::queueElement(double time, unsigned intersectionID){
        this->time = time;
        this->intersectionID = intersectionID;
    };
    
    queueElement::~queueElement() {
    }
   
    
    
    unsigned queueElement::getIntersectionID(){
        return this->intersectionID;
              
    };
    
    double queueElement::getTime(){
        return time;
    };