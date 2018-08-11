#include <iostream>
#include <chrono>
#include <random>
using namespace std;
using namespace std::chrono;

#include "match_maker.hpp"
using player_id_type = std::string;
constexpr int max_map_count = 1;
constexpr int max_player_count = 32;
using my_match_maker_type = match_maker<player_id_type, max_map_count, max_player_count>;

#define PRINT_PROGRESS 1

inline int random(int min, int max)
{
    uniform_int_distribution<int> dist(min, max);
    static random_device seed_gen;
    static mt19937 engine(seed_gen());
    return dist(engine);
}

inline int pow2(int n)
{
    int ret = 1;
    for(int i = 0 ; i < n ; ++ i) ret *= 2;
    return ret;
}

int main()
{
    set<player_id_type> matching_users;
    const int total_test_count = 3000;
    int current_test_count = 0;
    int matched_user_count = 0;
    
    my_match_maker_type mm([&](int map, vector<player_id_type> matched_users){
        
        for(const auto& user : matched_users) {matching_users.erase(user); }
        matched_user_count += matched_users.size();
        
#if PRINT_PROGRESS
        printf("~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~\n");
        printf("match complete. map: %d, player count: %d\n", map, static_cast<int>(matched_users.size()));
        printf("matched user's ids: ");
        for(const auto& user : matched_users)
        {
            printf("'%s' ", user.c_str());
        }
        printf("\n");
        printf("matched user count: %d, matching user count: %d\n",
               matched_user_count,
               static_cast<int>(matching_users.size()));
#endif
        if ( matched_user_count == total_test_count )
        {
            printf("test succeed.\n");
            exit(-1);
        }
    });
    
    auto start_time = steady_clock::now();
    while ( true )
    {
        auto curr = steady_clock::now();
        auto interval = duration_cast<milliseconds>(curr-start_time).count();
        if ( interval > 1 && current_test_count < total_test_count  )
        {
            match_hint hint;
            std::string inserted_id;
            static int s_suffix = 0;
            
            pair<bool, int> outcome;
            auto r = random(1, 10);
            if ( r >= 1 && r <= 4 ) // add beginner (40 %)
            {
                hint.elo = 1000 + random(0,499);
                hint.favor_map = 1;
                hint.favor_player_count = pow2(random(1, 5));
                
                inserted_id = "beginner_" + to_string(s_suffix++);
                outcome = mm.add_user(inserted_id, hint);
            }
            else if ( r >= 5 && r <= 8 ) // add intermediate (40 %)
            {
                hint.elo = 1500 + random(0,499);
                hint.favor_map = 1;
                hint.favor_player_count = pow2(random(1, 5));
                
                inserted_id = "intermediate_" + to_string(s_suffix++);
                outcome = mm.add_user(inserted_id, hint);
            }
            else if ( r >= 9 && r <= 10 ) // add expert (20 %)
            {
                hint.elo = 2000 + random(0,499);
                hint.favor_map = 1;
                hint.favor_player_count = pow2(random(1, 5));
                
                inserted_id = "expert_" + to_string(s_suffix++);
                outcome = mm.add_user(inserted_id, hint);
            }
            
            if ( outcome.first )
            {
#if PRINT_PROGRESS
                printf("add user succeed. id: %-20s, elo: %-4d, map: %-2d, favor count: %-2d, expected seconds: %-3d\n",
                       inserted_id.c_str(),
                       hint.elo,
                       hint.favor_map,
                       hint.favor_player_count,
                       outcome.second);
#endif
                
                matching_users.emplace(inserted_id);
                current_test_count++;
            }
            else
            {
                #if PRINT_PROGRESS
                printf("add user failed. error_type: %d\n", outcome.second);
                #endif
            }
            
            start_time = curr;
        }
        
        mm.process_match_making(); // must call every frame
    }
}
