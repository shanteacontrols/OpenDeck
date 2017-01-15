#!/bin/bash

#read MAJOR file
major=`cat MAJOR`

#read MINOR file
minor=`cat MINOR`

#read REVISION
revision=`cat REVISION`

#output $major, $minor and $revision into separate files
echo "software version: $major.$minor.$revision"
echo "$major,$minor,$revision" > version