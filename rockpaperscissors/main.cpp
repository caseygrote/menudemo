/*
 * Sifteo Rock Paper Scissors game written by Jim Surine in 2012 based on code from the Sifteo SDK.
 */

#include <sifteo.h>
#include <sifteo/menu.h>
#include <sifteo/math.h>
#include "assets.gen.h"
using namespace Sifteo;

// Static Globals
static VideoBuffer gVideo[CUBE_ALLOCATION];
static struct MenuEvent e[CUBE_ALLOCATION];
static Menu m[CUBE_ALLOCATION];
static struct MenuItem gItems[] = { {&IconRock, &LabelRock}, {&IconPaper, &LabelPaper}, {&IconScissors, &LabelScissors}, {NULL, NULL} };
static struct MenuAssets gAssets = {&BgTile, &Footer, &LabelEmpty, {&Tip1, &Tip2, NULL}};
Random r;
static uint8_t cubetouched;

static uint8_t cubeselected[CUBE_ALLOCATION];
static unsigned cubescore[CUBE_ALLOCATION];
static unsigned gNumCubesConnected = CubeSet::connected().count();

static AssetSlot MainSlot = AssetSlot::allocate()
    .bootstrap(BetterflowAssets);

static Metadata M = Metadata()
    .title("Rock Paper Scissors")
    .package("com.sifteo.sdk.rockpaperscissors", "1.0.0")
    .icon(Icon)
    .cubeRange(2, CUBE_ALLOCATION);

// consts
const uint8_t ROCK_SELECTED = 0;
const uint8_t PAPER_SELECTED = 1;
const uint8_t SCISSORS_SELECTED = 2;
const uint8_t NOT_SELECTED = 255;

// ideas for future
// todo: number of player selectable
// todo: number of rounds selectable
// todo: select different versions of game (simple 3, expert 5-6, ultimate 16-20)
// todo: start game
// todo: handle cube disconnects and reconnects
// todo: allow players to choose who goes first or let system decide by who is next
// todo: game play round # display
// todo: sounds, menu click, select, cover, uncover, battle, win, loss, tie, voice of battle results, background music, different during different play modes?
// todo: end of rounds - player winner display, loser display, tie dispaly
// todo: player colors? all players scores on cubes during selection?
// todo: double bonus rounds 2x 3x 4x scoring possibilities
// todo: variety mode with different version of the game played randomly (championship game?)
// todo: battle computer, use AI research info, allow multiple computer players, high scores against the computer, enter initials?
// todo: make my own menus system that uses a 2d grid selection system instead of 1d scrolling list.
// todo: create battle animations one one cube or across two cubes

static void begin()
{
    // Blank screens, attach VideoBuffers
    for(CubeID cube : CubeSet::connected())
    {
    	LOG("cube  %d\n", (int)cube);
        auto &vid = gVideo[cube];
        vid.initMode(BG0);
        vid.attach(cube);
        vid.bg0.erase(StripeTile);

    }
}

uint8_t doit(Menu &m, struct MenuEvent &e)
{
	uint8_t saveditem = NOT_SELECTED;

	if (m.pollEvent(&e))
	{
		switch (e.type)
		{
			case MENU_ITEM_PRESS:
				m.anchor(e.item);
				break;

			case MENU_EXIT:
				// this is not possible when pollEvent is used as the condition to the while loop.
				// NOTE: this event should never have its default handler skipped.
				ASSERT(false);
				break;

			case MENU_NEIGHBOR_ADD:
				LOG("found cube %d on side %d of menu (neighbor's %d side)\n",
					 e.neighbor.neighbor, e.neighbor.masterSide, e.neighbor.neighborSide);
				break;

			case MENU_NEIGHBOR_REMOVE:
				LOG("lost cube %d on side %d of menu (neighbor's %d side)\n",
					 e.neighbor.neighbor, e.neighbor.masterSide, e.neighbor.neighborSide);
				break;

			case MENU_ITEM_ARRIVE:
				LOG("arriving at menu item %d\n", e.item);
				saveditem = e.item;
				break;

			case MENU_ITEM_DEPART:
				LOG("departing from menu item %d, scrolling %s\n", saveditem, e.direction > 0 ? "forward" : "backward");
				break;

			case MENU_PREPAINT:
				// do your implementation-specific drawing here
				// NOTE: this event should never have its default handler skipped.
				break;

			case MENU_UNEVENTFUL:
				// this should never happen. if it does, it can/should be ignored.
				ASSERT(false);
				break;
		}

		m.performDefault();
		return NOT_SELECTED;
	}
	else
	{
		// Handle the exit event (so we can re-enter the same Menu)
		ASSERT(e.type == MENU_EXIT);
		m.performDefault();

		LOG("Selected Game: %d\n", e.item);
		return e.item;
	}
}

