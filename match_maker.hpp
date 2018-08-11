#pragma once

#include <unordered_map>
#include <set>
#include <string>
#include <chrono>
#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>

namespace error_type
{
    enum
    {
        e_no_error = 0,
        e_already_added_user,
        e_invalid_favor_map_type,
        e_invalid_favor_user_count,
        e_unknown_error,
    };
}

struct match_hint
{
    int elo;
    int favor_map;
    int favor_player_count;
};

template <typename id_type>
class match_node
{
    
public:
    
    match_node(const id_type& id, const match_hint& hint) :
    m_id(id),
    m_hint(hint),
    m_origin_hint(hint),
    m_last_update_time(std::chrono::steady_clock::now())
    {}
    ~match_node() = default;
    
    match_node(const match_node&) = default;
    match_node& operator=(const match_node&) = default;
    match_node(match_node&&) = default;
    match_node& operator=(match_node&&) = default;
    
    void id(const id_type& id) { m_id = id; }
	id_type id() const { return m_id; }
    
    void hint(const match_hint& hint) { m_hint = hint; }
    const match_hint& hint() const { return m_hint; }
    match_hint& hint() { return m_hint; }

    void last_update_time(const std::chrono::steady_clock::time_point& t) { m_last_update_time = t; }
    const std::chrono::steady_clock::time_point& last_update_time() const
    {
        return m_last_update_time;
    }
    
    const std::vector<std::vector<match_node<id_type>*>*>& included_queue_list() const
    {
        return m_included_queue_list;
    }
    
    std::vector<std::vector<match_node<id_type>*>*>& included_queue_list()
    {
        return m_included_queue_list;
    }
    
    void add_include_queue(std::vector<match_node<id_type>*>* queue)
    {
        m_included_queue_list.emplace_back(queue);
    }

	friend class viewer;
    
private:
    
	id_type m_id;
    match_hint m_hint;
    match_hint m_origin_hint;
    std::chrono::steady_clock::time_point m_last_update_time;
    std::vector<std::vector<match_node<id_type>*>*> m_included_queue_list;
    
};

template <typename id_type, int max_map_count, int max_player_count>
class match_maker
{
    
public:
    
    static constexpr int k_max_map_count = max_map_count;
    static constexpr int k_max_player_count = max_player_count;
    
    explicit match_maker(std::function<void(int, std::vector<id_type>)> match_callback) :
    m_match_callback(match_callback),
    m_match_update_seconds(5),
    m_matched_player_count(0)
    {
        static_assert(k_max_map_count > 0 , "Must be greater than 0");
        static_assert(k_max_player_count > 0 , "Must be greater than 0");
        static_assert((k_max_player_count & (k_max_player_count - 1)) == 0, "Must be a power of 2");
    }
    ~match_maker() = default;
    
    match_maker(const match_maker&) = delete;
    match_maker& operator=(const match_maker&) = delete;
    match_maker(match_maker&&) = delete;
    match_maker& operator=(match_maker&&) = delete;
    
    std::pair<bool, int> add_user(const id_type& id, const match_hint& hint)
    {
        if ( is_exist_node(id) )
        {
            return {false, error_type::e_already_added_user};
        }
        
        int error_out = 0;
        if ( is_valid_hint(hint, error_out) )
        {
            auto node = new match_node<id_type>(id, hint);
            m_managed_nodes.emplace(id, node);
            insert_to_queue(node);
            return { true, expected_time(hint) };
        }
        
        return { false, error_out };
    }
    
    bool remove_user(const id_type& id)
    {
        if ( !is_exist_node(id) )
            return false;
        
        auto node = m_managed_nodes[id];
        for ( auto& q : node->included_queue_list() )
        {
            q->erase(std::remove_if(std::begin(*q), std::end(*q), [id](match_node<id_type>* n){
                return (n->id() == id);
            }), std::end(*q));
        }
        
        delete node;
        m_managed_nodes.erase(id);
        
        return true;
    }
    
