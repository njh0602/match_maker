Match Maker for C++
============================

A simple, header-only match maker implementation for C++.

Concept
-------

~~~
                                                                      +--2--+ 
  [uid: 1] [elo: 1000] [perfer map: 1] [prefer player count: 2] --->  |  1  |  
                                                                      +-----+ 
                                                                      +--2--+  +--4--+
  [uid: 2] [elo: 1200] [perfer map: 1] [prefer player count: 4] --->  |  1  |  |  2  |
                                                                      +-----+  +-----+
  .
  .   (times over)
  .
  
  +--2--+               +--4--+
  |  2  | <--- expand   |  2  |    
  |  1  |               +-----+
  +-----+                        * queue is sorted by elo
  
  matched! (This is indicated by the callback function.)           
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