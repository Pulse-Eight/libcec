#!/bin/sh

## cmake doesn't read the variable when it doesn't end with a newline, and I haven't figured out how to have it add a newline directly...
if git rev-parse --git-dir > /dev/null 2>&1; then
  last_tag=`git describe --tags --abbrev=0`
  last_hash=`git --no-pager log --abbrev=7 -n 1 --pretty=format:"%h"`
  commits_since_tag=`git log ${last_tag}..HEAD --oneline | wc -l`
  git_dirty=`git diff HEAD | wc -l`
  if [ $commits_since_tag -gt 0 ]; then
    dirty=""
    if [ $git_dirty -gt 0 ]; then
      dirty="~dirty"
    fi
    echo "${last_tag}+${commits_since_tag}-${last_hash}${dirty}"
  else
    echo $last_tag
  fi
else
  echo "<unknown>"
fi
