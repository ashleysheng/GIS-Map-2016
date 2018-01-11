/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   queueElement.h
 * Author: andreje2
 *
 * Created on March 12, 2016, 2:09 PM
 */

#ifndef QUEUEELEMENT_H
#define QUEUEELEMENT_H

class queueElement {
public:
    queueElement();
    queueElement(double time, unsigned intersectionID);
    virtual ~queueElement();   
    friend bool operator<(const queueElement& LHS, const queueElement& RHS);  
    friend bool operator>(const queueElement& LHS, const queueElement& RHS);
    unsigned getIntersectionID();
    double getTime();
    double time;
    
private:
 
    unsigned intersectionID;
};


#endif /* QUEUEELEMENT_H */

