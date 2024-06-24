/*
 * uint_arithmetics.cpp
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

#include <operational_semantics.h>

/**
 * Defining all the inductive cases for uint expressions
 */
enum op_type {
    PLUS,
    MINUS,
    DIV,
    TIMES,
    VAL,
    EXPR
};

struct num_op {
    size_t val; // Base case value
    std::shared_ptr<num_op> left, right; // Operands
    op_type casus; // Inductive case

    /**
     * Expression printing
     * @param os    Stream
     * @param op    Expression
     * @return
     */
    friend std::ostream &operator<<(std::ostream &os, const num_op &op) {
        switch (op.casus) {
            case EXPR:
                return os << "(" << *op.left << ")";
            case VAL:
                return os << op.val;
            case PLUS:
                return os <<"(" << *op.left << ") + (" << *op.right << ")";
            case MINUS:
                return os <<"(" << *op.left << ") - (" << *op.right << ")";
            case TIMES:
                return os <<"(" << *op.left << ") * (" << *op.right << ")";
            case DIV:
                return os <<"(" << *op.left << ") / (" << *op.right << ")";
        }
        return os;
    }

    /**
     * Creating a base case number
     * @param val   Number
     */
    num_op(size_t val) : val{val}, left{nullptr}, right{nullptr}, casus{VAL} {}

    /**
     * Mimicking the (...) for an expression
     * @param arg   Expression
     */
    num_op(std::shared_ptr<num_op> arg) : left{std::move(arg)},
                                          right{nullptr},
                                          casus{EXPR} {}

    /**
     * Constructor for any binary numeric operator
     * @param left      Left operand
     * @param op        Opeator
     * @param right     Right operand
     */
    num_op(std::shared_ptr<num_op> left,
           op_type op,
           std::shared_ptr<num_op> right) : left{std::move(left)},
                                            right{std::move(right)},
                                            casus{op} {}

    /**
     * Checking whether the two expressions are syntactically equivalent
     * @param rhs
     * @return
     */
    bool operator==(const num_op &rhs) const {
        return val == rhs.val &&
               ((((bool)left) == ((bool)rhs.left)) && ((bool)left)) ? (*left == *rhs.left) : false &&
                                                                                             ((((bool)right) == ((bool)rhs.right)) && ((bool)right)) ? (*right == *rhs.right) : false&&
                                                                                                                                                                                casus == rhs.casus;
    }

    /**
     * Syntactic inequivalence
     * @param rhs
     * @return
     */
    bool operator!=(const num_op &rhs) const {
        return !(rhs == *this);
    }
};

namespace std {
    /**
     * Expression hashability
     */
    template <> struct hash<num_op> {
        size_t operator()(const struct num_op& x) const {
            size_t lh = (x.left) ? operator()(*x.left) *2+1 : 0;
            size_t rh = (x.right) ? operator()(*x.right) *2+1 : 0;
            size_t ops = lh ^ rh;
            return ((size_t)x.casus) ^ (x.val) ^ ops;
        }
    };
}




