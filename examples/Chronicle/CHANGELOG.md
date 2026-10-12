# Chronicle changelog

What changed in the game, build by build, as a player sees it. The engine's own changes are in
the `CHANGELOG.md` at the root of the repository.

## 0.6 - Unreleased

### New

- **His sixth year.** Every life now begins at 6, and its first year is a row of short tasks, a
  month or two each, that decide who the boy is at seven. You glean for your first rations and
  carry flour for your first coin. Then a first choice, **one of two**: *Climb the Mill Wheel*
  (Strength and Dexterity) or *Sit with the Old Scribe* (Intelligence and Vitality); the one
  you leave is gone. Then **two of three** tasks of two months that close before you can do
  them all: *Learn Knots from the Bargemen* (Dexterity and Intelligence), *Haul Sacks at the
  Mill* (Strength, at a cost in Vitality) or *Sing at the Chapel Door* (Intelligence and
  Renown). Then the market, the harvest (a point of everything), and a month at the mill that
  ends the day you are seven. A whole year takes about half a minute, and no two need be alike.
- **A keepsake.** *Carry Books for Father Anselm* pays in coin and in a small tome he tells you
  to keep. It is in your Inventory. Nothing asks for it yet.
- **The wooden blade is bought, then learned, then chosen.** From your eighth year the market
  sells a wooden blade, once, for 8 coin. Holding one brings *Practice with the Wooden Blade*
  into your training: two months, a point of Arms and the basics of the blade. Then **one of
  two**, three months each: *Practice Slashes* (Strength and Arms) or *Practice Thrusts*
  (Dexterity and Arms); the one you leave is gone. No age closes any of it.
- **Two things to do in a fight, for a boy.** The edge teaches the **Slash** (it bites deeper
  than a strike and misses more), the point the **Thrust** (it seldom misses, and bites a
  little less). They are a better way to strike and cost nothing: no breath, no turn to wait.
  They are yours for the rest of the life, whatever class you take.
- **Watch the Tourney at the Yard.** A month, from nine, for a boy with no blade. It leaves
  you a **Yard enthusiasm**, in your Inventory: with it the sergeants let you train at the
  yard, and your first term there takes it.
- **The yard teaches the basics.** A first term of *Train at the Yard* gives the basics of the
  blade to who came without them, so the edge or the point is theirs to choose too.
- **Learn the Yard's Breathing.** Three months, once, for who has both practiced slashes or
  thrusts and trained at the yard: a point of Discipline and the breathing itself, which
  you keep. Nothing in a fight uses it yet.

- **One or the other is said, in red.** A task that closes another when you take it (the mill
  wheel or the old scribe, slashes or thrusts) says so in its description before you choose,
  and the log writes which one went when its term is done.

### Changed

- **Every life starts smaller.** 6 years old, 4 of each stat and 35 Vitality, with nothing in
  your Inventory, where a life began at 7 with 6 of each, 40 Vitality and a year of rations.
  The sixth year gives it back, your way: you are seven with 5 to 8 in a stat.
- **Practice with Wooden Blades is gone.** Six months, three times over, for Arms and
  Dexterity: the three short steps above stand in its place. They bring 2 Arms in five months
  where it brought 3 in eighteen.
- **Train at the Yard has to be earned.** Its row comes once you hold an enthusiasm for the
  yard or the basics of the blade, either one, and no longer for Strength alone.
- **The Yard's racks lend the blade.** *Stock the Yard with Blades* made the old practice
  last five terms. It now lends every boy a wooden blade: none has to be bought.
- **Go to the Market replaces Explore Bellmoor.** One month of your sixth year opens the Town
  page, where your coin buys the rations you are short of by then. Every life goes there once:
  what was built in town stays built from life to life, but the page opens the month you go.
- **The tutorial is the sixth year.** A first life is shown its tasks one at a time and the
  guide says a sentence for each: your food, your coin, the choice (both tiles lit), your stats,
  the stopwatch on tasks that close, the market and how to buy, the task that takes two months,
  your age, and the classes the day you are seven. Its sentences are shorter-lived, and none
  asks you to wait for a task of three months any more. Later lives have no guide and the
  whole year on the page from the first day.
- **The town's work comes all at once at seven** in a first life too, counted in one toast
  ("New: 9 tasks") where each used to be named.
- **The places to walk to are tiles**, like the places of the town: its mark and its name, a
  seal for each level you know it to, and a line with what kind of walk it is and what it
  costs, or what it asks. A closed place can be chosen too: the button says what it asks.
