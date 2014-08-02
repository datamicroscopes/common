# microscopes-common [![Build Status](https://magnum.travis-ci.com/datamicroscopes/common.svg?token=vAx3hGEdmPG3ovJq2Zv6&branch=master)](https://magnum.travis-ci.com/datamicroscopes/common)

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

### Setting up the anaconda environment
We recommend you to not work in the default (root) environment

    $ conda create -n myenv anaconda 
    $ source activate myenv
    $ conda config --add channels distributions 
    $ conda config --add channels datamicroscopes
    $ conda install distributions pymc eigen3 cython
    $ export DYLD_LIBRARY_PATH=/path/to/anaconda/envs/myenv/lib # or LD_LIBRARY_PATH on linux
    
Now if you want nice git sha1 hashes for package versions (useful for dev), install `gitpython`. Note this step is optional. 
    
    $ pip install gitpython
    
### Building/testing the C++ library
Now that the conda environment is set up, use `cmake` to build the C++ library. Replace `debug` with either `relwithdebinfo` or `release` depending on the type of build.

    $ make debug 
    $ cd debug
    $ make 
    $ make test 
    $ make install # installs to prefix /path/to/anaconda/envs/myenv
    $ cd ..
  
### Building/testing the python library
Now use `pip` to install the python library.

    $ pip install . 
    $ cd test && nosetests
