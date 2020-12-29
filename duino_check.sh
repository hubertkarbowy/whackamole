#/bin/bash

if [ -z "${ARDUINO_HOME}" ]; then
    echo "Please set the ARDUINO_HOME environment variable before running make duino."
    exit 1
fi

if [ ! -d ${HOME}/.arduino15 ]; then
    echo "Cannot find the ${HOME}/.arduino15 directory - please install the IDE above version 1.5.0 first."
    exit 1
fi

if [ ! -d ${HOME}/Arduino ]; then
    echo "Cannot find the ${HOME}/Arduino directory - please install the IDE above version 1.5.0 first and the TensorFlow Lite library."
    exit 1
fi
