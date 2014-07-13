-include config.mk

DEBUG ?= 0

O := out
TOP := $(shell echo $${PWD-`pwd`})

# set the CXXFLAGS
CXXFLAGS := -fPIC -g -MD -Wall -std=c++0x -I$(TOP)/include
ifneq ($(strip $(DEBUG)),1)
	CXXFLAGS += -O3 -DNDEBUG
else
	CXXFLAGS += -DDEBUG_MODE
endif
ifneq ($(strip $(DISTRIBUTIONS_INC)),)
	CXXFLAGS += -I$(DISTRIBUTIONS_INC)
endif

# set the LDFLAGS
LDFLAGS := -lprotobuf -ldistributions_shared
ifneq ($(strip $(DISTRIBUTIONS_LIB)),)
	LDFLAGS += -L$(DISTRIBUTIONS_LIB) -Wl,-rpath,$(DISTRIBUTIONS_LIB)
endif

SRCFILES := $(wildcard src/common/*.cpp)
SRCFILES += $(wildcard src/common/recarray/*.cpp)
SRCFILES += $(wildcard src/common/sparse_ndarray/*.cpp)
SRCFILES += $(wildcard src/models/*.cpp)
SRCFILES += src/io/schema.pb.cpp
OBJFILES := $(patsubst src/%.cpp, $(O)/%.o, $(SRCFILES))

TESTPROG_SRCFILES := $(wildcard test/cxx/*.cpp)
TESTPROG_BINFILES := $(patsubst %.cpp, %.prog, $(TESTPROG_SRCFILES))

TESTPROG_LDFLAGS := $(LDFLAGS)
TESTPROG_LDFLAGS += -L$(TOP)/out -Wl,-rpath,$(TOP)/out
TESTPROG_LDFLAGS += -lmicroscopes_common

UNAME_S := $(shell uname -s)
TARGETS :=
LIBPATH_VARNAME :=
ifeq ($(UNAME_S),Linux)
	TARGETS := $(O)/libmicroscopes_common.so
	LIBPATH_VARNAME := LD_LIBRARY_PATH
	EXTNAME := so
	SOFLAGS := -shared
endif
ifeq ($(UNAME_S),Darwin)
	TARGETS := $(O)/libmicroscopes_common.dylib
	LIBPATH_VARNAME := DYLD_LIBRARY_PATH
	EXTNAME := dylib
	SOFLAGS := -dynamiclib -install_name $(TOP)/$(O)/libmicroscopes_common.$(EXTNAME)
endif

all: $(TARGETS)

.PHONY: build_test_cxx
build_test_cxx: $(TESTPROG_BINFILES)

.PHONY: build_py
build_py: $(O)/libmicroscopes_common.$(EXTNAME)
	python setup.py build_ext --inplace

$(O)/%.o: src/%.cpp include/microscopes/io/schema.pb.h
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(O)/libmicroscopes_common.$(EXTNAME): $(OBJFILES)
	$(CXX) -o $@ $(OBJFILES) $(LDFLAGS) $(SOFLAGS)

%.prog: %.cpp $(O)/libmicroscopes_common.$(EXTNAME)
	$(CXX) $(CXXFLAGS) $< -o $@ $(TESTPROG_LDFLAGS)

DEPFILES := $(wildcard out/common/*.d)
DEPFILES := $(wildcard out/common/recarray/*.d)
DEPFILES := $(wildcard out/common/sparse_ndarray/*.d)
DEPFILES += $(wildcard out/io/*.d)
DEPFILES += $(wildcard out/models/*.d)
ifneq ($(DEPFILES),)
-include $(DEPFILES)
endif

PBGENFILES := src/io/schema.pb.cpp include/microscopes/io/schema.pb.h microscopes/io/schema_pb2.py

.PHONY: clean
clean:
	rm -rf out test/cxx/*.{d,dSYM,prog} $(PBGENFILES)
	find microscopes \( -name '*.cpp' -or -name '*.so' -or -name '*.pyc' \) -type f -print0 | xargs -0 rm -f --

include/microscopes/io/schema.pb.h: microscopes/io/schema.proto
	mkdir -p src/io
	protoc --cpp_out=include --python_out=. microscopes/io/schema.proto
	mv include/microscopes/io/schema.pb.cc src/io/schema.pb.cpp

src/io/schema.pb.cpp: include/microscopes/io/schema.pb.h

.PHONY: test
test: build_py test_cxx
	$(LIBPATH_VARNAME)=$$$(LIBPATH_VARNAME):./out nosetests

.PHONY: test_cxx
test_cxx: build_test_cxx
	test/cxx/test_sparse_ndarray.prog

SHELL := /bin/bash
S := cd .travis
S1 := $(S) && cd distributions

.PHONY: travis_before_install
travis_before_install:
	sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
	sudo add-apt-repository -y ppa:cython-dev/master-ppa
	sudo apt-get update -qq
	sudo apt-get install -qq g++-4.8 gfortran protobuf-compiler libprotobuf-dev libeigen3-dev python-scipy cython

.PHONY: travis_install_py_deps
travis_install_py_deps:
	pip install -r .travis/requirements.txt

.PHONY: travis_install_distributions
travis_install_distributions: travis_install_py_deps
	($(S) && git clone https://github.com/forcedotcom/distributions.git)
	($(S1) && DISTRIBUTIONS_USE_PROTOBUF=1 PYDISTRIBUTIONS_USE_LIB=1 make install_cy)

.PHONY: travis_install
travis_install: travis_install_distributions
	cp .travis/config.mk .
	echo "DISTRIBUTIONS_LIB = $$VIRTUAL_ENV/lib" >> config.mk
	make build_py build_test_cxx

.PHONY: travis_script
travis_script: test