- **The Adventure tab says when it has something new.** The small count at its corner used to
  come only when a place opened to you. It now comes with the tab itself, the day of your first
  walk, and whenever a new place appears on the page, open or not yet.
- *Start Helping Out* is the last month before seven now, and brings 6 rations.
- The anonymous play statistics now say which build they are from and which tasks of the sixth
  year were done (which of the two, which two of the three).

### Fixed

- **The low-food hint of the tutorial could go unsaid.** It was lost when your rations had run
  low once before the hint was allowed to speak.

## 0.5 - 2026-10-11

### New

- **Walks outside the town.** The Hedgerows are the first of four places to walk to (the
  Hedgerows, Mill Pond, the Hollow Woods, the Quarry Road). The Hedgerows have a tile from the
  start, right under your work: tiles that take you out of the town have a look of their own, a
  red frame and their mark. When you begin it you are taken there: the stay is a number of
  days, and you spend them one choice at a time (gather, study the place, rest, seek a beast or
  watch it, go home). A work is a tile with its name and its days; hover it to read what it is,
  what it costs and what it gives. Mill Pond shows once you have walked the Hedgerows.
- **Fights.** A beast you seek is fought turn by turn. Each beast has a small panel of its own
  across the top, the size of yours, with you and your turn under it: its name, how it stands,
  and the move it is about to make, by its name alone (*Bite*). Hover the move to read what it
  is; what it does to you is written there once it has been used on you, or once you have
  watched the beast from cover. Your turn is a few buttons: strike, defend, take a remedy if you
  hold one, flee. A press does it, and hovering a button says what it would do. A small bar
  fills before each blow, yours and the beast's, so you see it coming; whoever is hurt has their
  vitality bar flash and fall, and what was taken lifts off it; a blow that misses lifts a
  small *MISS* instead. The order of the turn has a panel of its own at the top right: who acts
  when, and how many blows are coming at you. Every three turns of a fight cost a day of the
  stay. You can also let him fight on by himself. Wolves hunt in packs of
  three; one of them hangs back while two stand. No rat is quicker than a boy.
- **What you learn in a life, you fight with.** A man sworn to the Keep has a *Heavy cut*, and
  a veteran of the campaign a *Sweep*. A scholar of the Collegium throws a *Firebolt*, stronger
  with Intelligence, and Magister Orin's assistant knows a *Binding word* that costs a beast its
  move. The streets teach a *Feint*. Nobody else sees these in a fight.
- **The record, and walking it again.** Everything you do on a walk is written down. When you
  are home the record is kept, five to a place, and it can be walked again for you, once or
  several times in a row: no choice to make, no fight to play, and what the beasts drop is
  rolled again each time.
- **The Adventure tab** comes with your first walk, and the walks leave the Life page for it.
  Tabs that are not there leave no gap, and once you know the town they read Life, Town,
  Adventure. A small count at the tab's corner says a new place has opened.
  - *Where to walk* lists every place you know of, the open ones first: what kind of walk it
    is, its months and days, what it gives, how well you know it, and for a closed one what it
    asks and what you are short of. *Set out for the Hedgerows · 1 mo* starts a new walk to the
    place chosen.
  - *Records* is a table of the walks you have kept of that place, the best first: when it was
    walked and in how many steps, what it costs in months and Vitality, what it brings, and a
    *Walk* button that walks it once.
  - *Walk it again* is filled by the record chosen: how many times, when to stop (never, when
    the bag is short, when you hold enough of something, at an age), and four figures for what
    the walks take, use, bring and teach, with *Walk once* and *Queue*. While a queue runs it
    gives way to *At work now*: which walk of how many, and a way to stop after this one.
- **Knowledge.** Studying a place and watching a beast are kept by the world, from life to life:
  beasts get cheaper to find, you see their vitality and what they will do next, then their
  weakness, and a place you know well lets you stay longer.
- **Things to sell, and things to build with.** Berries, wortleaf, fish, reeds, pelts, hides,
  fangs are sold at the market. Wortleaf can be brewed into remedies. Firewood, clay, ash wood
  and stone are what places of the town are built of. All of it is in your Inventory, and all of
  it is lost when the life ends, except what the place you leave your coin to still needs: that
  much stays with the place, and the next life finds it asking less.
- **The Hunters' Lodge.** A tenth place in town, shown once you have walked the Hollow Woods and
  built of what your walks brought back. It opens the Hollow Woods to every later life.
- **Events on the way.** Now and then something waits for an answer: a nest in the hedge, a
  hunter by a cold fire. What you choose is written in the record.
