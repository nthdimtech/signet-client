#!/usr/bin/env bash

# dependencies: bash, git, cat

GITVERSION_FILE=client/gitversion.h


# Ensure that we are in the git repository base directory

if [ -e /dev/null ]; then
    NULLFILE=/dev/null
else
    # Windows
    NULLFILE=NUL
fi
# Use --git-dir to be able to handle submodules
# We assume that $GITDIR is not set or not pointing anywhere else
GITDIR=$(git rev-parse --git-dir 2>${NULLFILE})
if [ $? -ne 0 ]; then
    echo "${0}: The current working directory '"$(pwd)"' is not in a git repository!"
    exit
else
    BASEDIR=${GITDIR%%\.git*}
    if [ -n "$BASEDIR" ]; then
        cd ${BASEDIR}
    fi  # else: we are already in the git repository base directory
fi


# Add GITVERSION if git gives us a description, otherwise ensure that GITVERSION_FILE file is empty

if [ -e ${GITVERSION_FILE} ]; then
    gitver=$(git describe --tags --always --dirty 2>${NULLFILE})
    if [ -z "${gitver}" ]; then
        # remove DEFINES
        echo -n '' > ${GITVERSION_FILE}
    fi
    # add DEFINES
    cat > ${GITVERSION_FILE} <<EOF
#ifndef GITVERSION_H
#define GITVERSION_H

#define GITVERSION "${gitver}"

#endif // GITVERSION_H
EOF
    echo "${0}: GITVERSION is set to '"${gitver}"'"
else
    echo "${0}: File ${GITVERSION_FILE} does not exist."
    echo "${0}: Something went wrong when retrieving the repository base directory!"
    exit 2
fi
