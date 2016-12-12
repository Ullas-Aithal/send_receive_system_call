#!/bin/sh
cp -pf callnr.h /usr/src/include/minix/callnr.h
cp -pf table.c /usr/src/servers/pm/table.c
cp -pf proto.h /usr/src/servers/pm/proto.h
cp -pf mod_msg_passing_library.h /usr/src/include/mod_msg_passing_library.h
cp -pf mod_msg_passing.c /usr/src/servers/pm/mod_msg_passing.c
cp -pf mod_msg_passing_library.h /usr/src/servers/pm/mod_msg_passing_library.h
cp -pf Makefile /usr/src/servers/pm/Makefile
echo "Files copied. Run install"
