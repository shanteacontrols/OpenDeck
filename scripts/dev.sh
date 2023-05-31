#!/bin/bash

set -e

compose_file=docker-compose.yml

for arg in "$@"; do
    case "$arg" in
        --build)
            compose_file=docker-compose-build.yml
            build=1
            ;;
    esac
done

# Avoid using dasel or other YAML parsers here not to introduce
# dependencies for starting a container.
container=$(cat $compose_file | grep container_name | xargs | cut -d: -f2 | xargs)

if [[ $(docker ps | grep "${container}") != "" ]]
then
    echo "Found the following containers already online:"
    docker ps -f name="${container}"

    printf "\n%s" "Specify the container ID to attach to: "
    read -r container
    echo "Attaching to $container..."
else
    echo "Creating new container"

    if [[ "$build" -eq 1 ]]
    then
        docker-compose -f $compose_file build
    else
        docker-compose -f $compose_file pull
    fi

    docker-compose -f $compose_file up -d
fi

docker exec -it "$container" bash
