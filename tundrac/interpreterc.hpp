#include "base.hpp"

#ifndef INTERPRETER_C
#define INTERPRETER_C
struct Object;
class MappedFunction;
class Tag;
class UnMappedFunction;
struct List;
struct TypeSafeList;
struct Tuple;
class StructDecl;
class StructInstance;
class Enum;
class EnumKey;
class Pointer;
class BorrowedPointer;
class Class;
class Module;


std::string getTypeName(const std::any& value,bool cb = false,bool tp=false);





// Template class for specific types

class Typing {
public:
    Typing(const std::string& name)
        : name(name){
    }
    std::shared_ptr<Typing> clone() {
        return std::make_shared<Typing>(*this);
    }

    std::string getName()  { return name; }





    std::any convert(const std::any& v,std::string typekey) {

    
    logat("Typing convert for std::any called for type: " + getTypeName(v) + " to " + typekey,"Typing.c");
    auto conv = converters.find(typekey);
    if (conv == converters.end()) {
        logat("Converter for type " + typekey + " not found!","Typing.c");
        return {};  // Return empty if no converter found
    }
    if (getTypeName(v) == typekey){
        return v;
    }
    
    
    if (v.type() == typeid(int64_t)){
        //println("int test payload");
        std::any test;
        if (typekey != "i32"){
        test = conv->second(1);
        } else {
            test = conv->second(int16_t(1));
        }
        if (v.type() == test.type()){
            //println("true");
            return v;
        }
    }
    return conv->second(v);
}

private:
    std::string name;
};


std::shared_ptr<Typing> getTyping(const std::string& typeName) {
    static tsl::ordered_map<std::string, std::shared_ptr<Typing>> typings;


    if (typings.empty()) {
        typings["i8"]  = std::make_shared<Typing>("i8");
        typings["i16"] = std::make_shared<Typing>("i16");
        typings["i32"] = std::make_shared<Typing>("i32");
        typings["i64"] = std::make_shared<Typing>("i64");
        typings["u8"]  = std::make_shared<Typing>("u8");
        typings["u16"] = std::make_shared<Typing>("u16");
        typings["u32"] = std::make_shared<Typing>("u32");
        typings["u64"] = std::make_shared<Typing>("u64");
        typings["f32"] = std::make_shared<Typing>("f32");
        typings["f64"] = std::make_shared<Typing>("f64");
        typings["RawString"] = std::make_shared<Typing>("RawString");
        typings["Number"] = std::make_shared<Typing>("Number");
        typings["char"] = std::make_shared<Typing>("char");
        typings["wchar"] = std::make_shared<Typing>("wchar");
    }

    auto it = typings.find(typeName);
    if (it != typings.end()) {
        return it->second->clone();
    }
    return nullptr;
}


int idctr = 0;
tsl::ordered_map<int,std::vector<int>> lpos; // line & pos for errorisms
class ASTNode {
public:
    ASTNode(): id(++idctr){}
    std::string getPointsTO(){return pointsTO;}
    void setPointsTO(std::string x){this->pointsTO = x;}
    virtual ~ASTNode() = default;
    int id = 0;
    std::string pointsTO;
};


// Further value structs to be used in the interpreter (can be struct or class)


struct Object {
public:
    Object(std::any value, std::string type,bool negative=false) : value(value), type(type), nve(negative) {
        if (value.type() == typeid(Object)){
            this->value = std::any_cast<Object>(value).GetStore();
        }
    }

    bool isString(){
        try {
            std::any_cast<std::string>(value);
            return true;
        } catch (const std::bad_any_cast&) {
            return false;
        }        
    }

