#!/bin/bash
type=${1:-asan}
clean=${2:-0}
all=${3:-1}

../../Framework/cmake/buildc.sh ../../LightGbm/source $type $clean || exit 1;
cd "${0%/*}"
../../Framework/cmake/buildc.sh `pwd` $type $clean || exit 1;
cd - > /dev/null