/*
 * finite_ccs.cpp
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

#include <operational_semantics/small_step_semantics.h>
#include <string>

/**
 * Defining all the inductive cases for finite CCS
 */
enum finite_ccs_process_cases {
    NIL = 0,
    MultiPrefix,
    ParallelComposition,
    Restriction
};

/**
 * Structure for englobing all the inductive cases
 */
struct finite_ccs {
    finite_ccs_process_cases casus;
    std::vector<std::string> restr_label;
    std::vector<std::shared_ptr<finite_ccs>> parallel_compose;
    std::vector<std::pair<std::pair<bool,std::string>,std::shared_ptr<finite_ccs>>> multi_prefix;

    finite_ccs() : casus{NIL} {}
    finite_ccs(const finite_ccs& ) = default;
    finite_ccs(finite_ccs&& ) = default;
    finite_ccs& operator=(const finite_ccs& ) = default;
    finite_ccs& operator=(finite_ccs&& ) = default;
    finite_ccs(std::vector<std::shared_ptr<finite_ccs>> v) : casus{ParallelComposition}, parallel_compose(std::move(v)) {

    }

    finite_ccs(std::vector<std::pair<std::pair<bool,std::string>,std::shared_ptr<finite_ccs>>> ls) : casus{MultiPrefix} {
        multi_prefix.insert(multi_prefix.begin(), ls.begin(), ls.end());
    }

    finite_ccs(const std::vector<std::string>& label, std::shared_ptr<finite_ccs> lhs) : casus{Restriction}, restr_label{label} {
        parallel_compose.emplace_back(std::move(lhs));
    }

    bool operator==(const finite_ccs &rhs) const;

    bool operator!=(const finite_ccs &rhs) const {
        return !(rhs == *this);
    }


};

namespace std {
    // Making vector of strings hashable
    template <> struct hash<std::vector<std::string>> {
        size_t operator()(const std::vector<std::string>& v) const {
            size_t compose = 31;
            std::hash<std::string> H;
            for (const auto& k : v)
                compose += H(k) * 7;
            return compose;
        }
    };

    // Making pairs hashable
    template <typename K, typename V> struct hash<std::pair<K, V>> {
        size_t operator()(const std::pair<K, V>& v) const {
            std::hash<K> hK;
            std::hash<V> hV;
            return hK(v.first) ^ hV(v.second);
        }
    };

    // Making ccs formulae hashable
    template <> struct hash<finite_ccs> {
        size_t operator()(const finite_ccs& x) const {
            switch (x.casus) {
                case NIL:
                    return 1;
                case MultiPrefix: {
                    size_t base_case = 13;
                    for (const auto& [k, v] : x.multi_prefix) {
                        base_case += ((((std::hash<std::string>()(k.second)) * 2 + (k.first ? 1 : 0))) ^ operator()(*v))*7;
                    }
                    return base_case*8+4;
                }
                case ParallelComposition:
                    return ((operator()(*x.parallel_compose[0])) ^ (operator()(*x.parallel_compose[1])))*8+3;
                case Restriction:
                    return ((std::hash<std::vector<std::string>>()(x.restr_label) ^ operator()(*x.parallel_compose[0])) * 8) + 2;
            }
            return 0;
        }
    };
}

#include <map>
#include <set>

/**
 * Implementing CCS structural equality
 * @param rhs
 * @return
 */
bool finite_ccs::operator==(const finite_ccs &rhs) const {
    KeyEqualizer<finite_ccs> ke;
    if (casus != rhs.casus)
        return false;
    switch (casus) {
        case NIL:
            return true;
        case MultiPrefix: {
            std::set<std::pair<bool,std::string>> LS, RS;
            std::map<std::pair<bool,std::string>,
                    transition_node_set<finite_ccs>> multimapLHS, multimapRHS;
            for (const auto& [k,v] : multi_prefix) {
                LS.insert(k);
                multimapLHS[k].emplace(v);
            }
            for (const auto& [k,v] : rhs.multi_prefix) {
                RS.insert(k);
                multimapRHS[k].emplace(v);
            }
            if (LS != RS)
                return false;
            for (const auto& k : LS) {
                if (multimapLHS[k] != multimapRHS[k])
                    return false;
            }
            return true;
        } break;
        case ParallelComposition: {
            if (!(ke(parallel_compose[0], rhs.parallel_compose[0])))
                return false;
            return (ke(parallel_compose[1], rhs.parallel_compose[1]));
        }
        case Restriction: {
            if (restr_label != rhs.restr_label)
                return false;
            return (ke(parallel_compose[0], rhs.parallel_compose[0]));
        }
    }
    return false;
}

