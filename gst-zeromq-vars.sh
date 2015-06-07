#!/bin/sh

SCRIPT_PATH="${BASH_SOURCE[0]}";
if ([ -h "${SCRIPT_PATH}" ]) then
  while([ -h "${SCRIPT_PATH}" ]) do SCRIPT_PATH=`readlink "${SCRIPT_PATH}"`; done
fi
pushd . > /dev/null
cd `dirname ${SCRIPT_PATH}` > /dev/null
SCRIPT_PATH=`pwd`;
popd  > /dev/null

echo script path is $SCRIPT_PATH

export GST_PLUGIN_PATH_1_0=$GST_PLUGIN_PATH_1_0:$SCRIPT_PATH/src/zeromq/.libs:

