#!/bin/sh

### currently, looks for either a virtualenv or anaconda install

UNAME=`uname`
if [ "${UNAME}" = "Darwin" ]; then
    SOEXT=dylib
else
    SOEXT=so
fi

if [ -n "${VIRTUAL_ENV}" ]; then
    if [ -f "${VIRTUAL_ENV}/lib/libdistributions_shared.${SOEXT}" ] && [ -d "${VIRTUAL_ENV}/include/distributions" ]; then
        echo "${VIRTUAL_ENV}"
        exit 0
    fi
fi

if [ "${CONDA_BUILD}" = "1" ]; then
    if [ -f "${PREFIX}/lib/libdistributions_shared.${SOEXT}" ] && [ -d "${PREFIX}/include/distributions" ]; then
        echo "${PREFIX}"
        exit 0
    fi
fi

if [ -n "${CONDA_DEFAULT_ENV}" ]; then
    DIR=`conda info | grep 'default environment' | awk '{print $4}'`
    if [ -f "${DIR}/lib/libdistributions_shared.${SOEXT}" ] && [ -d "${DIR}/include/distributions" ]; then
        echo "${DIR}"
        exit 0
    fi
fi

exit 1
