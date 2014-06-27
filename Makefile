-include config.mk

DEBUG ?= 0

O := out
TOP := $(shell echo $${PWD-`pwd`})

# set the CXXFLAGS
CXXFLAGS := -fPIC -g -MD -Wall -std=c++0x -I$(TOP)/include
ifneq ($(strip $(DEBUG)),1)
	CXXFLAGS += -O3 -DNDEBUG
endif
ifneq ($(strip $(DISTRIBUTIONS_INC)),)
	CXXFLAGS += -I$(DISTRIBUTIONS_INC)
endif

# set the LDFLAGS
LDFLAGS := -lprotobuf -ldistributions_shared 
ifneq ($(strip $(DISTRIBUTIONS_LIB)),)
	LDFLAGS += -L$(DISTRIBUTIONS_LIB) -Wl,-rpath,$(DISTRIBUTIONS_LIB)
endif

SRCFILES := $(wildcard src/common/*.cpp src/io/*.cpp src/models/*.cpp) 
OBJFILES := $(patsubst src/%.cpp, $(O)/%.o, $(SRCFILES))

UNAME_S := $(shell uname -s)
TARGETS :=
LIBPATH_VARNAME :=
ifeq ($(UNAME_S),Linux)
	TARGETS := $(O)/libmicroscopes_common.so
	LIBPATH_VARNAME := LD_LIBRARY_PATH
endif
ifeq ($(UNAME_S),Darwin)
	TARGETS := $(O)/libmicroscopes_common.dylib
	LIBPATH_VARNAME := DYLD_LIBRARY_PATH
endif

all: $(TARGETS)

$(O)/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(O)/libmicroscopes_common.so: $(OBJFILES)
	gcc -shared -o $(O)/libmicroscopes_common.so $(OBJFILES) $(LDFLAGS)

$(O)/libmicroscopes_common.dylib: $(OBJFILES)
	g++ -dynamiclib -o $(O)/libmicroscopes_common.dylib $(OBJFILES) $(LDFLAGS)

DEPFILES := $(wildcard out/common/*.d out/io/*.d out/models/*.d)
ifneq ($(DEPFILES),)
-include $(DEPFILES)
endif

.PHONY: clean
clean: 
	rm -rf out
	find microscopes \( -name '*.cpp' -or -name '*.so' \) -type f -print0 | xargs -0 rm --

.PHONY: protobuf
protobuf:
	mkdir -p src/io
	protoc --cpp_out=include --python_out=. microscopes/io/schema.proto
	mv include/microscopes/io/schema.pb.cc src/io/schema.pb.cpp

.PHONY: test
test:
	$(LIBPATH_VARNAME)=$$$(LIBPATH_VARNAME):./out nosetests
