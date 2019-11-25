#pragma once

namespace {



struct Movable {
    static int& constructors() {
        static thread_local int constructors = 0;
        return constructors;
    }
    
    static int& destructors() {
        static thread_local int destructors = 0;
        return destructors;
    }

    static void reset() {
        constructors() = 0;
        destructors() = 0;
    }

    Movable() { ++constructors(); }
    Movable(int xx) : x(xx) { ++constructors(); }
    Movable(const Movable&) = delete;
    Movable(Movable&& o) : x(o.x) { o.x = 0; ++constructors(); }
    ~Movable() { ++destructors(); }

    int x = -1;
    operator int() const { return x; }
    auto hash() const { return x; }
};



struct Copyable {
    static int& constructors() {
        static thread_local int constructors = 0;
        return constructors;
    }
    
    static int& destructors() {
        static thread_local int destructors = 0;
        return destructors;
    }

    static void reset() {
        constructors() = 0;
        destructors() = 0;
    }

    Copyable() { ++constructors(); }
    Copyable(int xx) : x(xx) { ++constructors(); }
    Copyable(const Copyable& o) : x(o.x) { ++constructors(); }
    Copyable(Copyable&& o) = delete;
    ~Copyable() { ++destructors(); }

    int x = -1;
    operator int() const { return x; }
    auto hash() const { return x; }
};



} // namespace
