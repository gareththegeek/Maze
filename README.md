# Maze
A game for the DIY gamer kit

This game was created for the <a href="https://www.techwillsaveus.com/shop/diy-gamer-kit/">DIY Gamer Kit</a>

The game uses Prim's algorithm to randomly generate a maze.  The object of each level is to get to the exit at the bottom right of the maze.  The maze increases in width and height with each successive level up to a maximum of 22x20.  At this point I was hitting the limit of available RAM on the ATMega328 (2k).

The implementation of Prim's algorithm is based upon <a href="http://weblog.jamisbuck.org/2011/1/10/maze-generation-prim-s-algorithm">this blog post by Jamis Buck</a>
