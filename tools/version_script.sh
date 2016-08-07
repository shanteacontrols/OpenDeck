#!/bin/bash

#count all tags with "v*.0" pattern and assign value to $major
major=$(git tag -l "v*.0" | wc -l)

#list all tags descending until major tag is found, count the output and assign result to $minor
minor=$(git for-each-ref --format='%(*creatordate:raw)%(creatordate:raw) %(refname) %(*objectname) %(objectname)' refs/tags | sort -r | awk '{ print $3 }' | sed -e 's/refs\/tags\///g' | sed '/v*.0/Q' | wc -l)

#assign number of commits since last tag to $revision
function revisionCheck_lastTag {

	last_tag_commit=$(git for-each-ref --format='%(*creatordate:raw)%(creatordate:raw) %(refname) %(*objectname) %(objectname)' refs/tags | sort -r | awk '{ print $4 }' | sed -e 's/refs\/tags\///g' | head -1)
	revision=$(git rev-list --remotes | sed '/'$last_tag_commit'/Q' | wc -l)

}

#there are no major tags, count all tags instead
if [ $minor == 0 ]
then
minor=$(git tag | wc -l)

#if minor is still zero, revision is number of tags
if [ $minor == 0 ]
then
revision=$(git rev-list --remotes | wc -l)
else
revisionCheck_lastTag
fi
else
revisionCheck_lastTag
fi

if [ -z "$(git status --porcelain)" ]; then 
  development=0
else 
  development=1
fi

#output $major, $minor and $revision into separate files
echo "software version: $major.$minor.$revision.$development"
echo "$major,$minor,$revision,$development" > version