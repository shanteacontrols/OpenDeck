#!/bin/bash

#set revision to initial state
revision=0

#read MAJOR file
major_new=`cat MAJOR_NEW`

#read MINOR file
minor_new=`cat MINOR_NEW`

#read saved MAJOR file
major_old=`cat MAJOR_OLD`

#read saved MINOR file
minor_old=`cat MINOR_OLD`

#get the last commit from file
last_commit=`cat LAST_COMMIT`

if [[ ( $minor_new > $minor_old ) || ( $major_new > $major_old ) ]]
then
echo $minor_new > MINOR_OLD
echo $major_new > MAJOR_OLD
echo '0' > REVISION
echo $(git rev-parse HEAD) > LAST_COMMIT
else
revision=$(git rev-list $last_commit..HEAD --count)
fi

echo $revision > REVISION

#output $major, $minor and $revision into separate files
echo "software version: $major_new.$minor_new.$revision"
echo "$major_new,$minor_new,$revision" > version