int main() {
    std::string tau = ".";
    std::pair<bool,std::string> tauPair{false, tau};


    small_step_semantics<finite_ccs, std::pair<bool,std::string>> finiteCCS_graph_Semantics;

    // MultiPrefix rule: reducing the expression to the others to be returned
    finiteCCS_graph_Semantics.add_rule([](const std::shared_ptr<finite_ccs>& op) {
        return (op) && op->casus == MultiPrefix && (!op->multi_prefix.empty());
    }, [](language_semantics<finite_ccs, std::pair<bool,std::string>, finite_ccs>* rec, const std::shared_ptr<finite_ccs>& op) {
        return op->multi_prefix;
    });

    // Parallel Composition rule, for which we expand only one of the arguments at a time, and output the resulting transitions
    finiteCCS_graph_Semantics.add_rule([](const std::shared_ptr<finite_ccs>& op) {
        return (op) && op->casus == ParallelComposition && (!op->parallel_compose.empty());
    }, [tau,tauPair](language_semantics<finite_ccs, std::pair<bool,std::string>, finite_ccs>* rec, const std::shared_ptr<finite_ccs>& op) {
        std::unordered_map<std::string, std::pair<std::vector<std::pair<size_t, std::shared_ptr<finite_ccs>>>, std::vector<std::pair<size_t, std::shared_ptr<finite_ccs>>>>> map;
        std::vector<std::pair<std::pair<bool,std::string>,std::shared_ptr<finite_ccs>>> result;
        for (size_t i = 0, N = op->parallel_compose.size(); i<N; i++) {
            std::vector<std::pair<std::pair<bool,std::string>, std::shared_ptr<finite_ccs>>> par
                = rec->operator()(op->parallel_compose[i]);
            for (const auto& [key,val] : par) {
                if (key.second != tau) {
                    if (key.first) {
                        map[key.second].first.emplace_back(i, val);
                    } else {
                        map[key.second].second.emplace_back(i, val);
                    }
                }
                auto current = std::make_shared<finite_ccs>(*op);
                current->parallel_compose[i] = val;
                result.emplace_back(key, current);
            }
        }
        for (const auto& x : map) {
            for (auto i = std::begin(x.second.first); i != std::end(x.second.first); i++) {
                for (auto j = std::begin(x.second.second); j != std::end(x.second.second); j++) {
                    if (i->first != j->first) {
                        auto current = std::make_shared<finite_ccs>(*op);
                        current->parallel_compose[i->first] = i->second;
                        current->parallel_compose[j->first] = j->second;
                        result.emplace_back(tauPair, current);
                    }
                }
            }
        }
        return result;
    });

    // Restriction: removing as viable transitions all the ones that appear within the set of forbidden rules.
    // This is to force synchronisation between processes sharing the same signed-unsigned elements
    finiteCCS_graph_Semantics.add_rule([](const std::shared_ptr<finite_ccs>& op) {
        return (op) && op->casus == Restriction && (op->parallel_compose.size() == 1) && (!op->restr_label.empty());
    }, [tau,tauPair](language_semantics<finite_ccs, std::pair<bool,std::string>, finite_ccs>* rec, const std::shared_ptr<finite_ccs>& op) {
        std::unordered_map<std::string, std::pair<std::vector<std::pair<size_t, std::shared_ptr<finite_ccs>>>, std::vector<std::pair<size_t, std::shared_ptr<finite_ccs>>>>> map;
        std::vector<std::pair<std::pair<bool,std::string>,std::shared_ptr<finite_ccs>>> result;
        std::unordered_set<std::string> S{op->restr_label.begin(), op->restr_label.end()};
        std::vector<std::pair<std::pair<bool,std::string>, std::shared_ptr<finite_ccs>>> par
                = rec->operator()(op->parallel_compose[0]);
        par.erase(std::remove_if(par.begin(), par.end(), [S](const std::pair<std::pair<bool,std::string>, std::shared_ptr<finite_ccs>>& x) {
            return S.contains(x.first.second);
        }), par.end());
        for (const auto& ref : par) {
            auto current = std::make_shared<finite_ccs>(*op);
            current->parallel_compose[0] = ref.second;
            result.emplace_back(ref.first, current);
        }
        return result;
    });

    // deadlock
    auto nil = std::make_shared<finite_ccs>();
    // Some typing to make our life easier
    using multialt_cp = std::pair<std::pair<bool,std::string>,std::shared_ptr<finite_ccs>>;
    using multialt = std::vector<std::pair<std::pair<bool,std::string>,std::shared_ptr<finite_ccs>>>;
    using multiparall = std::vector<std::shared_ptr<finite_ccs>>;
    // Untagged label a
    std::pair<bool,std::string> a{false, "a"};
    // Untagged label b
    std::pair<bool,std::string> b{false, "b"};
    multialt_cp a_nil_cp{a, nil};
    multialt_cp b_nil_cp{b, nil};
    // Representing a.0
    auto a_nil = std::make_shared<finite_ccs>(multialt{a_nil_cp});
    // Representing b.0
    auto b_nil = std::make_shared<finite_ccs>(multialt{b_nil_cp});
    multialt_cp ab_nil_cp{a, b_nil};
    multialt_cp ba_nil_cp{b, a_nil};
    // Representing a.b.0 + b.a.0
    auto abnil_banil = std::make_shared<finite_ccs>(multialt{ab_nil_cp, ba_nil_cp});
    // Representing a.0 | b.0
    auto anil_parall_bnil = std::make_shared<finite_ccs>(multiparall{a_nil, b_nil});

    // Generating the graph for one of the two configurations
    finiteCCS_graph_Semantics.visit(abnil_banil);

    return 0;
}
