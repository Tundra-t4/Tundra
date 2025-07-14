
#include "base.hpp"
#include "interpreterc.hpp"

#ifndef SCOPE_VAR
#define SCOPE_VAR

struct Variable {
    std::any value;  
    int valueid;
    

    Variable() = default;
    Variable(std::any value,bool mut=false) : value(std::move(value)), mut(mut),valueid(vidct++) {}
    Variable(const Variable& other) : value(other.value), mut(other.mut), valueid(other.valueid) {}
    Variable(Variable&& other) noexcept : value(std::move(other.value)), mut(std::move(other.mut)),valueid(std::move(other.valueid)) {}
    void setVID(int id){
        this->valueid = id;
    }
    std::string selfup;

    void setselfup(std::string s){
        this->selfup = s;
    }

    // Assignment operators
    Variable& operator=(const Variable& other) {
        if (this != &other) {
            value = other.value;
        }
        return *this;
    }

    Variable& operator=(Variable&& other) noexcept {
        if (this != &other) {
            value = std::move(other.value);
        }
        return *this;
    }
    void update(std::any v){
        this->value = v;
    }

    // Type checking methods
    bool isTagF() const { return value.type() == typeid(TagFunction); }
    bool isTag() const { return value.type() == typeid(Tag); }
    bool isCEFN() const { return value.type() == typeid(CEXTFunction); }
    bool isModule() const { return value.type() == typeid(Module); }
    bool isModuleM() const { return value.type() == typeid(ModuleMethod); }
    bool isCFN() const { return value.type() == typeid(CFunction); }
    bool isClassI() const { return value.type() == typeid(ClassInstance); }
    bool isClass() const { return value.type() == typeid(Class); }
    bool isMFN() const { return value.type() == typeid(MappedFunction); }
    bool isUFN() const { return value.type() == typeid(UnMappedFunction); }
    bool isNull() const { return value.type() == typeid(nullification); }
    bool isNullv() const { return value.type() == typeid(nullificationv); }
    bool isInt() const { return value.type() == typeid(int32_t); }
    bool isString() const { return value.type() == typeid(std::string); }
    bool isBool() const { return value.type() == typeid(bool); }
    bool isObj() const { return value.type() == typeid(Object); }
    bool isList() const { return value.type() == typeid(List); }
    bool isTSL() const { return value.type() == typeid(TypeSafeList); }
    bool isTuple() const { return value.type() == typeid(Tuple); }
    bool isStruct() const { return value.type() == typeid(StructDecl); }
    bool isStructI() const { return value.type() == typeid(StructInstance); }
    bool isEnum() const { return value.type() == typeid(Enum); }
    bool isEK() const { return value.type() == typeid(EnumKey); }
    bool isPTR() const { return value.type() == typeid(Pointer); }
    bool isBPTR() const { return value.type() == typeid(BorrowedPointer); }
    bool isMut() {return mut;}
    void setMut() {this->mut=true;}
    CFunction getCFN() const {
        if (isCFN()) {
            return std::any_cast<CFunction>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    TagFunction getTagF() const {
        if (isTagF()) {
            return std::any_cast<TagFunction>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    Tag getTag() const {
        if (isTag()) {
            return std::any_cast<Tag>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    Module getModule() const {
        if (isModule()) {
            return std::any_cast<Module>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    ModuleMethod getModuleM() const {
        if (isModuleM()) {
            return std::any_cast<ModuleMethod>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    CEXTFunction getCEFN() const {
        if (isCEFN()) {
            return std::any_cast<CEXTFunction>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }

    Class getClass() const {
        if (isClass()) {
            return std::any_cast<Class>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    ClassInstance getClassI() const {
        if (isClassI()) {
            return std::any_cast<ClassInstance>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    int getInt() const {
        if (isInt()) {
            return std::any_cast<int32_t>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }

    nullification getNull() const {
        if (isNull()) {
            return std::any_cast<nullification>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }

    std::string getString() const {
        if (isString()) {
            return std::any_cast<std::string>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }

    bool getBool() const {
        if (isBool()) {
            return std::any_cast<bool>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }

    Object getObj() const {
        if (isObj()) {
            
            return std::any_cast<Object>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }

    List getList() const {
        if (isList()) {
            
            return std::any_cast<List>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    Tuple getTuple() const {
        if (isTuple()) {
            
            return std::any_cast<Tuple>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    TypeSafeList getTSL() const {
        if (isTSL()) {
            
            return std::any_cast<TypeSafeList>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    StructDecl getStruct() const {
        if (isStruct()) {
            return std::any_cast<StructDecl>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    StructInstance getStructI() const {
        if (isStructI()) {
            return std::any_cast<StructInstance>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    Enum getEnum() const {
        if (isEnum()) {
            return std::any_cast<Enum>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    EnumKey getEK() const {
        if (isEK()) {
            return std::any_cast<EnumKey>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    Pointer getPTR() const {
        if (isPTR()) {
            return std::any_cast<Pointer>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    BorrowedPointer getBPTR() const {
        if (isBPTR()) {
            return std::any_cast<BorrowedPointer>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    UnMappedFunction getUFN() const {
        if (isUFN()) {
            return std::any_cast<UnMappedFunction>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    MappedFunction getMFN() const {
        if (isMFN()) {
            return std::any_cast<MappedFunction>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }

    nullificationv getnullv() const {
        if (isNullv()) {
            return std::any_cast<nullificationv>(value);
        }
        throw std::bad_any_cast();  // Handle the error as needed
    }
    
    // Get the type as a string
    std::string getType() const {
        if (isInt()) return "int";
        if (isString()) return "RawString";
        if (isBool()) return "bool";
        if (isNull()) return "none";
        if (isNullv()) return "nil";
        if (isObj()) return "obj";
        if (isList()) return "list";
        if (isTuple()) return "tuple";
        if (isStructI()) return "struct";
        if (isEK()) return "ek";
        if (isEnum()) return "enum";
        if (isPTR()) return "ptr";
        if (isBPTR()) return "bptr";
        if (isStruct()) return "_struct";
        if (isTSL()) return "TSL";
        if (isUFN()) return "UFN";
        if (isCFN()) return "CFN";
        if (isCEFN()) return "CEFN";
        if (isMFN()) return "MFN";
        if (isClass()) return "Class";
        if (isClassI()) return "ClassI";
        if (isModule()) return "MLE";
        if (isModuleM()) return "MODM"; // the urge to just call this as (your) MOM
        if (isTag()) return "tag";
        if (isTagF()) return "tagF";
        return "unknown";
    }

    std::any getValue(){
        return value;
    }

private:
bool mut;
};

std::string getTypeName(const std::any& value,bool cb,bool tp){
    if (value.type() == typeid(MappedFunction)){
        auto mfn = std::any_cast<MappedFunction>(value);
        std::string v = "@sig(";
        for (auto& arg: mfn.getargs()){
            v += getTypeName(arg.second) + ",";

        }
        v += ") -> ";
        v += getTypeName(mfn.getretty());
        return v;
    };
    if (value.type() == typeid(StructInstance)) return "Struct";
    if (value.type() == typeid(StructDecl)) return "_struct";
    if (value.type() == typeid(Object)) return std::any_cast<Object>(value).getType();
    if (value.type() == typeid(List)) return "list";
    if (value.type() == typeid(TypeSafeList)) return "[" + getTypeName(std::any_cast<TypeSafeList>(value).getType()) + ":" + std::to_string(std::any_cast<TypeSafeList>(value).len()) + "]";
    if (value.type() == typeid(Tuple)) return "tup";
    if (value.type() == typeid(int8_t)) return "i8";
    if (value.type() == typeid(int16_t)) return "i16";
    if (value.type() == typeid(int32_t)) return "i32";
    if (value.type() == typeid(int64_t)) return "i64";
    if (value.type() == typeid(uint8_t)) return "u8";
    if (value.type() == typeid(uint16_t)) return "u16";
    if (value.type() == typeid(uint32_t)) return "u32";
    if (value.type() == typeid(uint64_t)) return "u64";
    if (value.type() == typeid(float)) return "f32";
    if (value.type() == typeid(double)) return "f64";
    if (value.type() == typeid(std::string)) return "RawString";
    if (value.type() == typeid(CEXTFunction)) return "CEXTFN";
    if (value.type() == typeid(Pointer)) return "&" + getTypeName(std::any_cast<Pointer>(value).getptr()->getValue());
    if (value.type() == typeid(BorrowedPointer)){
        std::string mstr = std::any_cast<BorrowedPointer>(value).isMut() ? " mut" : "";
        return "&b" + (mstr) + " &" + getTypeName(std::any_cast<BorrowedPointer>(value).getptr()->getValue());
    } 
    if (value.type() == typeid(int)) return "Number";
    if (value.type() == typeid(char)) return "Character";
    if (value.type() == typeid(std::wstring)) return "wchar";
    if (value.type() == typeid(nullificationv)) return "nil";
    if (value.type() == typeid(nullification)) return "none";
    if (value.type() == typeid(bool)) return "Bool";
    if (value.type() == typeid(ModuleMethod)) return std::any_cast<ModuleMethod>(value).hash + std::any_cast<ModuleMethod>(value).vname;
    if (value.type() == typeid(TagFunction)){return getTypeName(std::any_cast<TagFunction>(value).MFNlink);}
    if (value.type() == typeid(int8_t))  return "i8";
    if (value.type() == typeid(int16_t)) return "i16";
    if (value.type() == typeid(int32_t)) return "i32";
    if (value.type() == typeid(int64_t)) return "i64";
    if (value.type() == typeid(uint8_t))  return "u8";
    if (value.type() == typeid(uint16_t)) return "u16";
    if (value.type() == typeid(uint32_t)) return "u32";
    if (value.type() == typeid(uint64_t)) return "u64";
    if (value.type() == typeid(uintptr_t)) return "u64";
    if (value.type() == typeid(float)) return "f32";
    if (value.type() == typeid(double)) return "f64";
    if (value.type() == typeid(CFunction)) return "CompileFunction";

    println("ukt:" + demangle(value.type().name()));

    return "unknown";
}

template <typename K,typename T>
bool compareMap(const tsl::ordered_map<K, T>& map1, const tsl::ordered_map<K, T>& map2);





// Scope class to manage variables
class Scope {
public:
    Scope(bool isMain = false,std::shared_ptr<Scope> parentScope = nullptr) : isMain(isMain), parentScope(parentScope) {}

    void addVariable(const std::string& name, std::shared_ptr<Variable> var) {
        auto check = variables.find(name); // mut tracelog time.
        if (check != variables.end()){
            if (check->second->isMut() == false){
                std::string err = "Attempting to modify immutable variable: ";
                err = err + name;
                throw std::runtime_error(err);
            } else {
                //println("MUT: TRUE");
                //println(name);
                try {
                if (/*var->value.type() != typeid(nullification) &&*/ (var->value.type() != check->second->value.type() && ruleset["typing"] == "static")){
                    throw std::runtime_error("Attempting to modify static typed variable with different type than inferred");
                }} catch (std::bad_any_cast){
                    throw std::runtime_error("Ruleset.typing can only have a string value\nEither 'static' or 'dynamic'");
                }
                var->setMut();
                var->setVID( check->second->valueid);
                variables[name] = var;
            }
        } else {
        //println("MUT: x");
        //println(name);

            //println(name);
            //println("check: " + convertToString(check != variables.end()));
            ////println(convertToString(var->getInt()));

            
        
        variables[name] = var;
        }
        logat("Successfully set var","Scope.aV");
    }

    std::shared_ptr<Variable> getVariable(const std::string& name)  {
        auto it = variables.find(name);
        
        if (it != variables.end()) {
            
            return it->second;
        }
        
        if (parentScope) {
            auto it =  parentScope->getVariable(name);
            if (it->getType() == "ptr"){
                
                return std::make_shared<Variable>(BorrowedPointer(it->getPTR().getptr(),false));
            }
            return it;
        }
        throw std::runtime_error("Variable '" + name + "' does not exist within this scope or previous scopes");
        return nullptr;
    }
    void setLoop(){this->isLoop=true;}
    bool getLoop(){return isLoop;}
    void setSign(){this->sig=true;}
    bool getSign(){return sig;}
    void removeVariable(const std::string& name,bool panic=true) {
        if (variables.find(name)  == variables.end()){
            if (panic == true){
                throw std::runtime_error("Deletion of non-existent variable within scope: " + name);
            }
            return;
        }
        //println("Deleted " + name);
        variables.erase(name);
    }

    /*void printVariables()  { //legacy when we didn't have printf or even __compileprint__
    for (const auto& pair : variables) {
        if (dummyValues.find(pair.first) == dummyValues.end()){
        //std::cout << "Variable " << pair.first << " = ";
        std::string type = pair.second->getType();
        

        if (type == "int") {
            //std::cout << pair.second->getInt();
        } else if (type == "RawString") {
            //std::cout << pair.second->getString();
        } else if (type == "bool") {
            pair.second->visit();
        } else if (type=="obj"){
            Object obj = pair.second->getObj();
            std::string out = obj.printable();
            //std::cout << out;
        } else {
            //std::cout << "Type: " << type;
        }
        //std::cout << std::endl;
    }
    //println("DONE WITH VARS");
    }
    }*/

    bool isptr(std::string vcheck){
        if (pointers.find(vcheck) != pointers.end()){
            return true;
        }
        return false;
    }
    bool isMainScope() const { return isMain; }
    bool isVar(std::string n){
        auto b1 = variables.find(n) != variables.end();
        if (b1){
            return b1;
        }
        if (parentScope){
            return parentScope->isVar(n);
        }
        return false;
    }
    void freeall(){
        
        for (const auto& v:variables){
            std::string type = v.second->getType();
            if (type == "ptr"){
                v.second->getPTR().freemem();
                //println("Deleted ptr: " + v.first);
            }
            
        }
        
    }

private:
    bool sig;
    bool isMain;
    bool isLoop;
    std::shared_ptr<Scope> parentScope;
    tsl::ordered_map<std::string, std::shared_ptr<Variable>> variables;
    tsl::ordered_map<std::string, int> pointers;
};
#endif