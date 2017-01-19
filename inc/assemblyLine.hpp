/*
 * Copyright (c) 2016 Galen Cochrane
 * Galen Cochrane <galencochrane@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef ASSEMBLYLINE_ASSEMBLYLINE_HPP
#define ASSEMBLYLINE_ASSEMBLYLINE_HPP

#include <vector>
#include <tuple>
#include <thread>
#include "../extern/readerwriterqueue/readerwriterqueue.h"

namespace assemblyLine {

    // Auto-unfolding helper function for iterating over contiguous element pairs in a tuple
    template<typename TType, typename FType>
    void tupleForPairsAfterFirst(TType&&, FType, std::integral_constant<size_t,
            std::tuple_size<typename std::remove_reference<TType>::type >::value>) { }

    template<std::size_t I, typename TType, typename FType, typename = typename
            std::enable_if<I!=std::tuple_size<typename std::remove_reference<TType>::type>::value>::type >
    void tupleForPairsAfterFirst(TType&& t, FType f, std::integral_constant<size_t, I>) {
        f(std::get<I - 1>(t), std::get<I>(t), I);
        tupleForPairsAfterFirst(std::forward<TType>(t), f, std::integral_constant<size_t, I + 1>());
    }
    template<typename TType, typename FType>
    void tupleForPairsAfterFirst(TType&& t, FType f) {
        tupleForPairsAfterFirst(std::forward<TType>(t), f, std::integral_constant<size_t, 1>());
    }
    // Auto-unfolding helper functions for iterating over each element in a tuple
    template<typename TType, typename FType>
    void tupleForEach(TType&&, FType, std::integral_constant<size_t,
            std::tuple_size<typename std::remove_reference<TType>::type >::value>) { }

    template<std::size_t I, typename TType, typename FType, typename = typename
    std::enable_if<I!=std::tuple_size<typename std::remove_reference<TType>::type>::value>::type >
    void tupleForEach(TType&& t, FType f, std::integral_constant<size_t, I>) {
        f(std::get<I>(t));
        tupleForEach(std::forward<TType>(t), f, std::integral_constant<size_t, I + 1>());
    }
    template<typename TType, typename FType>
    void tupleForEach(TType&& t, FType f) {
        tupleForEach(std::forward<TType>(t), f, std::integral_constant<size_t, 0>());
    }

    // Module class
    template <typename Derived, typename I, typename O>
    class Module {
        Derived& derived();
    public:
        moodycamel::ReaderWriterQueue<I>  input;
        moodycamel::ReaderWriterQueue<O>* output;
        std::thread spawn();
        void setOutput(moodycamel::ReaderWriterQueue<O>* output);
        void beginLoop();
        bool quit = false;
    };

    // Chain class for connecting Modules
    template <typename... ModuleTypes>
    class Chain {
        std::tuple<ModuleTypes...> modules;
        std::vector<std::thread> threads;
        size_t size;
    public:
        Chain(ModuleTypes... modules);
        int engage();
        int disengage();

    };

    // Module method implementations
    template <typename Derived, typename I, typename O>
    void Module<Derived, I, O>::setOutput(moodycamel::ReaderWriterQueue<O>* output) {
        this->output = output;
    };
    template <typename Derived, typename I, typename O>
    Derived& Module<Derived, I, O>::derived() {
        return *static_cast<Derived*>(this);
    }
    template <typename Derived, typename I, typename O>
    std::thread Module<Derived, I, O>::spawn() {
        return std::thread(&Module<Derived, I, O>::beginLoop, this);
    };
    template <typename Derived, typename I, typename O>
    void Module<Derived, I, O>::beginLoop() {
        while (!quit) {
            derived().operate();
        }
    }

    // Chain method implementations
    template <typename... ModuleTypes>
    Chain<ModuleTypes...>::Chain(ModuleTypes... modules) {
        this->modules = std::make_tuple(modules...);
        tupleForPairsAfterFirst(this->modules, [](auto& prevModule, auto& module, size_t index) {
            prevModule->output = &(module->input);
        });
    }
    template <typename... ModuleTypes>
    int Chain<ModuleTypes...>::engage() {
        disengage();
        tupleForEach(this->modules, [&](auto& module) {
            module->quit = false;
            threads.emplace_back(module->spawn());
        });
        return 0;
    }
    template <typename... ModuleTypes>
    int Chain<ModuleTypes...>::disengage() {
        tupleForEach(this->modules, [](auto& module) {
            module->quit = true;
        });
        for (int i = 0; i < threads.size(); ++i) {
            if (threads[i].joinable()) {
                threads[i].join();
            }
        }
        threads.clear();
        return 0;
    }
}

#endif //ASSEMBLYLINE_ASSEMBLYLINE_HPP