- **Two new tasks.** *Race the Rooftops* (Dexterity and Renown, on the streets, from 11) for
  who means to live by quick hands, and *Keep the Night Offices* (Discipline) at the Collegium.

### Changed

- **A task's time at a glance.** A tile shows how long the task takes in a small box at its top
  right corner. Under its name, at the right, a stopwatch counts the months you still have to
  begin it, for a task that closes: all along for one with a door of its own, and in its last
  two years for the rest. It turns red in the last six months.
- **Sharp on a dense screen.** In a browser on a 4K monitor or a phone the game was drawn small
  and stretched. It is drawn at the screen's own resolution now.
- **The market is in town.** What is sold, bought and brewed at the market is on the Town page
  only: it no longer shows on the Life page before the town is known, and the *Go to the Market*
  tile is gone (the Town tab goes there).
- **Shorter tooltips.** A task no longer lists "Room for Rations" among what it asks while the
  store has room.
- **One Vitality.** Your Vitality is the same number at home, on a walk and in a fight: a boy
  has 40, where he had 8. A month with nothing to eat takes 5, and
  5 come back every two months (every month with a room at the inn). Work that builds Vitality
  brings 5, old age takes 5 every two months, and being carried home from a walk costs 5 of the
  most it can be. A save from before is brought to the new count when it is opened.
- The anonymous play statistics now say more of the walks and of the end of a life: how often
  each place was walked, by hand and by a record, the fights met and how they ended (won, lost,
  fled, left to fight on by himself), the walks he was carried home from, the longest queue, and
  which works of a class and of old age were done. Counts, as before: nothing of who plays.
- **The page is no longer built again for nothing.** When a task ends, only the tiles that
  changed are made again: the others stay where they are, and a tile you had chosen stays
  chosen. When the window changes size, what is on the page takes its new width where it
  stands (on the phone page, on a walk and in a fight too) instead of the whole page being
  built again. It is only built again when the layout itself changes: three columns to one.

### Fixed

- **Resizing the window left a mess.** Dragging the window by its edge, on the page or on a
  walk, could leave pieces of the page drawn over each other at the top left, and they stayed.
  Nothing is left behind now, at any size.

## 0.4.1 - 2026-10-11

Not tagged on its own: 0.5 has all of it.

### New

- **Playable on a phone.** In a browser narrower than 600 px (a phone held upright) the game
  is one column: the work at hand on top, one row of tabs (Life, Stats, Items, Years, Log,
  Town), and one of them at a time under it. Tap a task, a stat or a place to read about it; tap
  twice to begin a task, or use Begin.
- **Taps work.** A quick tap on a touch screen was not seen as a click.
- **A Go on button.** When the months stop (a stat is failing), a *Go on* button stands under
  the work. Before, only the Space key went on, and a phone has none.

### Changed

- The anonymous play statistics now include the size of the window the game is shown in and
  whether the screen is a touch screen, to know which screens to draw the game for.
- **All a place gives.** The tooltip of a place lists what every level you built gives, not the
  last one alone.

### Fixed

- A tooltip could stay on screen after its task was replaced (a month passed while the mouse
  was on it), hiding what was under it.

## 0.4 - 2026-10-10

### New

- **The town.** *Explore Bellmoor* (three months, once) opens a Town page beside the Life page.
  Its tile stands out in gold so you do not miss it, and the Town tab only appears once you have
  explored. A short tutorial then shows the tab, the market where your coin buys rations, and
  the places.
  It has nine places of three levels each, and it grows with what you do. At first you see
  two: the Market and the Mill. The others are found by living:
  - the Smithy, once you have worked a term at the forge;
  - the Yard, once you have joined the Border Campaign, and the Watch Gate, once you have
    taken the Captain's Chain;
  - the Chapel, once you have entered the Collegium, the Collegium Gate after assisting
    Magister Orin, and the Harrow Road after the expedition to Harrow;
  - the Inn, from the second life of a world.

  The day a place comes, the work that builds it comes with it. A place you have seen stays
  on the page for your next lives.
- **Works for the town.** On the Town page, under the places, a work raises a place by one
  level. Clicking a place picks its work for you. They cost months and coin, and some cost a
  stat or years of your life. This is time you do not spend on yourself.
- **The town stays.** What you raise is kept when a life ends. The next life is born into that
  town: tasks that are shorter or pay more, classes that ask a little less, and a few things you
  start with (rations, a skill point, a friend at the Watch). The town never gives a class for
  free.
- **The gift at death.** When a life ends with coin, you can leave it to one place. It counts
  toward that place's next level.
