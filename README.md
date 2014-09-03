# microscopes-common [![Build Status](https://travis-ci.org/datamicroscopes/common.svg?branch=master)](https://travis-ci.org/datamicroscopes/common)

The base package. 

## Installation
The only officially supported mechanism is to use [Anaconda](https://store.continuum.io/cshop/anaconda/). 

    $ conda config --add channels distributions 
    $ conda config --add channels datamicroscopes
    $ conda install microscopes-common

We realize that forcing a particular package/environment manager may be offensive to some.
But Anaconda is one of the best solutions out there for distributing complicated python libraries
which depend on many C/C++ libraries. Who wants to deal with library verison mismatch issues, or 
messing around with `LD_LIBRARY_PATH`?

This is probably the closest to "one-click" install that we can achieve in the forseeable future. 

## Building
If you simply want to use datamicroscopes, then see the above instructions on installation. If you want to actively contribute, then follow these steps for building. Our build process relies on anaconda to take care of the dependencies, and then `pip` to install our modified libraries. 

### Requirements
Note that a compiler with support for C++11 is required for building. On Linux, this means `g++ >= 4.8` (4.6 supports C++11, but its headers are somewhat broken for `<random>` which we rely on). On OS X, this means the latest version of XCode. Currently, we only support OS X 10.7 or greater. 

#### Conflict with system libprotobuf
Currently, our `CMakeLists.txt` does not give precedence to Anaconda's version of libprotobuf but instead prefers the system one if present. The easiest workaround for the time being is to uninstall the system versions. This will be fixed soon.

### Setting up the anaconda environment
We recommend you to not work in the default (root) environment

    $ conda create -n myenv anaconda 
    $ source activate myenv
    $ conda config --add channels distributions 
    $ conda config --add channels datamicroscopes
    $ conda install distributions eigen3 cython cmake
    $ export DYLD_LIBRARY_PATH=/path/to/anaconda/envs/myenv/lib # or LD_LIBRARY_PATH on linux
    
Note that on OS X, instead of `DYLD_LIBRARY_PATH` it is also possible to specify `DYLD_FALLBACK_LIBRARY_PATH`.  This fixes some issues with using `libpng` (and consequently `matplotlib`) in conjunction with the datamicroscopes library.

Now if you want nice git SHA1 hashes for package versions (useful for dev), install `gitpython`. Note this step is optional. 
    
    $ pip install gitpython
    
### Building/testing the C++ library
Now that the conda environment is set up, use `cmake` to build the C++ library. Enter the commands below, replacing `debug` with either `relwithdebinfo` or `release` depending on the type of build. Note the first invocation to `make` simply invokes `cmake` with the right prefixes set up.

    $ make debug # calls cmake
    $ cd debug
    $ make 
    $ make test 
    $ make install # installs to prefix /path/to/anaconda/envs/myenv
    $ cd ..
  
### Building/testing the python library
Now use `pip` to install the python library. Either

    $ pip install . 
    $ cd test && nosetests -v
    
or alternatively to build in the source tree:
   
    $ pip install -e . 
    $ nosetests -v
