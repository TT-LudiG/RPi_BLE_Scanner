#ifndef GSMEXCEPTIONS_H
#define GSMEXCEPTIONS_H

#include <exception>

// EXCEPTION_GSM_0

class GSMInitialiseException: public std::exception
{	
public:	
    virtual const char* what() const throw()
    {
        return "EXCEPTION_GSM_0: Failed to initialise the GSM controller.";
    }
};

#endif