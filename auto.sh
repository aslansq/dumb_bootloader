#!/bin/bash

thisPath=$(realpath "$0")
thisDirPath=$(dirname "$thisPath")

echoerr() { echo "$@" 1>&2; }

ungracefulExit()
{
	echoerr "$@"
	echoerr ERROR!!
	exit 1
}

build()
{
	path=$1
	cd $path
	if [ $? != 0 ]
	then
		ungracefulExit cd to $path fail
	fi

	rm -rf build
	mkdir build
	cd build
	cmake ..
	make

	if [ $? != 0 ]
	then
		ungracefulExit $path build fail
	else
		echo $path build success
	fi
}

custom_copy()
{
	path=$1
	cp $path $thisDirPath/build
	if [ $? != 0 ]
	then
		ungracefulExit $path copy fail
	else
		echo $path copy success
	fi
}

custom_run()
{
	cmd=$@
	cmdDirPath=$(dirname "$cmd")
	cd $cmdDirPath
	$cmd
	if [ $? != 0 ]
	then
		ungracefulExit $cmd run fail
	else
		echo $cmd run success
	fi
}

cd $thisDirPath

rm -rf $thisDirPath/build
mkdir -p $thisDirPath/build

build $thisDirPath/app
build $thisDirPath/app/signer
build $thisDirPath/app/flasher
build $thisDirPath/bl

custom_copy $thisDirPath/app/build/app.elf
custom_copy $thisDirPath/app/build/app.elf.bin
custom_copy $thisDirPath/app/signer/build/signer
custom_copy $thisDirPath/app/flasher/build/flasher
custom_copy $thisDirPath/bl/build/bl.elf
custom_copy $thisDirPath/bl/build/bl.elf.bin

custom_run $thisDirPath/build/signer