    std::string getString() {
        try {
            return std::any_cast<std::string>(value);
        } catch (const std::bad_any_cast&) {
            std::string str = "Stored value is not a string.";
            str = str+ " Value is: " + value.type().name();
            throw std::runtime_error(str);
        }
    }
    std::string getType() const { return type;}
    std::any GetStore() {
        if (convertedvalue.has_value() == true){
            logat("free return","Object.GS");
            

            return convertedvalue;
        }
        logat("Beginning obj conversion","Object.GS");
        std::any ret = GetStoreFn();
        
        this->convertedvalue = ret;

        logat("Successful conversion","Object.GS");
        return ret;
    }
    double max(std::string type){
        if (type[0] == 'f'){
            
        }
        if (type[0] == 'u'){
            return static_cast<double>((1ULL  << std::stoi(type.substr(1))-1)*2 - 1);
        } else {
            return static_cast<double>((1ULL  << std::stoi(type.substr(1))-1) - 1);
        }
    } // better to just make a constants list instead of constantly calling
    std::any GetStoreFn() {
        std::vector<std::string> inttypeNames = {
        "i8", "i16", "i32", "i64",    // Signed integer types
        "u8", "u16", "u32", "u64",    // Unsigned integer types
        "f32", "f64",                 // Floating-point types
        "isize","usize"
        };
        auto typer = getTyping(type);  // Get the typing for the current type
        std::string valueStr;
        
        // Convert the value to a string if it's a string or using the convertToString function
        logat("isString: " + convertToString(isString()),"Object.GSF");
        auto iss = isString();
        if (iss) {
            valueStr = getString();
        } else {
            valueStr = convertToString(value);
        }
        logat("PD -> " +getTypeName(value)  + " -> " + type ,"Object.GSF");
        
        if (in(getTypeName(value),inttypeNames) == false){
            iss = true;
            
        }
        logat("isString: " + convertToString(iss) + "\n" + getTypeName(value),"Object.GSF");
        double converted;
        if (iss == false){
        try {



        // Try to convert the string to an integer
        converted = std::any_cast<double>(convertToDouble(valueStr));  // Attempt conversion to int
        } catch(...){
            logat("F","Object.GSF");
            converted= NULL;
        }
        logat("FD","Object.GSF");
        
        //std::cout << "max: " << type << ": " << max(type) << std::endl;
        if (in(type,inttypeNames) == true && converted > max(type)){
            throw std::runtime_error("Value exceeds the maximum of " + type + "\nMax of: " + convertToString(max(type)));
        }
        logat("FM","Object.GSF");
        

        if (nve == true){
            //println("NVEEE!");
            converted = -1*converted;
        }
        } else {
            converted = NULL;
        }
        //std::cout << nve << std::endl;
        std::any ret;
        //println("Just about to convert!");

        // Check if conversion to integer was successful
        if (converted == NULL && type != "f64" && type != "f32" && iss == true) {
            // If `*p` is non-null, conversion failed, treat it as a string
            //println("String detected, performing string conversion");
            logat("RawString detected, performing string conversion to " + type + " iss:" + std::to_string(iss),"Object.GSF");
            ret = typer->convert(std::any(valueStr),type);  // Convert the string using the appropriate typing
        } else if (type == "f64"){
            if (nve == true){
            
            return static_cast<double>(-1)*std::any_cast<double>(typer->convert(value,"f64"));
            } else {
                return typer->convert(value,"f64");
            }
        } else if (type == "f32"){
            if (nve == true){
            
            return static_cast<float>(-1)*std::any_cast<float>(typer->convert(value,"f32"));
            } else {
                return typer->convert(value,"f32");
            }
        } else {
            // Conversion succeeded, treat it as an integer
            logat("Integer detected, performing integer conversion to " + type,"Object.GSF");
            //println("Integer detected, performing integer conversion!");
            ret = typer->convert(std::any(converted),type);  // Convert the integer using the appropriate typing
        }
        

        //println("Conversion done!");

        return ret;
    }
    std::string printable(){
        std::string valueStr;
        if (isString() == true){
        valueStr = getString();
        } else {
            valueStr = convertToString(this->GetStore());
        }
        //println("RETURNED");
        return valueStr;
    }
    void setNVE(bool n){
        //println("NVE SET");
        
        this->nve = n;
    }
    void setType(std::string newtype){
        type = newtype;
    }
    std::any value;

private:
    
    std::string type;
    std::string strval;
    std::any convertedvalue;
    bool nve;
};




class StructDecl {
public:
    StructDecl( tsl::ordered_map<std::string,std::any> value,tsl::ordered_map<std::string,std::any> cons,tsl::ordered_map<std::string,std::any> fns={})
        : value(value), constants(cons), fns(fns){}
     tsl::ordered_map<std::string,std::any> getValue() { return value; }
     tsl::ordered_map<std::string,std::any> getConsts()  { return constants; }
     tsl::ordered_map<std::string,std::any> getFns()  { return fns; }
    void addstruct(std::string ls){
        this->listed_structs.push_back(ls);
    }
    std::vector<std::string> getls(){return listed_structs;}
    void setls(std::vector<std::string> sls){
        listed_structs = sls;
    }
private:
    tsl::ordered_map<std::string,std::any> value;
    tsl::ordered_map<std::string,std::any> constants;
    tsl::ordered_map<std::string,std::any> fns;
    std::vector<std::string> listed_structs = {};
};