    void process_match_making()
    {
        for ( auto& pair : m_managed_nodes )
        {
            auto node = pair.second;
            auto curr = std::chrono::steady_clock::now();
            auto interval = std::chrono::duration_cast<std::chrono::seconds>(curr - node->last_update_time());
            if ( interval.count() > m_match_update_seconds && is_expandable_node(node) )
            {
                expand_node(node);
            }
        }
        
        if ( !m_dirty_queues.empty() )
        {
            for ( const auto& pair : m_dirty_queues )
            {
                evaluate(pair);
            }
            m_dirty_queues.clear();
        }
    }
    
    void match_update_seconds(uint64_t seconds)
    {
        m_match_update_seconds = seconds;
    }
    
private:
    
    bool is_exist_node(const id_type& id)
    {
        return (m_managed_nodes.count(id) != 0);
    }
    
    bool is_valid_hint(const match_hint& hint, int& error_type)
    {
        if (hint.favor_map < 1 || hint.favor_map > k_max_map_count)
        {
            error_type = error_type::e_invalid_favor_map_type;
            return false;
        }
        
        if (hint.favor_player_count < 1 ||
            hint.favor_player_count > k_max_player_count ||
            (hint.favor_player_count & (hint.favor_player_count - 1)) != 0) // must be power of 2
        {
            error_type = error_type::e_invalid_favor_user_count;
            return false;
        }
        
        error_type = error_type::e_no_error;
        return true;
    }
    
    bool is_expandable_node(match_node<id_type>* node)
    {
        if ( node->hint().favor_player_count > 2 )
            return true;
        return false;
    }
    
    void expand_node(match_node<id_type>* node)
    {
        node->hint().favor_player_count /= 2;
        insert_to_queue(node);
    }
    
    void insert_to_queue(match_node<id_type>* node)
    {
        auto& q = m_queues[node->hint().favor_map][node->hint().favor_player_count];
        auto iter = std::lower_bound(std::begin(q), std::end(q), node, [](match_node<id_type>* n1, match_node<id_type>* n2)
                                     {
                                         return n1->hint().elo < n2->hint().elo;
                                     });
        q.insert(iter, node);
        node->add_include_queue(&q);
        m_dirty_queues.emplace(node->hint().favor_map, node->hint().favor_player_count);
        node->last_update_time(std::chrono::steady_clock::now());
    }
    
    void evaluate(const std::pair<int, int>& index)
    {
        auto i = index.first;  // favor_map
        auto j = index.second; // favor_player count
        while ( m_queues[i][j].size() >= j )
        {
            // match!
            std::vector<id_type> matched_player_ids;
            for ( int k = 0 ; k < j ; ++k )
            {
                matched_player_ids.emplace_back(m_queues[i][j][k]->id());
            }
            
            for (const auto& id : matched_player_ids)
            {
                remove_user(id);
            }
            
            m_matched_player_count += j;
            m_match_callback(i, std::move(matched_player_ids));
        }
    }
    
    int expected_time(const match_hint& hint)
    {
        int result = INT_MAX;
        int weight = 0;
        const int DUMMY_NETWORK_PING = 2;
        const int NEW_PLYER_JOIN_TIME = 1;
        for (int i = hint.favor_player_count; i > 0 ; i /= 2)
        {
            auto left_slot = static_cast<int>(i - m_queues[hint.favor_map][hint.favor_player_count].size());
            auto curr_step = static_cast<int>(left_slot * NEW_PLYER_JOIN_TIME + weight * m_match_update_seconds);
            result = std::min<int>(result, curr_step);
            weight ++;
        }
        return (result + DUMMY_NETWORK_PING);
    }
    
private:
    
    std::set<std::pair<int, int>> m_dirty_queues;
    std::unordered_map<id_type, match_node<id_type>*> m_managed_nodes;
    std::vector<match_node<id_type>*> m_queues[k_max_map_count + 1][k_max_player_count + 1];
    std::function<void(int, std::vector<id_type>)> m_match_callback;
    
    int64_t m_match_update_seconds;
    uint32_t m_matched_player_count;
    
};
