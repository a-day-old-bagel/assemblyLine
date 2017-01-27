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
#include "tupleForEach.hpp"

namespace assemblyLine {

    /*
     * The Module class represents one basic operation that presumably has an input and an output.
     * Module must be implemented using the Curiously Recurring Template Pattern (CRTP).
     * The inheriting class must provide an operate() method and init() method.
     * Presumably, the operate() method will pull things from the input queue, do something
     * with them, and then push things onto the output queue. The init() method is for whatever the user wants, and
     * probably most useful for things that must happen on the same thread as other things in the operate() method, but
     * are only meant to happen once (upon initialization).
     *
     * Template parameters are:
     * 1. The name of the class inheriting from Module (google CRTP for why)
     * 2. The type of the input items
     * 3. The type of the output items
     *
     * See test.cpp for an example of how to use these.
     */
    template <typename Derived, typename I, typename O>
    class Module {
        template <typename ResultType, typename... ModuleTypes> friend class Chain;
        Derived& derived();
        std::thread spawn();
        void beginLoop();
        bool quit = false;
    protected:
        moodycamel::ReaderWriterQueue<O>* output;
    public:
        moodycamel::ReaderWriterQueue<I>  input;
        void setOutput(moodycamel::ReaderWriterQueue<O>* output);
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
        derived().init();
        while (!quit) {
            derived().operate();
        }
    }

    /*
     * The Chain class handles a collection of Modules, where each Module's output presumably feeds into the input
     * of the next. In reality, this depends on the user's implementation of their Modules' operate() methods.
     *
     * Template arguments are:
     * 1. type of final results that the chain produces (same as output parameter of last module)
     * ... The types of each module in order (these types must inherit from Module and implement operate() and init() ).
     *
     * see test.cpp for an example of how to use these.
     */
    template <typename ResultType, typename... ModuleTypes>
    class Chain {
        std::tuple<ModuleTypes*...> modules;
        std::vector<std::thread> threads;

        /*
         * Don't look at these two functions, they'll burn your eyes out.
         */
        template <typename ...MT>
        auto linkModules()
        -> typename std::enable_if<(sizeof...(MT) > 1), void>::type {
            tupleForEachPair(modules, [&](auto& prevModule, auto& module, size_t index) {
                prevModule->output = &(module->input);
                if (index = sizeof...(ModuleTypes) - 1) {
                    module->output = &results;
                }
            });
        }
        template <typename ...MT>
        auto linkModules()
        -> typename std::enable_if<(sizeof...(MT) == 1), void>::type {
            std::get<0>(modules)->output = &results;
        }

    public:
        /*
         * Constructor take pointers to instantiated Modules who's types were given as template parameters
         */
        Chain(ModuleTypes*... modules);
        /*
         * Final results of the assembly line (the Chain) can be retrieved from here as they come in.
         */
        moodycamel::ReaderWriterQueue<ResultType> results;
        /*
         * First calls disengage() in case modules were already working, then starts all modules working in
         * their own respective threads. Depending on the user's Module implementations,
         * results could now start appearing in the results queue.
         */
        int engage();
        /*
         * Stops work on all Modules. However, it is up to the user to make sure that the operate() and init()
         * methods for each module actually return. If they do not, then this call may block indefinitely while waiting
         * for threads to exit.
         * When disengage() is called, any data currently working its way through the chain may or may not make it
         * into "results". More guarantees may be made in future versions. For now, just make sure you've gotten
         * the data you want out of the chain before you call disengage.
         */
        int disengage();
    };
    /*
     * Chain method implementations follow...
     */
    template <typename ResultType, typename... ModuleTypes>
    Chain<ResultType, ModuleTypes...>::Chain(ModuleTypes*... modules) {
        this->modules = std::make_tuple(modules...);
        linkModules<ModuleTypes...>();
    }
    template <typename ResultType, typename... ModuleTypes>
    int Chain<ResultType, ModuleTypes...>::engage() {
        disengage();
        tupleForEach(this->modules, [&](auto& module, size_t index) {
            module->quit = false;
            threads.emplace_back(module->spawn());
        });
        return 0;
    }
    template <typename ResultType, typename... ModuleTypes>
    int Chain<ResultType, ModuleTypes...>::disengage() {
        tupleForEach(this->modules, [&](auto& module, size_t index) {
            if (threads.size() > index && threads[index].joinable()) {
                module->quit = true;
                threads[index].join();
            }
        });
        threads.clear();
        return 0;
    }
}

#endif //ASSEMBLYLINE_ASSEMBLYLINE_HPP