class Class {
  public:
      Class(const tsl::ordered_map<std::string,std::any> value,tsl::ordered_map<std::string,std::any> cons)
          : value(value), constants(cons){}
      const tsl::ordered_map<std::string,std::any> getValue() const { return value; }
      const tsl::ordered_map<std::string,std::any> getConsts() const { return constants; }
  private:
      const tsl::ordered_map<std::string,std::any> value;
      tsl::ordered_map<std::string,std::any> constants;
  };

class ClassInstance {
  public:
      ClassInstance(const tsl::ordered_map<std::string,std::any> value,tsl::ordered_map<std::string,std::any> cons)
          : value(value), constants(cons){}
      const tsl::ordered_map<std::string,std::any> getValue() const { return value; }
      const tsl::ordered_map<std::string,std::any> getConsts() const { return constants; }
  private:
      const tsl::ordered_map<std::string,std::any> value;
      tsl::ordered_map<std::string,std::any> constants;
  };

class StructInstance {
    public:
    StructInstance( tsl::ordered_map<std::string,std::any> value, tsl::ordered_map<std::string,std::any> cons,tsl::ordered_map<std::string,std::any> fns={} )
        : value(value),cons(cons), Fns(fns){}
     tsl::ordered_map<std::string,std::any> getValue()  { return value; }
     tsl::ordered_map<std::string,std::any> getCons()  { return cons; }
    tsl::ordered_map<std::string,std::any> getFns()  { return Fns; }
    void setSN(std::string inp){
        SN = inp;
    }
    std::string getSN(){
        return SN;
    }
    private:
    tsl::ordered_map<std::string,std::any> value;
    tsl::ordered_map<std::string,std::any> cons;
    tsl::ordered_map<std::string,std::any> Fns;
    std::string SN;
};

struct TypeSafeList {
    public:
    TypeSafeList(tsl::ordered_map<int,std::any> list,std::any  type,int size): list(list),type(type), size(size){}
    std::any get(int index){
        if (index < 0){
            index = list.size() + index;
        }
        std::any found = list.find(index);
        auto f = std::any_cast<tsl::ordered_map<int, std::any>::iterator>(found);
        if (f == list.end()){
            throw std::runtime_error("Index " + convertToString(index) + " not found in list." );
        }
        return f->second;
    }
    void add(std::any value){
        if (compareAnyTy(type,value) == false){
            throw std::runtime_error("Type of value added to TypeSafe list should be: " + getTypeName(type) + ".\nType got: " + getTypeName(value));
        }
        this->list[list.size()] = value;}
    void remove(){this->list.erase(list.size()-1);}
    int len() {return size;}
    tsl::ordered_map<int,std::any> getlist(){return list;}
    bool getSZU() const {
        return SZU;
    }
    void setSZU(bool b){
        SZU = b;
    }
    std::any getType()const{return type;}
    void set(int index, std::any v){
        if (compareAnyTy(type,v) == false){
            throw std::runtime_error("Type of value added to TypeSafe list should be: " + getTypeName(type) + ".\nType got: " + getTypeName(v));
        }
        this->list[index] = v;
    }
    int size;
    private:
    tsl::ordered_map<int,std::any> list;
    std::any  type;
    
    bool SZU = false;
};


struct Tuple {
    public:
    Tuple(tsl::ordered_map<int,std::any> list,std::string type): list(list),type(type){}
    std::any get(int index){
        if (index < 0){
            index = list.size() + index;
        }
        std::any found = list.find(index);
        auto f = std::any_cast<tsl::ordered_map<int, std::any>::iterator>(found);
        if (f == list.end()){
            throw std::runtime_error("Index " + convertToString(index) + " not found in tuple." );
        }
        return f->second;
    }
    void set(int index,std::any value){
        if (list.find(index) == list.end()){
            throw std::runtime_error("Tuple index sizes are immutable.");
        } else if (getTypeName(value) != type && type!=""){
            throw std::runtime_error("Tuple is typed.");
        }
        this->list[index] = value;}
    int len() {return list.size();}
    std::string getType()const{return type;}
    tsl::ordered_map<int,std::any> getcomparer() const {
       return list;
    }
    private:
    tsl::ordered_map<int,std::any> list;
    std::string type;
};

