# PROJECT DESCRIPTION

C draft base for an 8-bit overhead Zelda-inspired game. I'm designing this to be (hopefully) straightforward to rewrite in assembly for actual 8-bit machines. The rendering code is targeting Acorn computers from the early 80's, which have a flat-memory model and a character-oriented framebuffer layout in system ram. For the 320x256 monochrome mode I'm using, this is between $5800-$7FFF with addresses being arranged in 8px-by-8px row-major cell layout with each byte a stripe of a cell and each bit a pixel. I'm using an array uint8_t[0x8000] to simulate the memory and dumping the framebuffer out to a window canvas via SDL, but graphics are otherwise done in software using masks and shifts. This is still very much WIP but basic rendering functions are done for now. Assets/graphics were sourced from various 1-bit tilepacks on itch dot io. Test levels were made in Tiled, exported in CSV then compressed and bitpacked to 5-bits using a separate compressor I have written that also changes the tileIDs to fit in the range [0-31], and they are then decompressed as needed. Work is otherwise my own except where noted.

![Screenshot](notes/screenshot.png)


## DEVDIARY NOTES

### 16/10/2025
### Collision Handling: Static Items

Suppose I walk over a pickup, which is a static item. This results in:
- a player inventory update
- background redrawn over the item's tile
- player being redrawn over the background
- the item's roomID being changed to a not-in-any-room value
- the roombuffer being refreshed with a call to loadStatics()
    (collisions are checked against roombuffer; this actually removes the item)


### Rendering Optimisations

#### UNSORTED ROOMBUFFER
- suppose the REDRAW_STATICS flag is raised on the camera.
- suppose I have 1 static that needs to be redrawn, but this is not known A.O.T
- suppose roombuffer has capacity for 14 items, but currently holds 7 items plus
    a sentinel value to stop early.
- without sorting I must visit all items in roombuffer preceding the sentinel.
- on each item visited in roombuffer preceding sentinel:
    - 1 sentinel check
	- 1 type check
	- if type==static, then redraw flag check

Worst case: 7 sentinels, 7 type checks, 1 flag check + end sentinel = 16 checks

#### CATEGORISED ROOMBUFFER
- suppose the REDRAW_STATICS flag is raised on the camera.
- suppose I have 1 static that needs to be redrawn, but this is not known A.O.T
- suppose statics section of roombuffer has capacity 10 items, but holds 1 item 
    and a sentinel to stop early
- I only visit statics in the roombuffer to check their redraw flags. These are 
    presorted, so no type checks needed.
- on each item visited in the statics section preceding sentinel:
	- 1 sentinel check
	- 0 type checks
	- 1 redraw flag check
	
Worst case: 1 sentinel check, 1 redraw check, 1 final sentinel check = 3 checks

If I do opt to use a renderqueue, then this doesn't really change much. I'll
still need a sentinel to stop early due to fixed size. If I don't keep render 
items sorted then I also need to check times as I visit each item in the queue.

---

### 07/10/2025
### Streaks

When I move quickly (eg by 2+ pixels per frame) sometimes a trail gets left 
behind. I think this is because my character moves to a new sprite container 
without clearing the old one. Perhaps I need to implement an erase when my 
character is running.

I already track the direction of travel. Perhaps I can implement only redrawing 
those background tiles prior to the sprite container update. When I perform the 
sprite container update, I could check for when the hshift or vshift wraps 
around and also delete those cells related to the player direction.

---

### 30/09/2025
### Animation Problem

- Option #1: store pointers to animation sets directly on each movable / player 
with common label e.g. ME_UP, ME_DOWN, etc. this will increase the size of each 
movable a lot as more animation sets are added, so probably wont scale well

- Option #2: store an absolute offset from animdefs start on each movable / player. 
this is essentially the same as storing a pointer on each object but you save a 
byte.

- Option #3: store a base + relative offset on each movable / player without a 
pointer table.  this requires the various animation sets to be laid out in a 
common order AND size. 
(eg PLAYER U 3 frames, D 3 frames ... ENEMY U 3 frames, D 3 frames ...)

- Option #4: store a base + relative offset on each movable / player with a 
pointer table. this requires animation sets to share a common order, but size 
can be variable. 
(eg PLAYER U 3 frames, D 3 frames ... ENEMY U 2 frames, D 2 frames ... )

I will be going with option 4 (for now).