#!/bin/sh

## cmake doesn't read the variable when it doesn't end with a newline, and I haven't figured out how to have it add a newline directly...
echo -n "`git --no-pager log --abbrev=7 -n 1 --pretty=format:"%h"`\n"