struct List {
    public:
    List(tsl::ordered_map<int,std::any> list): list(list){}
    std::any get(int index){
        if (index < 0){
            index = list.size() + index;
        }
        std::any found = list.find(index);
        auto f = std::any_cast<tsl::ordered_map<int, std::any>::iterator>(found);
        if (f == list.end()){
            throw std::runtime_error("Index " + convertToString(index) + " not found in list." );
        }
        return f->second;
    }
    void add(std::any value){this->list[list.size()] = value;}
    void remove(){this->list.erase(list.size()-1);}
    int len() {return list.size();}
    std::vector<std::string> StringRep(){
        std::vector<std::string> SR;
        for (auto& v : list){
            SR.push_back(convertToString(v.second));
        }
    }
    private:
    tsl::ordered_map<int,std::any> list;
};

struct Break {
    public:
    Break(std::any value=std::any()): value(value) {}
    std::any getValue(){return value;}
    private:
    std::any value;
};

struct Return {
    public:
    Return(std::any value=std::any()): value(value) {}
    std::any getValue(){return value;}
    private:
    std::any value;
};



class TypeEnumKey {
    public:
    TypeEnumKey(std::string id): id(id){}
    std::string getId() const {return id;}
    private:
    std::string id;
};
// EnumVariant - baseenum: my_data, variant: rect, values: (1,2)
class EnumVariant{
    public:
    EnumVariant(tsl::ordered_map<int,std::string> data): data(data){}
    tsl::ordered_map<int,std::string> getData() const {return data;}
    void addStore(int index,std::any v){
        this->store[index] = v;
    }
    void dumptemp(tsl::ordered_map<std::string, std::string> temps){
        tsl::ordered_map<int,std::string> tdata;
        for (auto&k:data){
            //println("1: " + k.second + "\n2. " + temps.find(k.second)->second);
            tdata[k.first] = temps.find(k.second)->second;
        }
        this->data = tdata;
    }
    std::vector<std::string> checktypes() {
        std::vector<std::string> ct;
        for (auto& v: data){
            ct.push_back(getTypeName(v.second));
        }
        return ct;
    }
    private:
    tsl::ordered_map<int,std::string> data;
    tsl::ordered_map<int,std::any> store;
};
 // need a node to interpret this
class FixedDataEnumKey{
    public:
    FixedDataEnumKey(tsl::ordered_map<int,std::string> data): data(data){}
    tsl::ordered_map<int,std::string> getData() const {return data;}
    void addStore(int index,std::any v){
        this->store[index] = v;
    }
    private:
    tsl::ordered_map<int,std::string> data;
    tsl::ordered_map<int,std::any> store;
};




class Enum {
    public:

    Enum(std::unordered_map<std::string,EnumKey> fields): fields(std::move(fields)){}
    std::unordered_map<std::string,EnumKey> getValue() const {return fields;}

    // Copy constructor (shallow copy, or deep copy if needed)
    Enum(const Enum& other) : fields(other.fields) {}

    // Copy assignment operator
    Enum& operator=(const Enum& other) {
        if (this != &other) {
            fields = other.fields;  // Assign the unordered_map
        }
        return *this;
    }

    // Move constructor (optional, for efficiency)
    Enum(Enum&& other) noexcept : fields(std::move(other.fields)) {}

    // Move assignment operator (optional, for efficiency)
    Enum& operator=(Enum&& other) noexcept {
        if (this != &other) {
            fields = std::move(other.fields);
        }
        return *this;
    }

    private:
    std::unordered_map<std::string,EnumKey> fields;

};

class EnumKey {
    public:
    EnumKey(std::string type, std::vector<std::any> value,Enum base=Enum({}),std::string name = ""): type(type), value(value),base(base),name(name) {}
    // Copy constructor
    EnumKey(const EnumKey& other)
        : type(other.type), value(other.value), base(other.base), name(other.name) {}

