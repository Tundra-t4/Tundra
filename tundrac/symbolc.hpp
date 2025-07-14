#include "base.hpp"
#include "AST.hpp"

#ifndef SYMBOL_C
#define SYMBOL_C

struct SymReturn {
    public:
    SymReturn(std::any value=std::any()): value(value) {}
    std::any getValue(){return value;}
    private:
    std::any value;
};

struct SymTypeSafeList {
    public:
    SymTypeSafeList(std::any type, int64_t size): type(type), size(size) {}
    std::any get(int index){
        return type;
    }
    int len() {return size;}
    void setSZU(bool s) {this->szu = s;}
    bool getSZU(){return szu;}
    private:
    std::any type;
    int64_t size;
    bool szu = false;
};

#endif