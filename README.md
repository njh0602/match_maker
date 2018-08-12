Match Maker for C++
============================

A simple, header-only match maker implementation for C++.

Concept
-------

~~~
                                                                insert    +-goal count: 2-+ 
  [uid: 1] [elo: 1000] [perfer map: 1] [prefer player count: 2] ------->  |     uid:1     |  
                                                                          +---------------+ 
                                                                insert    +-goal count: 2-+  +-goal count: 4-+
  [uid: 2] [elo: 1200] [perfer map: 1] [prefer player count: 4] ------->  |     uid:1     |  |     uid:2     |
                                                                          +---------------+  +---------------+               
                                                                insert    +-goal count: 2-+  +-goal count: 4-+
  [uid: 3] [elo: 900]  [perfer map: 1] [prefer player count: 4] ------->  |     uid:1     |  |     uid:2     |
                                                                          +---------------+  |     uid:3     |
                                                                                             +---------------+
  .
  .   (times over)
  .
  
  +---2---+             +--4--+
  | 2(new)| <--- expand |  2  |    
  | 1     |             |  3  |
  | 3(new)|             +-----+   * queue is sorted by elo
  +-------+
  (uid 3, uid 1) matched! (This is indicated by the callback function.)           
  
  +--2--+  +--4--+
  |  2  |  |  2  |  after match.
  +-----+  +-----+
  
  .
  .  (times over)
  .
                                                                 insert
  [uid: 4] [elo: 1300] [perfer map: 1] [prefer player count: 8] ------->  +--2--+  +--4--+  +--8--+
                                                                          |  2  |  |  2  |  |  4  |
                                                                          +-----+  +-----+  +-----+
  .
  .  (times over)
  .
  
  +--2--+  +---4---+                +--8--+
  |  2  |  | 4(new)|  <--- expand   |  4  |
  +-----+  |   2   |                +-----+
           +-------+
  .
  .  (times over)
  .         
  
  +---2---+               +--4--+ +--8--+
  | 4(new)|  <--- expand  |  4  | |  4  |
  |   2   |               |  2  | +-----+
  +-------+               +-----+
  
  (uid 2, uid 4) matched!
  
  * When the match is over, all queues and player memory are automatically destroyed
  * When the player expands to another queue, it wastes less memory because the pointer is expanded rather than copied.
~~~

## Usage

The implementation is contained in a single header file, `match_maker.hpp`. Simply copy
the file to a convenient place in your project and include it.

~~~cpp
match_maker<id_type, max_map_count, max_player_count> mm(/* callback for match event */);
mm.add_user(/*match hint*/);
mm.remove_user(/*user uid*/);
while ( true )
{
    mm.process_match_making();
}
~~~

See the tests for more example.

Tests
-----

Tests can be run with

~~~
g++ -std=c++11 -Wall -o tests test.cpp
./tests
~~~

## Limitation

- The preference map does not change over the match time.
- The maximum number of players must be a power of 2.
-  `add_user()` and `remove_user()` functions that are called at once do not work normally. (Match requests are usually coming separately. See the test.cpp)
- Not Thread Safe

Contributions
-------------

Contributions are welcome. Please use the Github issue tracker.