    // Copy assignment operator
    EnumKey& operator=(const EnumKey& other) {
        if (this != &other) {
            type = other.type;
            value = other.value;
            base = other.base; // Requires the copy assignment of Enum
            name = other.name;
        }
        return *this;
    }
    void setBase(Enum& e){
        this->base = e;
    }

    std::string getName(){return name;}
    std::vector<std::any> getValue() {return value;}
    Enum getBase() const {return base;}
    const std::string getType() const {return type;}
    private:
    std::string name;
    std::string type;
    std::vector<std::any> value;
    Enum base;



};

class PtrVal {
public:
    PtrVal(std::any value) : value(value) {}

    std::any getValue() const {
        
        return value;
    }

    void setValue(std::any v) {
        if (v.type() != value.type() && v.type() != typeid(nullptr)) {
            throw std::runtime_error("Type mismatch expected:: " + demangle(value.type().name()) + "\ngot: " + demangle(v.type().name()));
        }
        this->value = v;
    }
    void CastsetValue(std::any v) {
        this->value = v;
    }

private:
    std::any value;
};


class Pointer {
public:
    // Constructor
    Pointer(PtrVal* ptr) : ptr(ptr) {}
    

    bool getMut() const {
        return mut;
    }

    void setMut(bool t) {
        this->mut = t;
    }

    // Return the pointer to PtrVal
    PtrVal* getptr() const {
        if (freed == true){
            throw std::runtime_error("POINTER IS ALREADY FREED.");
        }
        return ptr;
    }

    // Free the allocated memory
    void freemem() {
        if (freed == false){
        if (ptr){
        if (ptr != nullptr){
        //delete ptr;
        freed = true;
        ptr = nullptr;
        }
        }
        }
    }
    void setFree(){
        this->freed = true;
    }

    // Clone the Pointer (shallow copy of PtrVal)
    Pointer clone() {
        return Pointer(new PtrVal(ptr->getValue()));  // This creates a shallow copy of the pointer
    }

    // Edit the PtrVal
    void editValue(std::any v) {
        ptr->setValue(v);
    }
    void pointsto(std::string v){
        //println("POINTING TO: " + v);
        
        this->pointfrom = v;
        //exit(0);
    }
    std::string ptrType(){
        return getTypeName(ptr->getValue());
    }
    std::string getpt(){return pointfrom;}

private:
    PtrVal* ptr;
    bool mut = false;
    bool freed = false;
    std::string pointfrom;
};

// BorrowedPointer class
class BorrowedPointer {
public:
    // Constructor takes a reference to PtrVal and a flag for mutability
    BorrowedPointer(PtrVal* ref, bool isMutable) : ptr(ref), mutableFlag(isMutable) {}

    // Return the value stored in PtrVal
    std::any getValue() const {
        return ptr->getValue();
    }

    std::string ptrType(){
        return getTypeName(ptr->getValue());
    }    

    // Return the reference to PtrVal
    PtrVal* getptr() const {
        return ptr;
    }

    // Edit the value if the pointer is mutable
    void editValue(std::any v) {
        if (!mutableFlag) {
            throw std::runtime_error("Cannot edit: Pointer is immutable.");
        }
        ptr->setValue(v);
    }
    bool isMut()const{return mutableFlag;}

private:
    PtrVal* ptr;  // Raw pointer to PtrVal
    bool mutableFlag;

};

class UnMappedFunction {
public:
    UnMappedFunction(const std::shared_ptr<ASTNode> value,tsl::ordered_map<int,std::string> args)
        : value(value), args(args){}
    const std::shared_ptr<ASTNode> getBody() const { return value; }
    const tsl::ordered_map<int,std::string> getargs() const { return args; }
private:
    const std::shared_ptr<ASTNode> value;
    tsl::ordered_map<int,std::string> args;
};

class Searchable{
    public:
    Searchable(std::string type, tsl::ordered_map<int,std::any> tochecks): type(type), tochecks(tochecks) {}
    std::string getType(){return type;}
    tsl::ordered_map<int,std::any> getToChecks(){return tochecks;}
    private:
    std::string type;
    tsl::ordered_map<int,std::any> tochecks;

};

class Future;

