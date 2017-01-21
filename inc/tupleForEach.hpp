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
#ifndef ASSEMBLYLINE_TUPLEFOREACH_HPP
#define ASSEMBLYLINE_TUPLEFOREACH_HPP

namespace assemblyLine {

    /*
     * Auto-unfolding helper functions for iterating over each element in a tuple (or each pair, overlapping)
     */
    template<typename TType, typename FType>
    void _tupleForEachPair(TType&&, FType, std::integral_constant <size_t,
    std::tuple_size<typename std::remove_reference<TType>::type>::value>) { }

    template<typename TType, typename FType>
    void _tupleForEach(TType&&, FType, std::integral_constant <size_t,
    std::tuple_size<typename std::remove_reference<TType>::type>::value>) { }

    template<std::size_t I, typename TType, typename FType, typename = typename
    std::enable_if<I != std::tuple_size<typename std::remove_reference<TType>::type>::value>::type>
    void _tupleForEachPair(TType&& tuple, FType func, std::integral_constant <size_t, I>) {
        func(std::get<I - 1>(tuple), std::get<I>(tuple), I);
        _tupleForEachPair(std::forward<TType>(tuple), func, std::integral_constant<size_t, I + 1>());
    }

    template<std::size_t I, typename TType, typename FType, typename = typename
    std::enable_if<I != std::tuple_size<typename std::remove_reference<TType>::type>::value>::type>
    void _tupleForEach(TType&& tuple, FType func, std::integral_constant <size_t, I>) {
        func(std::get<I>(tuple), I);
        _tupleForEach(std::forward<TType>(tuple), func, std::integral_constant<size_t, I + 1>());
    }

    template<typename TType, typename FType>
    void tupleForEachPair(TType&& tuple, FType func) {
        _tupleForEachPair(std::forward<TType>(tuple), func, std::integral_constant<size_t, 1>());
    }

    template<typename TType, typename FType>
    void tupleForEach(TType&& tuple, FType func) {
        _tupleForEach(std::forward<TType>(tuple), func, std::integral_constant<size_t, 0>());
    }
}

#endif //ASSEMBLYLINE_TUPLEFOREACH_HPP
