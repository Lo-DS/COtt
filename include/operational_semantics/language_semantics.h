/*
 * language_semantics.h
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

#ifndef OPERATIONAL_SEMANTICS_LANGUAGE_SEMANTICS_H
#define OPERATIONAL_SEMANTICS_LANGUAGE_SEMANTICS_H

#include <operational_semantics/has_equality.h>
#include <operational_semantics/is_hashable.h>
#include <memory>
#include <functional>

template <typename InputType,
        typename TransitionType,
        typename ResultType,
        typename Rec>
using semantics_rule = std::pair<
/**
* Tests whether the rule applies
* @param t     Current input data
* @return      Whether the rule can be applied, or to pursue the next one
*/
std::function<bool(const std::shared_ptr<InputType>& t)>,

/**
* Under the assumption that the test passes, this returns the next expected step
* @param t Step from which start the transition
* @return  Null if the test is not passed, otherwise a vector to smart pointer to the next expected step,
*          alongside a representation of the transition associated to it.
*/
std::function<std::vector<std::pair<TransitionType,std::shared_ptr<ResultType>>>(Rec*, const std::shared_ptr<InputType>& t)>>;



template <typename InputType,
        typename TransitionType,
        typename ResultType>
struct language_semantics : public std::function<std::vector<std::pair<std::string,std::shared_ptr<ResultType>>>(const std::shared_ptr<InputType>&)> {

/**
 * Adding a rule to be evaluated. These are considered by decreasing priority, depending on the insertion order
 * @param f1    Testing condition, for checking whether the current case is applicable to the current rule (preliminary entry-point)
 * @param f2    Expression for determinign the rewriting: if not viable, an empty expression is returend
 */
void add_rule(const std::function<bool(const std::shared_ptr<InputType>& )>& f1,
              //std::vector<std::pair<std::pair<bool,std::string>,std::shared_ptr<finite_ccs>>>
              const std::function<std::vector<std::pair<TransitionType,std::shared_ptr<ResultType>>>(language_semantics<InputType,TransitionType,ResultType>*, const std::shared_ptr<InputType>& t)>& f2) {
rules_by_priority.emplace_back(f1, f2);
}

/**
 * Recursive call for all the rules of the language semantics
 * @param t     Term to be evaluated
 * @return      Resulting expression, with the transition rule being applied if relevant.
 *              If the expression is ill-formed, it is likely that none of the rules can be
 *              applied, and therefore the vector will be empty
 */
std::vector<std::pair<TransitionType,std::shared_ptr<ResultType>>> operator()(const std::shared_ptr<InputType>& t) {
    for (const auto& [test, transform] : rules_by_priority) {
        if (test(t)) {
            return transform(this, t);
        }
    }
    return {};
}

private:
std::vector<semantics_rule<InputType,TransitionType,ResultType,language_semantics<InputType,TransitionType,ResultType>>> rules_by_priority;

};

/**
 * Operation allowing you to print the results of the semantic computation
 * @tparam result
 * @param os
 * @param v
 * @return
 */
template<typename result> std::ostream& operator<<(std::ostream& os, const std::vector<std::pair<std::string,std::shared_ptr<result>>>& v) {
    bool first = true;
    for (const auto& [key, value]: v) {
        if (value) {
            if (first) {
                os << "results:" << std::endl;
                first = false;
            }
            os << "\t - " << key << ": " << *value << std::endl;
        }
    }
    return os;
}

#endif //OPERATIONAL_SEMANTICS_LANGUAGE_SEMANTICS_H
