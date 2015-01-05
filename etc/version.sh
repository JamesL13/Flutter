#!/bin/sh

DIR=$(cd "$(dirname "$0")" && pwd)

printf "#ifndef FLUTTER_VERSION_H\n#define FLUTTER_VERSION_H\n\n"

printf "/* Automatically generated version file from git tags. --grim */\n\n"

printf "#define FLUTTER_VERSION_MAJOR " && cat $DIR/vmaj
printf "#define FLUTTER_VERSION_MINOR " && cat $DIR/vmin
printf "#define FLUTTER_VERSION_PATCH " && cat $DIR/vpat
printf "#define FLUTTER_VERSION_STRING \""

while read LN
do
    printf $LN
done < $DIR/version

printf "\"\n\n"

printf "#endif\n"

