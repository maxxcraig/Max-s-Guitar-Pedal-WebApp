#!/bin/bash
cd "$(dirname "$0")" || exit
rm -rf build
cmake -Bbuild -S. -DCMAKE_BUILD_TYPE=Debug
cmake --build build
open build/GuitarPedalApp_artefacts/GuitarPedalApp.app


