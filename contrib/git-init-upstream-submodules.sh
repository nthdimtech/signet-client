#!/usr/bin/env bash

UPSTREAM_BASEURL="https://github.com/nthdimtech/"  # *including* a trailing slash!

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

# Replace all "../something" urls
echo "Patching .gitmodules"
sed -ibackup -e "s#url = \.\./#url = ${UPSTREAM_BASEURL}#" .gitmodules

echo "Synchronizing new urls"
git submodule sync

echo "Initializing submodules"
git submodule update --init

echo "Restoring original .gitmodules"
mv -vf .gitmodulesbackup .gitmodules

echo "All done."
