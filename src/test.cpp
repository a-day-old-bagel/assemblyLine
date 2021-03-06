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

#include "assemblyLine.hpp"
#include <chrono>

using namespace assemblyLine;

class IntFloatModule : public Module<IntFloatModule, int, float> {
public:
    void init() {

    }
    void operate() {
        int n;
        while(input.try_dequeue(n)) {
            output->enqueue(n * 2.f);
        }
    }
    void deinit() {

    }
};
class FloatCharModule : public Module<FloatCharModule, float, unsigned char> {
public:
    void init() {

    }
    void operate() {
        float f;
        while(input.try_dequeue(f)) {
            output->enqueue((unsigned char)((int)f + 1));
        }
    }
    void deinit() {

    }
};

int main(int argc, char* argv[]) {

    IntFloatModule modA;
    FloatCharModule modB;

    Chain<unsigned char, IntFloatModule, FloatCharModule> opChain(&modA, &modB);
    opChain.engage();

    for (int i = 0; i < 10; ++i) {
        modA.input.enqueue(i);
    }

    // wait for all data to (probably) make it into opChain.results
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    opChain.disengage();

    int n;
    while(opChain.results.try_dequeue(n)) {
        printf("%i\n", n);
    }

    return 0;
}