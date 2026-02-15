#!/usr/bin/env bash

rm -rf bin

javac --source-path src -d bin src/si/Main.java $*
