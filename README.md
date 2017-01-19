##Assembly Line is a small templated header-only C++ tool.
It is meant to be used in systems that deal with lots of data being streamed through many different operations, or "modules".
A module is any logic that accepts incoming data, uses or changes it in some way, and then streams some form of it right back out as fast as it can.
A chain of these modules constitues an "assembly line".
Modules are run in their own threads asynchronously thanks to this awesome lock-free queue: https://github.com/cameron314/readerwriterqueue

Assembly Line was originally written to help with processing lidar data in real time.