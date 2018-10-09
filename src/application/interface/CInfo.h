#pragma once

#include "dbms/src/DataTypes.h"
#include "sysex/src/DataTypes.h"

typedef bool (*cinfoHandler_t)(dbBlockID_t dbBlock, sysExParameter_t componentID);

///
/// \brief Common handler used to identify currently active component during SysEx configuration.
/// Must be implemented externally.
///
extern cinfoHandler_t cinfoHandler;