class MappedFunction {
public:
    MappedFunction(const std::shared_ptr<ASTNode> value, tsl::ordered_map<std::string,std::any> args,tsl::ordered_map<int,std::string> tick,std::any retty,tsl::ordered_map<std::string,std::string> internals,bool isselfptr=false)
        : value(value), args(args), tick(tick),retty(retty), internals(internals),isselfptr(isselfptr){}
    const std::shared_ptr<ASTNode> getBody() const { return value; }
    tsl::ordered_map<std::string,std::any> getargs() { return args; }
    tsl::ordered_map<int,std::string> gettick() {
        return tick;
    }
    tsl::ordered_map<std::string,std::string> getinternals() {
        return internals;
    }
    std::any getretty() const {
        return retty;
    }
    void setselfupdate(std::string s){
        this->updateself = s;
    }
    void setself(std::any v){
        args["self"] = v;
    }
    void removeself(){
        args.erase("self");
    }
    std::string getselfupdate(){
        return updateself;
    }
    void setselforigin(std::shared_ptr<ASTNode> slf){
        selforigin = slf;
    }
    std::shared_ptr<ASTNode> getselforigin(){
        return selforigin;
    }
    bool issignature = false;
    void setselfptr(bool s){
        this->isselfptr = s;
    }
    bool isselfptr = false;
private:
    const std::shared_ptr<ASTNode> value;
    std::shared_ptr<ASTNode> selforigin=nullptr;
    tsl::ordered_map<std::string,std::any> args;
    tsl::ordered_map<int,std::string> tick;
    tsl::ordered_map<std::string,std::string> internals;
    std::any retty;
    std::string updateself;
};


class Tag {
    public:
    Tag(tsl::ordered_map<int,std::any> tagtypes):tagtypes(tagtypes){}
    
    tsl::ordered_map<int,std::any> tagtypes;
    private:
};

class CFunction {
public:
    // Change constructor to take std::function by value
    CFunction(std::function<void(std::string)> value)
        : value(std::move(value)) {}

    const std::function<void(std::string)>& getFN() const { return value; }

    void call(std::string arg) {
        value(arg);  // Call the stored function
    }

private:
    std::function<void(std::string)> value;  // Store by value
};

class CEXTFunction {
public:
    // Change constructor to take std::function by value
    CEXTFunction(std::any value,MappedFunction MFNlink)
        : value(value), MFNlink(MFNlink) {
            if (!value.has_value()){
                throw std::runtime_error("Invalid value set");
            }
        }

    std::any getDV() { return value; }
    MappedFunction MFNlink;



private:
    std::any value;  // Store by value
};

class TagFunction {
public:
    // Change constructor to take std::function by value
    TagFunction(MappedFunction MFNlink,std::shared_ptr<ASTNode> AN)
        : MFNlink(MFNlink), AN(AN) {
        }

    MappedFunction MFNlink;
    std::shared_ptr<ASTNode> AN;



private:
    std::any value;  // Store by value
};

class Module {
    public:
    Module(std::string path){
    path += ".t4";
    if (AP.has("t4path")){
        for (auto& dp : AP.values["t4path"]){
            //std::cout << "searching " << dp + "/" + path<< std::endl;
            if (std::filesystem::exists(dp + "/" + path)){
                path = std::filesystem::absolute(dp + "/" + path).string();
                break;
            }
        }
    } else {
        path = std::filesystem::absolute(path).string();
    }
    //std::cout << "unbundling " << path << std::endl;
    logat("unbundling " + path,"Module");
    bundler.unbundle(path,&imports,&ocontent);
    hash = imports["t4hash"];
    objects[hash] = ocontent;
    imports.erase("t4hash");
    }
    void addMethod(std::string name, std::any method){
        methods[name] = method;
    }

    void addNode(std::string name, std::vector<std::shared_ptr<ASTNode>> nodes){
        MNodes[name] = nodes;
    }
   
    ObjectBundler bundler = ObjectBundler();
    std::string hash;
    std::vector<std::string> from;
    std::unordered_map<std::string,std::string> imports;
    std::unordered_map<std::string, std::any> methods;
    std::unordered_map<std::string,std::vector<std::shared_ptr<ASTNode>>> MNodes;
    std::vector<char> ocontent;
    private:

};

class ModuleMethod {
     public:
        ModuleMethod(std::string hash, std::string vname, std::any mfn) : hash(hash), vname(vname), mfn(mfn){}
        std::string hash;
        std::string vname;
        std::any mfn;
    private:
        
};


int vidct = 0;

#endif