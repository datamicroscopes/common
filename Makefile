all: release

.PHONY: release
release:
	@echo "Setting up cmake (release)"
	@python ./cmake/print_cmake_command.py 
	[ -d release ] || (mkdir release && cd release && eval `python ../cmake/print_cmake_command.py`)

.PHONY: debug
debug:
	@echo "Setting up cmake (debug)"
	@python ./cmake/print_cmake_command.py --debug 
	[ -d debug ] || (mkdir debug && cd debug && eval `python ../cmake/print_cmake_command.py --debug`)

.PHONY: test
test:
	([ -d release ] && cd release && make test)
	([ -d debug ] && cd debug && make test)
	(cd test && nosetests --verbose)

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
	mkdir -p build
	(cd build && $(COMPILER) cmake -DCMAKE_INSTALL_PREFIX=$$VIRTUAL_ENV .. && make && make install)
	$(COMPILER) pip install .

.PHONY: travis_script
travis_script: test
