#!/bin/bash

args_num="$#"
kmod_path="/proc/physlin"

function print_reg_read_val {
	physical_addr=$1
	echo "$physical_addr" > $kmod_path
	cat $kmod_path
}

if [ $args_num -eq 1 ]
then
	physical_addr=$1
	print_reg_read_val $physical_addr
	exit 0
fi
