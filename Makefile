SHELL := /bin/bash

.PHONY: travis_before_install
travis_before_install:
	sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
	sudo add-apt-repository -y ppa:cython-dev/master-ppa
	sudo apt-get update -qq
	sudo apt-get install -qq g++-4.8 gfortran protobuf-compiler \
		libprotobuf-dev libeigen3-dev python-scipy cython \
		libblas-dev liblapack-dev libatlas-base-dev

.PHONY: travis_install_py_deps
travis_install_py_deps:
	pip install -r .travis/requirements.txt

.PHONY: travis_install_distributions
travis_install_distributions: travis_install_py_deps
	(cd .travis && python install_distributions.py)

COMPILER := CC=gcc-4.8 CXX=g++-4.8

.PHONY: travis_install
travis_install: travis_install_distributions
	mkdir build
	(cd build && $(COMPILER) cmake -DCMAKE_INSTALL_PREFIX=$$VIRTUAL_ENV .. && make && make install)
	$(COMPILER) pip install .

.PHONY: travis_script
travis_script: 
	(cd build && make test)
	(cd test && nosetests --verbose)
