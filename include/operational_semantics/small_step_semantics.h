/*
 * small_step_semantics.h
 * This file is part of COtt
 *
 * Copyright (C) 2024 - Giacomo Bergami
 *
 * COtt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * COtt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with COtt. If not, see <http://www.gnu.org/licenses/>.
 */



//
// Created by giacomo on 24/06/24.
//

#ifndef COTT_SMALL_STEP_SEMANTICS_H
#define COTT_SMALL_STEP_SEMANTICS_H

#include <operational_semantics/language_semantics.h>

/**
 * Default hasher for a smart-pointer, by exploiting the hash value associated to the pointed object
 * @tparam Key
 */
template <typename Key>
struct KeyHasher
{
    static_assert(is_std_hashable_v<Key>, "Error: the key should be hashable");

    std::size_t operator()(const std::shared_ptr<Key>& k) const
    {
        using std::size_t;
        using std::hash;
        using std::string;
        if (!k) {
            return 0;
        } else {
            return hash<Key>()(*k.get()) * 2 +1;
        }
    }
};

/**
 * Default smart pointer comparator, also encompassing whether the pointer is pointing to null
 * @tparam Key
 */
template <typename Key>
struct KeyEqualizer
{
    static_assert(CHECK::EqualExists<Key>::value, "Error: the key should come with a default equality predicate");

    bool operator()(const std::shared_ptr<Key>& __x, const std::shared_ptr<Key>& __y) const
    { return ((((bool) (__x)) == ((bool) (__y)))) && ((!(((bool) (__x)))) || *__x == *__y); }

};

#include <unordered_map>
#include <unordered_set>
#include <stack>

template <typename TransitionNode>
using transition_node_set =  std::unordered_set<std::shared_ptr<TransitionNode>,
        KeyHasher<TransitionNode>,
        KeyEqualizer<TransitionNode>>;

/**
 * A small-step semantics is a specific case of a language semantics, where the returned type
 * matches with the input type. Furthermore, as we want to generate a transition graph from the
 * expression while evaluating it, we want to provide a forward graph representation of the same
 * expression, thus making it available for any further static code analysis. Thus, we are always
 * interpreting an expression in terms of its associated graph
 *
 * @tparam TransitionNode
 * @tparam TransitionLabel
 */
template <typename TransitionNode,
        typename TransitionLabel>
struct small_step_semantics : public language_semantics<TransitionNode,TransitionLabel,TransitionNode> {
    static_assert(is_std_hashable_v<TransitionNode>, "Error: the node type should be hashable, so to fill in a transition graph from the constituents");
    static_assert(CHECK::EqualExists<TransitionNode>::value, "Error: the node type should have an equivalence operator associated to it, otherwise, we cannot determine the node equivalence for generating a transition graph");
    static_assert(is_std_hashable_v<TransitionLabel>, "Error: the transition label type should be hashable, so to guarantee the label unicity");
    static_assert(CHECK::EqualExists<TransitionLabel>::value, "Error: the transition label type should have an equivalence operator associated to it, otherwise, we cannot determine the label equivalence for generating a unique transition label");


    std::unordered_map<std::shared_ptr<TransitionNode>,
            std::unordered_map<TransitionLabel, transition_node_set<TransitionNode>>,
            KeyHasher<TransitionNode>,
            KeyEqualizer<TransitionNode>>

            forward_transition_graph;

    transition_node_set<TransitionNode> visited_nodes;


    void visit(const std::shared_ptr<TransitionNode>& start) {
        // Clearing from previous computations
        visited_nodes.clear();
        forward_transition_graph.clear();
        // Starting the recursive view over the language
        std::stack<std::shared_ptr<TransitionNode>> S;
        S.emplace(start);
        while (!S.empty()) {
            auto top = S.top();
            S.pop();
            if (visited_nodes.contains(top)) continue; // Visiting the expression only if we haven't done it yet
            visited_nodes.emplace(top); // Adding this to the visited nodesz
            // Using the parent's associated expression for evaluating the next steps to be computed
            // This is why the returned type has to be the same of the input type, otherwise we cannot
            // engage with a recursive call.
            std::vector<std::pair<TransitionLabel,std::shared_ptr<TransitionNode>>> result = this->operator()(top);
            for (const auto& [label, dst] : result) {
                auto& adjList = forward_transition_graph[top];
                auto& dstPlace = adjList[label];
                dstPlace.emplace(dst); // Adding this to the graph
                S.emplace(dst); // Adding this to the recursive view, only if we haven't visited it yet!
            }
        }
    }

};

#endif //COTT_SMALL_STEP_SEMANTICS_H