- **A word at the end.** When a life ends of old age, one small line under its ending asks for a
  comment or a rating on the game's page. A life cut short is not asked anything.
- **Hover a place** to see what it gives, what its next level asks compared with what you have,
  and who raised it.
- **A notice on a tab.** When something new comes onto the page you are not looking at (a new
  task, something new at the market, a new place), its tab shows a small red count until you
  open it. Only what was never on the page counts: a task you can afford again is not news.
- **The tiles that turn a life stand out.** The four classes and *Explore Bellmoor* have a gold
  frame and their mark in the corner, locked or not.
- **They say what they do.** Hover one of them: it tells you what it changes, with a word from
  the town.

### Changed

- **Buying is done in town.** *Buy rations* and *Buy reagents* are on the Town page, at the
  market. The Life page keeps a *Go to the Market* tile that opens it.
- **Quieter tiles.** A locked tile no longer writes what it needs under its name (an age, a
  stat, coin, room). Its tooltip still says all of it. A tile only says when it is about to
  close.

### Fixed

- Parts of the Town page were sometimes drawn over the Life page, and the other way round, when
  a tile changed on the page you were not looking at.
- A task begun by a double click could not be clicked again until the mouse moved.
- The tutorial could speak of the market in a new chronicle before the town was explored. It
  now comes the first time you open the Town page, on *Buy rations*.

## 0.3 - 2026-10-09

### New

- **A first life opens on one task.** One tile: *Start Helping Out* takes a month, brings you to
  7 and gives you your first rations. The other tasks join the list one at a time
  as you finish terms, and the three classes come with the fourth, when the guide speaks of them. The second life and the ones
  after it open with the whole list, as before.
- **A guide.** Through the first six works a short sentence under the thing it is about says
  what to look at, a pointing hand stands beside it and the rest of the page is shaded. *Next*
  passes a sentence without waiting for it, and *Skip tutorial* ends the guide. To begin a work it
  shows the task first, then the Begin button, and says that a double click on the task begins
  it too. Nothing is blocked:
  you can press anything at any time. It is said once, in the first life only, and a reload
  resumes where it was.
- **Lore in the log.** A few lines about Bellmoor are written in the log as a life goes by.
- **Help in the Kitchen.** A work that feeds you while you are at it and sends you off with
  rations.

### Changed

- **Buy rations is gone from the Life page until the town opens; the kitchen feeds him.**
- The welcome line of a first life is now the guide's first sentence.

## 0.2 - 2026-10-08

Everything since 0.1, the first build published on itch.io (2026-10-07).

### New

- **A guided start.** A new life opens with its first work already chosen: press Begin and the
  months pass. A line in "At work now" says what to do, and "Pass a month" only comes once a
  first work is done.
- **A word of welcome.** A first life is told in one line what the game is.
- **Classes.** Paths are now called classes. The three of them (the Keep, the Collegium and the
  Hidden Hand) stand in the list from the first day, locked, so you can see what a childhood
  leads to.
- **A fourth class: the Renegade.** Miss all three, and the month the last one closes "Take to
  the Greenwood" opens. It has its own works, a proving, a mastery (King of the Greenwood), two
  deeds and its own ending.
- **A Begin button.** Choosing an activity puts a Begin button in "At work now", with the months
  it takes ("Do it now" for what takes no time). A second click or Enter still begins it.
- **See before you choose.** Pointing at an activity shows what it would bring to your stats
  and to what you hold.
- **Gains you can see.** A figure that rises lifts a "+2" beside it, and the line just written
  in the log is lit for a moment.
- **Toasts.** Deeds, milestones and activities that have just become possible are said at the
  foot of the window.
- **Warnings before it hurts.** Rations running low are said from three months left, with what
  to do about it, and a stat the coming month would take from turns red before the month, not
  after.
- **Locked activities say why.** A locked tile names the first thing it still asks.
- **Limits in sight.** What has a limit reads "12/60" in its row.
- **Smaller windows.** The three columns fit windows down to 680 x 566 (1136 x 664 before), and
  the left column scrolls when it is too tall.
- **Plainer names.** The panels are Inventory, Stats and Log.

### Fixed

- A crash ("Segmentation fault") some players met.
- The Begin button always read "3 mo", whatever was chosen.
- The log lost its last lines when a work began.
- The "+1" of a gain stood a line too high when the same month added a row.
- The page could be drawn half placed for a few frames, and new tiles once without their
  ground. A month is also quicker to work out.

## 0.1 - 2026-10-07

The first build on itch.io: one life from 7 to 30 and past it, three paths, the web build.
