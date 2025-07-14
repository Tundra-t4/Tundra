#include "base.hpp"
#include "interpreterc.hpp"
#include "symbolc.hpp"
#include "Scope.hpp"
#include "Tokens.hpp"
#include "Lexer.hpp"
#include "AST.hpp"


#ifndef TypeCheck
#define TypeCheck
// Type checkers
template <typename K,typename T>
bool compareOMap(const tsl::ordered_map<K, T>& map1, const tsl::ordered_map<K, T>& map2);
template <typename K,typename T>
bool compareUMap(const std::unordered_map<K, T>& map1, const std::unordered_map<K, T>& map2);
bool compareAnyTy(std::any a, std::any b) {
    logat(demangle(a.type().name()) + " == " + demangle(b.type().name()),"cAT");
    bool checker = false;
    if (a.type() == typeid(std::shared_ptr<Future>) && b.type() == typeid(std::shared_ptr<Future>)){
        return true;
    }
    if (a.type() == typeid(std::shared_ptr<Future>)){
        return compareAnyTy((std::any_cast<std::shared_ptr<Future>>(a))->get(),b);
    }
    if (a.type() == typeid(Enum) && b.type() == typeid(EnumKey)){
        const auto& EnumA = std::any_cast<Enum>(a);
        const auto& EKB = std::any_cast<EnumKey>(b);
        return compareAnyTy(EnumA,EKB.getBase());
    }
    if (a.type() == typeid(StructDecl) && b.type() == typeid(StructInstance)){
        auto& sstructA = std::any_cast<StructDecl&>(a);
        auto& sinstanceB = std::any_cast<StructInstance&>(b);
        auto fields = sstructA.getValue();
        auto ifields = sinstanceB.getValue();
        for (const auto& f : fields){
            if (ifields.find(f.first) == ifields.end()){
                return false;
            }
            if (compareAnyTy(fields[f.first],ifields[f.first])  == false){
                return false;
            }
        }
        auto cons = sstructA.getConsts();
        auto cons2 = sinstanceB.getCons();
        if (!compareMap(cons,cons)) return false;
        auto fns = sstructA.getFns();
        auto fns2 = sinstanceB.getFns();
        if (!compareMap(fns,fns2)) return false;
        return true;
    }
    if (b.type() == typeid(StructDecl) && a.type() == typeid(StructInstance)){
        auto& sstructA = std::any_cast<StructDecl&>(b);
        auto& sinstanceB = std::any_cast<StructInstance&>(a);
        auto fields = sstructA.getValue();
        auto ifields = sinstanceB.getValue();
        for (const auto& f : fields){
            if (ifields.find(f.first) == ifields.end()){
                return false;
            }
            if (compareAnyTy(fields[f.first],ifields[f.first])  == false){
                return false;
            }
        }
        auto cons = sstructA.getConsts();
        auto cons2 = sinstanceB.getCons();
        if (!compareMap(cons,cons)) return false;
        auto fns = sstructA.getFns();
        auto fns2 = sinstanceB.getFns();
        if (!compareMap(fns,fns2)) return false;
        return true;
    }
    if (a.type() == typeid(Class) && b.type() == typeid(ClassInstance)){
        const auto& c = std::any_cast<Class>(a);
        const auto& ci = std::any_cast<ClassInstance>(b);
        return compareMap(c.getConsts(),ci.getConsts()) && compareMap(c.getValue(),ci.getValue());
    }
    if (a.type() == typeid(Object) && b.type() != typeid(Object)){
        return compareAnyTy(std::any_cast<Object>(a).GetStore(),b);
    }
    if (b.type() == typeid(Object) && a.type() != typeid(Object)){
        return compareAnyTy(std::any_cast<Object>(b).GetStore(),a);
    }

    if (a.type() != b.type()) return false;
    // change to getT
    if (a.type() == typeid(StructDecl)) {
        auto& structA = std::any_cast<StructDecl&>(a);
        auto& structB = std::any_cast<StructDecl&>(b);
        return compareOMap<std::string>(structA.getValue(), structB.getValue()) && compareMap(structA.getConsts(),structB.getConsts()) && compareMap(structA.getFns(),structB.getFns());
    } else if (a.type() == typeid(StructInstance)) {
        auto& instanceA = std::any_cast<StructInstance&>(a);
        auto& instanceB = std::any_cast<StructInstance&>(b);
        return compareOMap<std::string>(instanceA.getValue(),instanceB.getValue()) && compareMap(instanceA.getCons(),instanceB.getCons()) && compareMap(instanceA.getFns(),instanceB.getFns());
    } else if (a.type() == typeid(EnumKey)) {


        auto& enumkA = std::any_cast<EnumKey&>(a);
        auto& enumkB = std::any_cast<EnumKey&>(b);

        if (enumkA.getValue().size() != enumkB.getValue().size()){
            return false;
        }
        if (enumkA.getName() != enumkB.getName()){
            return false;
        }
        for(int i = 0;i != enumkA.getValue().size();i++) {
            if (compareAnyTy(enumkA.getValue()[i],enumkB.getValue()[i]) == false){
                return false;
            }
        }
        return true;
    } else if (a.type() == typeid(Enum)) {
        const auto& enumA = std::any_cast<const Enum&>(a);
        const auto& enumB = std::any_cast<const Enum&>(b);
        return compareUMap<std::string,EnumKey>(enumA.getValue(),enumB.getValue());
    } else if (a.type() == typeid(TypeSafeList)) {
         auto& listA = std::any_cast< TypeSafeList&>(a);
         auto& listB = std::any_cast< TypeSafeList&>(b);
        if (listA.getSZU()){
            return compareAnyTy(listA.getType(),listB.getType());
        }
        return listA.len() == listB.len() && compareAnyTy(listA.getType(),listB.getType());
    } else if (a.type() == typeid(Tuple)) {
         auto& tupleA = std::any_cast< Tuple&>(a);
         auto& tupleB = std::any_cast< Tuple&>(b);
        return tupleA.len() == tupleB.len() && compareMap(tupleA.getcomparer(),tupleB.getcomparer());
    } else if (a.type() == typeid(List)) {
        auto& listA = std::any_cast<List&>(a);
        auto& listB = std::any_cast<List&>(b);
        return listA.len() == listB.len();
    } else if (a.type() == typeid(PtrVal)) {
        const auto& ptrA = std::any_cast<const PtrVal&>(a);
        const auto& ptrB = std::any_cast<const PtrVal&>(b);
        return compareAnyTy(ptrA.getValue(), ptrB.getValue());
    } else if (a.type() == typeid(PtrVal*)) {
    const auto* ptrA = std::any_cast<const PtrVal*>(a);
    const auto* ptrB = std::any_cast<const PtrVal*>(b);

    // Compare the pointers directly
    if (ptrA == ptrB) {
        return true; // Same reference
    }} else if (a.type() == typeid(Pointer)) {
        const auto& ptrA = std::any_cast<const Pointer&>(a);
        const auto& ptrB = std::any_cast<const Pointer&>(b);
        return compareAnyTy(ptrA.getptr()->getValue(),ptrB.getptr()->getValue()) && ptrA.getMut() == ptrA.getMut();
    } else if (a.type() == typeid(BorrowedPointer)) {
        const auto& borrowA = std::any_cast<const BorrowedPointer&>(a);
        const auto& borrowB = std::any_cast<const BorrowedPointer&>(b);
        return borrowA.isMut() == borrowB.isMut() && getTypeName(borrowA.getptr()->getValue()) == getTypeName(borrowB.getptr()->getValue());
    } else if (a.type() == typeid(Class)){
        const auto& c = std::any_cast<Class>(a);
        const auto& cb = std::any_cast<Class>(b);
        return compareMap(c.getConsts(),cb.getConsts()) && compareMap(c.getValue(),cb.getValue());
    } else if (a.type() == typeid(ClassInstance)){
        const auto& c = std::any_cast<ClassInstance>(a);
        const auto& cb = std::any_cast<ClassInstance>(b);
        return compareMap(c.getConsts(),cb.getConsts()) && compareMap(c.getValue(),cb.getValue());
    } else if (a.type() == typeid(Object)) {
        const auto& objA = std::any_cast<const Object&>(a);
        const auto& objB = std::any_cast<const Object&>(b);
        return objA.getType() == objB.getType(); // make this better
    } else if (a.type() == typeid(MappedFunction)) {
        auto& MFNA = std::any_cast<MappedFunction&>(a);
        auto& MFNB = std::any_cast<MappedFunction&>(b);
        auto MFNAT = MFNA.gettick();

        auto MFNBT = MFNB.gettick();
        if (MFNAT.size() != MFNBT.size()){
            return false;
        }
        auto MFNAA = MFNA.getargs();
        auto MFNBA = MFNB.getargs();
        //println("argcheck");
        for (const auto& k: MFNAT){

            if ((k.second != "self" && k.first != 0) || k.second != "self"){

            if (compareAnyTy(MFNAA[k.second],MFNBA[MFNBT[k.first]]) == false){
               
                return false;
            }} else if (MFNBT[0] != "self"){
                return false;
            } else {
                return true;
            }


        }


        if (compareAnyTy(MFNA.getretty(),MFNB.getretty()) == false){
            return false;
        }

        return true;
    } else if (a.type() == b.type()){
        return true;
    }


    return false; 
}

