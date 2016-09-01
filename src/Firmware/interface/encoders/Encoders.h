#ifndef ENCODERS_H_
#define ENCODERS_H_

#include "../../hardware/core/Core.h"

class Encoders  {

    public:
    Encoders();
    void update();

};

extern Encoders encoders;

#endif
