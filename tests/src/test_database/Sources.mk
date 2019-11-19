vpath application/%.cpp ../src
vpath common/%.cpp ../src
vpath modules/%.cpp ../
vpath modules/%.c ../

SOURCES_$(shell basename $(dir $(lastword $(MAKEFILE_LIST)))) := \
modules/dbms/src/LESSDB.cpp \
stubs/database/DB_ReadWrite.cpp \
application/database/Database.cpp