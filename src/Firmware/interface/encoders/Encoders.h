#ifndef ENCODERS_H_
#define ENCODERS_H_

#include "../../board/Board.h"

class Encoders  {

    public:
    Encoders();
    void update();

};

extern Encoders encoders;

#endif
