#!/bin/bash

#read MAJOR file
major=`cat MAJOR`

#read MINOR file
minor=`cat MINOR`

#read REVISION file
revision=`cat REVISION`

git tag v$major.$minor.$revision
git push --tags