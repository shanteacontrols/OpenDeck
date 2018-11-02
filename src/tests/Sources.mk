vpath application/%.cpp ../
vpath modules/%.cpp ../

SOURCES_database := \
database/Database.cpp \
modules/dbms/src/DBMS.cpp \
stubs/database/DB_ReadWrite.cpp \
application/database/Database.cpp

#make sure all objects are located in build directory
#also make sure objects have .o extension

OBJECTS_database := $(addprefix build/,$(SOURCES_database))
OBJECTS_database := $(OBJECTS_database:.c=.o)
OBJECTS_database := $(OBJECTS_database:.cpp=.o)

#common include dirs
INCLUDE_DIRS := \
-I"../application/" \
-I"../modules/"

INCLUDE_FILES += -include "../application/board/avr/variants/$(BOARD_TYPE)/Hardware.h"