void onNeighborAdd(void * x, unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide)
{
	LOG("Neighbor Add: %02x:%d - %02x:%d\n", firstID, firstSide, secondID, secondSide);
	if ((cubeselected[firstID] == ROCK_SELECTED) && (cubeselected[secondID] == PAPER_SELECTED))
	{
		cubeselected[firstID] = NOT_SELECTED;
		gVideo[firstID].bg0.text(vec(0,7), Font, "Loser");
		gVideo[firstID].bg0.text(vec(0,9), Font, "Paper wraps");
		gVideo[firstID].bg0.text(vec(0,11), Font, "rock.");
	}
	if ((cubeselected[firstID] == PAPER_SELECTED) && (cubeselected[secondID] == SCISSORS_SELECTED))
	{
		cubeselected[firstID] = NOT_SELECTED;
		gVideo[firstID].bg0.text(vec(0,7), Font, "Loser");
		gVideo[firstID].bg0.text(vec(0,9), Font, "Scissors cuts");
		gVideo[firstID].bg0.text(vec(0,11), Font, "paper.");
	}
	if ((cubeselected[firstID] == SCISSORS_SELECTED) && (cubeselected[secondID] == ROCK_SELECTED))
	{
		cubeselected[firstID] = NOT_SELECTED;
		gVideo[firstID].bg0.text(vec(0,7), Font, "Loser");
		gVideo[firstID].bg0.text(vec(0,9), Font, "Rock dulls");
		gVideo[firstID].bg0.text(vec(0,11), Font, "scissors.");
	}
	if ((cubeselected[secondID] == ROCK_SELECTED) && (cubeselected[firstID] == PAPER_SELECTED))
	{
		cubeselected[secondID] = NOT_SELECTED;
		gVideo[secondID].bg0.text(vec(0,7), Font, "Loser");
		gVideo[secondID].bg0.text(vec(0,9), Font, "Paper wraps");
		gVideo[secondID].bg0.text(vec(0,11), Font, "rock.");
	}
	if ((cubeselected[secondID] == PAPER_SELECTED) && (cubeselected[firstID] == SCISSORS_SELECTED))
	{
		cubeselected[secondID] = NOT_SELECTED;
		gVideo[secondID].bg0.text(vec(0,7), Font, "Loser");
		gVideo[secondID].bg0.text(vec(0,9), Font, "Scissors cuts");
		gVideo[secondID].bg0.text(vec(0,11), Font, "paper.");
	}
	if ((cubeselected[secondID] == SCISSORS_SELECTED) && (cubeselected[firstID] == ROCK_SELECTED))
	{
		cubeselected[secondID] = NOT_SELECTED;
		gVideo[secondID].bg0.text(vec(0,7), Font, "Loser");
		gVideo[secondID].bg0.text(vec(0,9), Font, "Rock dulls");
		gVideo[secondID].bg0.text(vec(0,11), Font, "scissors.");
	}
	if ((cubeselected[firstID] == cubeselected[secondID]) && (cubeselected[firstID] != NOT_SELECTED))
	{
		gVideo[firstID].bg0.text(vec(0,7), Font, "Tie");
		gVideo[secondID].bg0.text(vec(0,7), Font, "Tie");
	}
}

void onTouch(void *, unsigned id)
{
    CubeID cube(id);
    if (cube.isTouching() == true)
    {
    	// we only want touches not un-touches to trigger
    	cubetouched = id;
    }
}

void main()
{
	int i;
	uint8_t winner;

	begin();

	LOG("cubes allocated %d\n", gNumCubesConnected);

	// initialize menus
	for (i = 0; i < gNumCubesConnected; i++)
	{
		m[i].init(gVideo[i], &gAssets, gItems);
		// random starting menu item
		m[i].anchor(r.raw() % 3);
		// init to nothing selected
		cubeselected[i] = NOT_SELECTED;
		// init scores to zero
		cubescore[i] = 0;
	}

    // attach event handler
    Events::neighborAdd.set(&onNeighborAdd);
    Events::cubeTouch.set(&onTouch);

    // game lasts forever
    while (true)
    {
    	// handle menus
    	for (i = 0; i < gNumCubesConnected; i++)
    	{
			if (cubeselected[i] == NOT_SELECTED)
			{
				cubeselected[i] = doit(m[i],e[i]);
			}
    	}

		// is anyone still not selected?
		for (i = 0; i < gNumCubesConnected; i++)
		{
			if (cubeselected[i] == NOT_SELECTED)
			{
				break;
			}
		}

		// player are still selecting, wait longer
		if (i < gNumCubesConnected)
		{
			continue;
		}

		// search for winners
		winner = NOT_SELECTED;
		while (winner == NOT_SELECTED)
		{
			System::paint();

			// find a possible winner
			for (i = 0; i < gNumCubesConnected; i++)
			{
				if (cubeselected[i] != NOT_SELECTED)
				{
					winner = cubeselected[i];
					break;
				}
			}

			// look for other different possible winner
			for (i = 0; i < gNumCubesConnected; i++)
			{
				if ((cubeselected[i] != NOT_SELECTED) && (cubeselected[i] != winner))
				{
					winner = NOT_SELECTED;
					break;
				}
			}
		}

		// find all the winners and give them a point
		for (i = 0; i < gNumCubesConnected; i++)
		{
			if (cubeselected[i] == winner)
			{
				cubescore[i]++;
				gVideo[i].bg0.text(vec(0,7), Font, "Winner");
			    LOG("Winner %d", i);
			}
		}

		// wait for someone to touch a cube
		cubetouched = NOT_SELECTED;
		while (cubetouched == NOT_SELECTED)
		{
			System::paint();
		}

		// show everyones score
		for (i = 0; i < gNumCubesConnected; i++)
		{
		    String<20> str;
		    str << "Score: " << cubescore[i];
			gVideo[i].bg0.text(vec(0,7), Font, str);
		}

		// wait for someone to touch a cube
		cubetouched = NOT_SELECTED;
		while (cubetouched == NOT_SELECTED)
		{
			System::paint();
		}

		// start another round
		for (i = 0; i < gNumCubesConnected; i++)
		{
			cubeselected[i] = NOT_SELECTED;
		}
    }
}
