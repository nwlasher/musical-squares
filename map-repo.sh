#!/usr/bin/env bash

# This 'script' remembers this command so I don't have to
# Usage: chmod +x map-repo.sh -> ./map-repo.sh [Enter]

tree --dirsfirst --noreport -n -F --charset=utf-8 \
     -I '.git|.pio|build|repo-tree.txt' > Documentation/Technical-reference/repo-tree.txt
