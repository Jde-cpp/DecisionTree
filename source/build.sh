#!/bin/bash
debug=${1:-1}
clean=${2:-0}

#echo all=$all
cd "${0%/*}"
./lightgbm/build.sh $debug $clean
if [ $? -eq 1 ]; then
	exit 1
fi
./xgb/build.sh $debug $clean
if [ $? -eq 1 ]; then
	exit 1
fi
if [ $clean -eq 1 ]; then
	make -C. clean DEBUG=$debug
	if [ $debug -eq 1 ]; then
		ccache g++-8 -c -g -pthread -fPIC -std=c++17 -Wall -fsanitize=address -fno-omit-frame-pointer -Wno-unknown-pragmas -DJDE_DTS_EXPORTS -O0 pc.h -o.obj/debug/stdafx.h.gch -I../../framework -I/home/duffyj/code/libraries/spdlog/include -I/home/duffyj/code/libraries/eigen -I/home/duffyj/code/libraries/json/include  -I/home/duffyj/code/libraries/boostorg/boost_1_68_0
	else
		ccache g++-8 -c -g -pthread -fPIC -std=c++17 -Wall -Wno-unknown-pragmas -DJDE_DTS_EXPORTS -march=native -DNDEBUG -O3 pc.h -o.obj/release/stdafx.h.gch -I../../framework -I/home/duffyj/code/libraries/spdlog/include -I/home/duffyj/code/libraries/eigen -I/home/duffyj/code/libraries/json/include  -I/home/duffyj/code/libraries/boostorg/boost_1_68_0
	fi
	if [ $? -eq 1 ]; then
		exit 1
	fi
fi
make -C. -j DEBUG=$debug
cd -
exit $?