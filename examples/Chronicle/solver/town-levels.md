# The town's levels, as the solver reaches them

Measured 2026-10-19 on the real rules of the commit that adds the stone-weavers' study
(`research.weaver`, "Chronicle economy: the stone-weavers' study teaches the gem weaver"), on
branch `chronicle-economy`. The rules measured differ from that commit in words only (a tile's
`note`, the fire flask's `about`, "1 Pelt").

## How

For each level, task mode on a copy of the rules, one process a level, two at a time:

    release/ChronicleSolver <rules copy> <solver copy>/solver.pg task raise.<place>.<n> lives=6 tries=6 threads=2

under `ulimit -v 5000000` and `timeout 900` (2700 for the inn's third). Task mode is the hybrid's
planner stage in a world of its own. A figure is the soonest life and age the level is raised
at, within 6 lives.

## The figures

| place | level 1 | level 2 | level 3 | level 3 asks (found) |
|---|---|---|---|---|
| market | life 1, 8y7m | life 1, 18y7m | life 1, 20y6m | pelts |
| mill | life 1, 9y3m | life 2, 11y0m | not reached | pike's jaw (boss) |
| smithy | life 1, 10y5m | life 1, 18y7m | not reached | a plate of the Old Shell (boss) |
| yard | life 1, 23y8m | life 2, 17y11m | life 2, 12y2m | hides |
| gate | life 1, 29y3m | life 3, 8y4m | life 3, 16y11m | fangs |
| chapel | life 1, 18y8m | life 2, 17y1m | life 2, 19y3m | a white antler |
| collegium | life 2, 22y10m | life 3, 10y5m | life 5, 32y5m | wisp-glass |
| harrow | not reached | not measured | not measured | bog iron |
| inn | life 2, 8y9m | life 2, 11y8m | not reached | the Old Grey's pelt (boss) |
| storehouse | life 2, 7y3m | life 2, 13y9m | life 2, 20y6m | bog iron |
| lodge | life 1, 15y9m | life 1, 16y9m | life 1, 18y5m | (none) |

- **Gems come without a boss now.** Every level 2 is reached, and so is every level 3 that asks
  no boss's trophy. The solver studies the stone-weavers' craft at 14y0m in its first life,
  binds a weaver at 15, and later lives bind one from 11 or 12.
- **Not reached: mill 3, smithy 3, inn 3.** Each asks a boss's trophy, and the solver never
  fights a boss. That is the solver's reach, not the amounts: they were reached in the part 2
  boss proxy (life 2), and no number was changed.
- **The Harrow Road is not measured.** Its place comes onto the page only after the expedition to
  Harrow (`harrow_truth`), and the solver never reaches that expedition: "Coin 0/10; Stone 0/20;
  Clay 0/20; not listed". It was the same before part 2. Its levels 2 and 3 ran out of time
  (900 s) without a line, for the same reason.
- Where a level 3 is sooner than its level 2 (the yard), each was searched in a world of its own:
  the level 3 search raises the levels before it on its way.

## No level asks a found item a life cannot reach

- **Wisp-glass (collegium 3):** reached, life 5, 32y5m, with the glass the solver took from the
  marsh lights itself (the Marsh opens at 18). Late, and within the 6 lives.
- **Bog iron (storehouse 3):** reached, life 2, 20y6m, with bog iron the solver dug in the Marsh.
- **Bog iron (harrow 3):** the same bog iron; the level is not measured, for the place (above).
- Every walk that brings a found item is open to every class, long before a life ends.

## Numbers

None changed: no level that the solver can reach takes more than 6 lives.