int main() {
    language_semantics<num_op, std::string, size_t> transformer;

    // None rule: is the argument is a null pointer, interpreting this as a zero number
    transformer.add_rule([](const std::shared_ptr<num_op>& arg) {
        return (!arg);
    }, [](language_semantics<num_op, std::string, size_t>* rec, const std::shared_ptr<num_op>& arg) {
        std::vector<std::pair<std::string,std::shared_ptr<size_t>>> result;
        result.emplace_back("none", std::make_shared<size_t>(0));
        return result;
    });
    // Base case: if the value is a number, returning this as its semantics
    transformer.add_rule([](const std::shared_ptr<num_op>& arg) {
        return (arg) && arg->casus == VAL;
    }, [](language_semantics<num_op, std::string, size_t>* rec, const std::shared_ptr<num_op>& arg) {
        std::vector<std::pair<std::string,std::shared_ptr<size_t>>> result;
        result.emplace_back("val", std::make_shared<size_t>(arg->val));
        return result;
    });
    // Generic expression: if this is a parenthesis, then return the evaluation of the enclosed expression
    transformer.add_rule([](const std::shared_ptr<num_op>& arg) {
        return (arg) && (arg->left) && arg->casus == EXPR;
    }, [](language_semantics<num_op, std::string, size_t>* rec, const std::shared_ptr<num_op>& arg) {
        return rec->operator()(arg->left);
    });
    // Plus: if the operator is a plus and none of the argument is null, and if the recursive call is still holding by
    // returning a non-empty vector, then considering the first returned output for the computation for each operand
    // and perform the sum of the two intermediate results
    transformer.add_rule([](const std::shared_ptr<num_op>& arg) {
        return (arg) && (arg->left) && (arg->right) && arg->casus == PLUS;
    }, [](language_semantics<num_op, std::string, size_t>* rec, const std::shared_ptr<num_op>& arg) {
        auto L = rec->operator()(arg->left);
        auto R = rec->operator()(arg->right);
        std::vector<std::pair<std::string,std::shared_ptr<size_t>>> result;
        if ((!L.empty()) && (!R.empty())) {
            size_t op_result = *L[0].second + *R[0].second ;
            result.emplace_back("PLUS", std::make_shared<size_t>(op_result));
        }
        return result;
    });
    // Times: same as above, but for the multiplication
    transformer.add_rule([](const std::shared_ptr<num_op>& arg) {
        return (arg) && (arg->left) && (arg->right) && arg->casus == TIMES;
    }, [](language_semantics<num_op, std::string, size_t>* rec, const std::shared_ptr<num_op>& arg) {
        auto L = rec->operator()(arg->left);
        auto R = rec->operator()(arg->right);
        std::vector<std::pair<std::string,std::shared_ptr<size_t>>> result;
        if ((!L.empty()) && (!R.empty())) {
            size_t op_result = *L[0].second * *R[0].second ;
            result.emplace_back("TIMES", std::make_shared<size_t>(op_result));
        }
        return result;
    });
    // Minus: same as above, but considering tha the left operand shall always be greater or equal to the right one,
    // as we do not allow for negative values. Otherwise, no value is provided
    transformer.add_rule([](const std::shared_ptr<num_op>& arg) {
        return (arg) && (arg->left) && (arg->right) && arg->casus == MINUS;
    }, [](language_semantics<num_op, std::string, size_t>* rec, const std::shared_ptr<num_op>& arg) {
        auto L = rec->operator()(arg->left);
        auto R = rec->operator()(arg->right);
        std::vector<std::pair<std::string,std::shared_ptr<size_t>>> result;
        if ((!L.empty()) && (!R.empty()) && (*L[0].second >= *R[0].second)) {
            size_t op_result = *L[0].second - *R[0].second ;
            result.emplace_back("MINUS", std::make_shared<size_t>(op_result));
        }
        return result;
    });
    // Division: same as the sum, but considering that the right operand is not evaluated to zero, as we do not allow
    // a division by zero
    transformer.add_rule([](const std::shared_ptr<num_op>& arg) {
        return (arg) && (arg->left) && (arg->right) && arg->casus == DIV;
    }, [](language_semantics<num_op, std::string, size_t>* rec, const std::shared_ptr<num_op>& arg) {
        auto L = rec->operator()(arg->left);
        auto R = rec->operator()(arg->right);
        std::vector<std::pair<std::string,std::shared_ptr<size_t>>> result;
        if ((!L.empty()) && (!R.empty()) && (*R[0].second > 0)) {
            size_t op_result = *L[0].second / *R[0].second ;
            result.emplace_back("MINUS", std::make_shared<size_t>(op_result));
        }
        return result;
    });

    // Creating the base cases
    std::shared_ptr<num_op> one = std::make_shared<num_op>(1);
    std::shared_ptr<num_op> two = std::make_shared<num_op>(2);
    std::shared_ptr<num_op> three = std::make_shared<num_op>(3);
    std::shared_ptr<num_op> four = std::make_shared<num_op>(4);
    std::shared_ptr<num_op> zero = std::make_shared<num_op>(0);

    // Creating the expressions
    auto op1 = std::make_shared<num_op>(std::make_shared<num_op>(one, PLUS, two), TIMES, three);
    auto op2 = std::make_shared<num_op>(std::make_shared<num_op>(one, PLUS, two), DIV, three);
    auto op3 = std::make_shared<num_op>(std::make_shared<num_op>(one, PLUS, two), MINUS, three);
    auto op4 = std::make_shared<num_op>(std::make_shared<num_op>(one, PLUS, two), MINUS, four);
    auto op5 = std::make_shared<num_op>(std::make_shared<num_op>(one, PLUS, two), DIV, zero);

    // Now, printing and evaluating each expression
    {
        std::cout << "Operation: " << *op1 << std::endl;
        auto result = transformer(op1);
        std::cout << result << std::endl;
    }
    {
        std::cout << "Operation: " << *op2 << std::endl;
        auto result = transformer(op2);
        std::cout << result << std::endl;
    }
    {

        std::cout << "Operation: " << *op3 << std::endl;
        auto result = transformer(op3);
        std::cout << result << std::endl;
    }
    {

        std::cout << "Operation: " << *op4 << std::endl;
        auto result = transformer(op4);
        std::cout << result << std::endl;
    }
    {

        std::cout << "Operation: " << *op5 << std::endl;
        auto result = transformer(op5);
        std::cout << result << std::endl;
    }


    return 0;
}
