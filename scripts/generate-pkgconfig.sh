#!/bin/bash
#
# This script create a pkg-config file for this clone of yavl-cpp
# It's fixed to reference to the ../build as the build directory.
#
set -e

abspath=$(cd `dirname "${BASH_SOURCE[0]}"`/.. && pwd)
pkg_config_path="$1"

if [ -z ${pkg_config_path} ]; then
   echo Please specify your pkgconfig directory
   exit 1
fi

mkdir -p ${pkg_config_path}
cat > ${pkg_config_path}/yavl-cpp.pc  <<EOF
Name: Yavl-cpp
Description: A YAML format validator for C++
Version: 0.5.1

prefix=$abspath
libdir=\${prefix}/build
includedir=\${prefix}/include

Requires: yaml-cpp >= 0.5.1
Libs: -L\${libdir} -lyavl-cpp
Cflags: -I\${includedir}
EOF
