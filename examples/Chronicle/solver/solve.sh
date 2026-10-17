#!/bin/bash
# solve.sh - Chronicle's solver over several processes, merged into one report.
#
#   examples/Chronicle/solver/solve.sh [all|beam|hybrid] [processes] [report] [key=value...]
#
# From the repository's root, with ChronicleSolver built in release/.
#   all   (the default) each process plays the same shared world, then takes its share of the
#         tasks left (solver.pg's shard=i/n)
#   beam  each process runs the search that tries every task with dice of its own (seed=1, 2...):
#         more processes, more of the game seen in the same time
#   hybrid  each process runs the same beam until it stops finding tasks, then takes its share of
#         the tasks it left to the planner (shard=i/n)
# The parts are merged into the report (chronicle-solver.txt by default). The options after the
# report go to every process: lives=5, tries=20 for all; width=8, minutes=0 for beam; for hybrid
# both, and stall=240, stallFinds=3.
#
# One process with threads does better than several for hybrid (and beam with one seed): the
# beam is run once and the planner's tasks go to whichever thread is free:
#   release/ChronicleSolver examples/Chronicle/rules examples/Chronicle/solver/solver.pg hybrid \
#       report=chronicle-solver.txt width=10 lives=5 tries=6 threads=8
# The processes here run without threads (threads=0) unless the options say otherwise.

set -e

mode=all

if [[ "$1" == "all" || "$1" == "beam" || "$1" == "hybrid" ]]; then
    mode=$1
    shift
fi

jobs=${1:-6}
report=${2:-chronicle-solver.txt}
shift 2 2>/dev/null || shift $#

solver=release/ChronicleSolver
rules=examples/Chronicle/rules
script=examples/Chronicle/solver/solver.pg

parts=()

# Each process compiles the rules beside a copy of its own: a script's bytecode is written next to
# it when it is loaded, and two processes writing the same .pgc spoil it for both
copies=$(mktemp -d)
trap 'rm -rf "$copies"' EXIT

for ((i = 0; i < jobs; i++)); do
    part="$report.$i"
    parts+=("$part.part")
    mkdir -p "$copies/$i"
    cp "$rules"/*.pg "$copies/$i/"
    if [[ $mode == beam ]]; then
        "$solver" "$copies/$i" "$script" beam "report=$part" "seed=$((i + 1))" threads=0 "$@" > "$part.log" 2>&1 &
    elif [[ $mode == hybrid ]]; then
        "$solver" "$copies/$i" "$script" hybrid "report=$part" "shard=$i/$jobs" threads=0 "$@" > "$part.log" 2>&1 &
    else
        "$solver" "$copies/$i" "$script" all "report=$part" "shard=$i/$jobs" threads=0 "$@" > "$part.log" 2>&1 &
    fi
done

wait

mkdir -p "$copies/merge"
cp "$rules"/*.pg "$copies/merge/"
"$solver" "$copies/merge" "$script" merge "report=$report" "${parts[@]}" | head -1
echo "written to $report (each part's log beside it: $report.<i>.log)"
