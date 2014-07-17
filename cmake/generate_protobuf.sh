#!/bin/sh
DIR=$1
mkdir -p $DIR/include/microscopes/io $DIR/src/io
protoc --cpp_out=$DIR/include microscopes/io/schema.proto
mv $DIR/include/microscopes/io/schema.pb.cc $DIR/src/io/schema.pb.cpp
