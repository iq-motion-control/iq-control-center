
#include "qserial_extended.hpp"
#include "main.h"

//Deconstructor
QSerialExtended::~QSerialExtended(){
    if(qser_port_->isOpen()){
        qser_port_->close();
    }

    delete qser_port_;
}

