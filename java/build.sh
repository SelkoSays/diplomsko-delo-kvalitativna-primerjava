#!/usr/bin/env bash

rm -rf bin

javac --module-path lib/jna --source-path src -d bin src/si/Main.java $*