template <typename K,typename T>
bool compareMap(const tsl::ordered_map<K, T>& map1, const tsl::ordered_map<K, T>& map2) {

    if (map1.size() != map2.size()) return false;
    for (const auto& [key, value1] : map1) {
        logat(convertToString(key),"cM");
        const auto it = map2.find(key);
        if (it == map2.end()) return false;
        const T& value2 = it->second;
        if (compareAnyTy(value1, value2) == false) return false;
    }
    return true;
}
template <typename K,typename T>
bool compareOMap(const tsl::ordered_map<K, T>& map1, const tsl::ordered_map<K, T>& map2) {

    if (map1.size() != map2.size()) return false;
    for (const auto& [key, value1] : map1) {
        const auto it = map2.find(key);
        if (it == map2.end()) return false;
        const T& value2 = it->second;
        if (compareAnyTy(value1, value2) == false) return false;
    }
    return true;
}

template <typename K,typename T>
bool compareUMap(const std::unordered_map<K, T>& map1, const std::unordered_map<K, T>& map2) {

    if (map1.size() != map2.size()) return false;
    for (const auto& [key, value1] : map1) {
        const auto it = map2.find(key);
        if (it == map2.end()) return false;
        const T& value2 = it->second;
        if (compareAnyTy(value1, value2) == false) return false;
    }
    return true;
}
#endif