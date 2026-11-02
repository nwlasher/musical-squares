#!/usr/bin/env bash

# This 'script' remembers this command so I don't have to
# Usage: chmod +x map-repo.sh -> ./map-repo.sh [Enter]

# Old from when directory was small
#tree --dirsfirst --noreport -n -F --charset=utf-8 \
#     -I '.git|.pio|build|repo-tree.txt' > Documentation/Technical-reference/repo-tree.txt

# New handles trimming non essential paths
OUT=Documentation/Technical-reference/repo-tree.txt

find . -mindepth 1 \( -name .git -o -name .pio -o -name build -o -name repo-tree.txt \) -prune -o \
     \( -type d -printf '%P/\n' -o -type f -printf '%P\n' \) \
  | grep -Ev '^Code/Testbenches/[^/]+/.' \
  | sort \
  | tree --fromfile --dirsfirst --noreport -n -F --charset=utf-8 > "$OUT"
