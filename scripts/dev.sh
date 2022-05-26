#!/bin/bash

set -e

project="opendeck"
service="opendeck"

if [[ $(docker ps | grep "${project}_${service}") != "" ]]
then
    echo "Found the following containers already online:"
    docker ps -f name="${project}"_"${service}"

    printf "\n%s" "Specify the container ID to attach to: "
    read -r container
    echo "Attaching to $container..."
else
    echo "Creating new container"
    docker-compose build
    docker-compose -f docker-compose.yml up -d
    container=${project}_${service}_1
fi

docker exec -it "$container" bash
