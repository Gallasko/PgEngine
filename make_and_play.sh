#!/bin/bash
# Usage: ./make_and_play.sh <target> [game args...]
# e.g.   ./make_and_play.sh Chronicle --fresh --age 7
target="${1}"
shift
cd release
make -j8 "${target}" || exit 1
cd ..
./release/"${target}" "$@"
