/*
 * Sifteo SDK Example.
 */

#include <sifteo.h>
#include <sifteo/menu.h>
#include "assets.gen.h"
using namespace Sifteo;

// Static Globals
static const unsigned gNumCubes = 3;
static VideoBuffer gVideo[gNumCubes];
//static struct MenuItem gItems[] = { {&IconSBOL, &LabelChroma}, {&IconPromoter, &LabelSandwich}, {&IconPeano, &LabelPeano}, {&IconBuddy, &LabelBuddy}, {&IconPromoter, NULL}, {NULL, NULL} };
static struct MenuItem gItems[] = { { &IconSBOL, &LabelChroma }, { &IconPromoter, &LabelSandwich }, { &IconPromoter, NULL }, { NULL, NULL } };
static struct MenuAssets gAssets = {&BgTile, &Footer, &LabelEmpty, {&Tip0, &Tip1, &Tip2, NULL}};

static AssetSlot MainSlot = AssetSlot::allocate()
    .bootstrap(BetterflowAssets);

static Metadata M = Metadata()
    .title("Menu SDK Demo")
    .package("com.sifteo.sdk.menudemo", "1.0.0")
    .icon(Icon)
    .cubeRange(gNumCubes);


static void begin() {
    // Blank screens, attach VideoBuffers
    for(CubeID cube = 0; cube != gNumCubes; ++cube) {
        auto &vid = gVideo[cube]; //initializes all cubes to striped background @ev
        vid.initMode(BG0);
        vid.attach(cube);
        vid.bg0.erase(StripeTile);
    }

}

static void nextCube(VideoBuffer thisCube, MenuEvent ev){ 
	if (ev.neighbor.neighborSide == 0){ //how do you convert between neighbors and cubes? @ev
		Neighborhood attached = thisCube.physicalNeighbors();
		
		CubeID cube = gVideo[1].cube();
		gVideo[1].initMode(BG0);
		gVideo[1].bg0.erase(BgTile);
		LOG("it's happening again\n");
	}
}

void main()
{
    begin();

    Menu m(gVideo[0], &gAssets, gItems); //gVideo[x] initializes the x'th cube to the menu screen @ev
    m.anchor(0);

    struct MenuEvent e;
    uint8_t item;

    while (1) {
        while (m.pollEvent(&e)) {

            switch (e.type) {

                case MENU_ITEM_PRESS:
                    // Game Buddy is not clickable, so don't do anything on press
                    if (e.item >= 3) {
                        // Prevent the default action
						LOG("menu item not clickable");
                        continue;
                    } else {
                        m.anchor(e.item);
                    }
                    if (e.item == 4) {
                        static unsigned randomIcon = 0;
                        randomIcon = (randomIcon + 1) % e.item;
                        m.replaceIcon(e.item, gItems[randomIcon].icon, gItems[randomIcon].label);
                    }
                    break;

                case MENU_EXIT:
                    // this is not possible when pollEvent is used as the condition to the while loop.
                    // NOTE: this event should never have its default handler skipped.
                    ASSERT(false);
                    break;

                case MENU_NEIGHBOR_ADD:
                    LOG("found cube %d on side %d of menu (neighbor's %d side)\n",
                         e.neighbor.neighbor, e.neighbor.masterSide, e.neighbor.neighborSide);
					nextCube(gVideo[0], e); //aux method for neighbor attaching @ev
                    break;

                case MENU_NEIGHBOR_REMOVE:
                    LOG("lost cube %d on side %d of menu (neighbor's %d side)\n",
                         e.neighbor.neighbor, e.neighbor.masterSide, e.neighbor.neighborSide);
					if (e.neighbor.neighborSide == 0){ //will move this to aux. method later @ev
						gVideo[1].initMode(BG0);
						gVideo[1].bg0.erase(StripeTile);
					}
                    break;

                case MENU_ITEM_ARRIVE:
                    LOG("arriving at menu item %d\n", e.item);
                    item = e.item;
                    break;

                case MENU_ITEM_DEPART:
                    //LOG("departing from menu item %d, scrolling %s\n", item, e.direction > 0 ? "forward" : "backward");
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
        }

        // Handle the exit event (so we can re-enter the same Menu)
        ASSERT(e.type == MENU_EXIT);
        m.performDefault();

        LOG("Selected Game: %d\n", e.item);
    }
}
