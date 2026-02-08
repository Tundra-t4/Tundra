#ifndef TYPE_SYSTEM_HPP
#define TYPE_SYSTEM_HPP
#define PTYPE_SYSTEM_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include "llvm/IR/BasicBlock.h"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include <llvm/IR/InlineAsm.h>
#include "llvm/IR/Module.h"

#include "llvm/IR/Type.h"

#include <llvm/IR/Metadata.h>

// Forward declaration
class Type;

// Type kinds
enum class TypeKind {
    PRIMITIVE,
    POINTER,
    REFERENCE,
    ARRAY,
    STRUCT,
    ENUM,
    FUNCTION,
    TUPLE,
    VOID,
    GENERIC,
    UNKNOWN
};

// Base Type class
class Type {
public:
    explicit Type(TypeKind kind) : kind_(kind) {}
    virtual ~Type() = default;

    TypeKind kind() const { return kind_; }
    
    bool is_primitive() const { return kind_ == TypeKind::PRIMITIVE; }
    bool is_pointer() const { return kind_ == TypeKind::POINTER; }
    bool is_reference() const { return kind_ == TypeKind::REFERENCE; }
    bool is_array() const { return kind_ == TypeKind::ARRAY; }
    bool is_struct() const { return kind_ == TypeKind::STRUCT; }
    bool is_function() const { return kind_ == TypeKind::FUNCTION; }
    bool is_void() const { return kind_ == TypeKind::VOID; }
    
    virtual std::string to_string() const = 0;
    virtual bool equals(const Type& other) const = 0;
    virtual size_t size_bytes() const = 0;
    virtual std::shared_ptr<Type> clone() const = 0;
    virtual llvm::Type* llvm_type(llvm::LLVMContext& ctx)  = 0;

protected:
    TypeKind kind_;
};

// Primitive types (i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, bool, char)
class PrimitiveType : public Type {
public:
    enum class Primitive {
        I8, I16, I32, I64,
        U8, U16, U32, U64,
        F32, F64,
        BOOL,
        CHAR,
        STRING
    };

    explicit PrimitiveType(Primitive prim) 
        : Type(TypeKind::PRIMITIVE), primitive_(prim) {}

    Primitive primitive() const { return primitive_; }
    
    bool is_signed_int() const {
        return primitive_ >= Primitive::I8 && primitive_ <= Primitive::I64;
    }
    
    bool is_unsigned_int() const {
        return primitive_ >= Primitive::U8 && primitive_ <= Primitive::U64;
    }
    
    bool is_integer() const {
        return is_signed_int() || is_unsigned_int();
    }
    
    bool is_float() const {
        return primitive_ == Primitive::F32 || primitive_ == Primitive::F64;
    }

    std::string to_string() const override {
        switch (primitive_) {
            case Primitive::I8: return "i8";
            case Primitive::I16: return "i16";
            case Primitive::I32: return "i32";
            case Primitive::I64: return "i64";
            case Primitive::U8: return "u8";
            case Primitive::U16: return "u16";
            case Primitive::U32: return "u32";
            case Primitive::U64: return "u64";
            case Primitive::F32: return "f32";
            case Primitive::F64: return "f64";
            case Primitive::BOOL: return "Bool";
            case Primitive::CHAR: return "Character";
            case Primitive::STRING: return "RawString";
        }
        return "unknown";
    }

    bool equals(const Type& other) const override {
        if (!other.is_primitive()) return false;
        return primitive_ == static_cast<const PrimitiveType&>(other).primitive_;
    }

    size_t size_bytes() const override {
        switch (primitive_) {
            case Primitive::I8:
            case Primitive::U8:
            case Primitive::BOOL:
            case Primitive::CHAR:
                return 1;
            case Primitive::I16:
            case Primitive::U16:
                return 2;
            case Primitive::I32:
            case Primitive::U32:
            case Primitive::F32:
                return 4;
            case Primitive::I64:
            case Primitive::U64:
            case Primitive::F64:
                return 8;
            case Primitive::STRING:
                return sizeof(void*); // Pointer to string data
        }
        return 0;
    }

    std::shared_ptr<Type> clone() const override {
        return std::make_shared<PrimitiveType>(primitive_);
    }

    llvm::Type* llvm_type(llvm::LLVMContext& ctx) override {
        switch (primitive_){
            case Primitive::BOOL:
                return llvm::Type::getInt1Ty(ctx);
            case Primitive::I8: /* byte basically */
            case Primitive::U8:
            case Primitive::CHAR:
                return llvm::Type::getInt8Ty(ctx);
            case Primitive::I16:
            case Primitive::U16:
                return llvm::Type::getInt16Ty(ctx);
            case Primitive::I32:
            case Primitive::U32:
                return llvm::Type::getInt32Ty(ctx);
            case Primitive::I64:
            case Primitive::U64:
                return llvm::Type::getInt64Ty(ctx);
            case Primitive::F32:
                return llvm::Type::getFloatTy(ctx);
            case Primitive::F64:
                return llvm::Type::getDoubleTy(ctx);
            case Primitive::STRING:
                return llvm::PointerType::get(ctx, 0); // String is technically a pointer ahhh so no specific type, opaque
                
        }
    }

private:
    Primitive primitive_;
};

// Void type
class VoidType : public Type {
public:
    VoidType() : Type(TypeKind::VOID) {}

    std::string to_string() const override { return "void"; }
    
    bool equals(const Type& other) const override {
        return other.is_void();
    }

    size_t size_bytes() const override { return 0; }

    std::shared_ptr<Type> clone() const override {
        return std::make_shared<VoidType>();
    }

    llvm::Type* llvm_type(llvm::LLVMContext& ctx) override {
        return llvm::Type::getVoidTy(ctx);
    }
};

// Pointer type
class PointerType : public Type {
public:
    explicit PointerType(std::shared_ptr<Type> pointee, bool is_borrowed = false)
        : Type(TypeKind::POINTER), pointee_(pointee), is_borrowed_(is_borrowed) {}

    std::shared_ptr<Type> pointee() const { return pointee_; }
    bool is_borrowed() const { return is_borrowed_; }

    std::string to_string() const override {
        return (is_borrowed_ ? "&b " : "&") + pointee_->to_string();
    }

    bool equals(const Type& other) const override {
        if (!other.is_pointer()) return false;
        const auto& ptr = static_cast<const PointerType&>(other);
        return is_borrowed_ == ptr.is_borrowed_ && pointee_->equals(*ptr.pointee_);
    }

    size_t size_bytes() const override {
        return sizeof(void*);
    }

    std::shared_ptr<Type> clone() const override {
        return std::make_shared<PointerType>(pointee_->clone(), is_borrowed_);
    }

    llvm::Type* llvm_type(llvm::LLVMContext& ctx) override {
        return llvm::PointerType::get(ctx,0);
    }

private:
    std::shared_ptr<Type> pointee_;
    bool is_borrowed_;
};

// Array/List type
class ArrayType : public Type {
public:
    explicit ArrayType(std::shared_ptr<Type> element_type, std::optional<size_t> size = std::nullopt)
        : Type(TypeKind::ARRAY), element_type_(element_type), size_(size) {}

    std::shared_ptr<Type> element_type() const { return element_type_; }
    std::optional<size_t> size() const { return size_; }
    bool is_sized() const { return size_.has_value(); }

    std::string to_string() const override {
        if (size_) {
            return "[" + element_type_->to_string() + ":" + std::to_string(*size_) + "]";
        }
        return "[" + element_type_->to_string() + "]";
    }

    bool equals(const Type& other) const override {
        if (!other.is_array()) return false;
        const auto& arr = static_cast<const ArrayType&>(other);
        return element_type_->equals(*arr.element_type_) && size_ == arr.size_;
    }

    size_t size_bytes() const override {
        if (size_) {
            return element_type_->size_bytes() * (*size_);
        }
        return sizeof(void*); // Dynamic array is a pointer
    }

    std::shared_ptr<Type> clone() const override {
        return std::make_shared<ArrayType>(element_type_->clone(), size_);
    }

    llvm::Type* llvm_type(llvm::LLVMContext& ctx) override {
        return llvm::ArrayType::get(element_type_->llvm_type(ctx),size_bytes());
    }

private:
    std::shared_ptr<Type> element_type_;
    std::optional<size_t> size_;
};

// Tuple type
class TupleType : public Type {
public:
    explicit TupleType(std::vector<std::shared_ptr<Type>> elements)
        : Type(TypeKind::TUPLE), elements_(std::move(elements)) {}

    const std::vector<std::shared_ptr<Type>>& elements() const { return elements_; }

    std::string to_string() const override {
        std::string result = "(";
        for (size_t i = 0; i < elements_.size(); ++i) {
            result += elements_[i]->to_string();
            if (i < elements_.size() - 1) result += ", ";
        }
        result += ")";
        return result;
    }

    bool equals(const Type& other) const override {
        if (other.kind() != TypeKind::TUPLE) return false;
        const auto& tuple = static_cast<const TupleType&>(other);
        if (elements_.size() != tuple.elements_.size()) return false;
        for (size_t i = 0; i < elements_.size(); ++i) {
            if (!elements_[i]->equals(*tuple.elements_[i])) return false;
        }
        return true;
    }

    size_t size_bytes() const override {
        size_t total = 0;
        for (const auto& elem : elements_) {
            total += elem->size_bytes();
        }
        return total;
    }

    std::shared_ptr<Type> clone() const override {
        std::vector<std::shared_ptr<Type>> cloned;
        for (const auto& elem : elements_) {
            cloned.push_back(elem->clone());
        }
        return std::make_shared<TupleType>(std::move(cloned));
    }

    llvm::Type* llvm_type(llvm::LLVMContext& ctx) override {
        std::vector<llvm::Type*> types;
        for (auto& type: elements_){
            types.push_back(type->llvm_type(ctx));
        }
        return llvm::StructType::get(ctx,types,false); /* see if packing is wanted */
    }

private:
    std::vector<std::shared_ptr<Type>> elements_;
};

// Function type
class FunctionType : public Type {
public:
    explicit FunctionType(std::vector<std::shared_ptr<Type>> param_types,
                          std::shared_ptr<Type> return_type,
                          bool is_variadic = false)
        : Type(TypeKind::FUNCTION)
        , param_types_(std::move(param_types))
        , return_type_(return_type)
        , is_variadic_(is_variadic) {}

    const std::vector<std::shared_ptr<Type>>& param_types() const { return param_types_; }
    std::shared_ptr<Type> return_type() const { return return_type_; }
    bool is_variadic() const { return is_variadic_; }

    std::string to_string() const override {
        std::string result = "@sig(";
        for (size_t i = 0; i < param_types_.size(); ++i) {
            result += param_types_[i]->to_string();
            if (i < param_types_.size() - 1) result += ", ";
        }
        if (is_variadic_) result += ", ...";
        result += ") -> " + return_type_->to_string();
        return result;
    }

    bool equals(const Type& other) const override {
        if (!other.is_function()) return false;
        const auto& func = static_cast<const FunctionType&>(other);
        if (param_types_.size() != func.param_types_.size()) return false;
        if (is_variadic_ != func.is_variadic_) return false;
        for (size_t i = 0; i < param_types_.size(); ++i) {
            if (!param_types_[i]->equals(*func.param_types_[i])) return false;
        }
        return return_type_->equals(*func.return_type_);
    }

    size_t size_bytes() const override {
        return sizeof(void*); // Function pointer
    }

    std::shared_ptr<Type> clone() const override {
        std::vector<std::shared_ptr<Type>> cloned_params;
        for (const auto& param : param_types_) {
            cloned_params.push_back(param->clone());
        }
        return std::make_shared<FunctionType>(
            std::move(cloned_params),
            return_type_->clone(),
            is_variadic_
        );
    }

    llvm::Type* llvm_type(llvm::LLVMContext& ctx) override {
        std::vector<llvm::Type*> types;
        for (auto& type: param_types_){
            types.push_back(type->llvm_type(ctx));
        }
        return llvm::FunctionType::get(return_type_->llvm_type(ctx),types,is_variadic_);
    }

private:
    std::vector<std::shared_ptr<Type>> param_types_;
    std::shared_ptr<Type> return_type_;
    bool is_variadic_;
};

// Struct type
class StructType : public Type {
public:
    struct Field {
        std::string name;
        std::shared_ptr<Type> type;
    };

    explicit StructType(const std::string& name, std::vector<Field> fields)
        : Type(TypeKind::STRUCT), name_(name), fields_(std::move(fields)) {}

    const std::string& name() const { return name_; }
    const std::vector<Field>& fields() const { return fields_; }

    std::optional<std::shared_ptr<Type>> get_field_type(const std::string& field_name) const {
        for (const auto& field : fields_) {
            if (field.name == field_name) {
                return field.type;
            }
        }
        return std::nullopt;
    }

    std::string to_string() const override {
        return "struct " + name_;
    }

    bool equals(const Type& other) const override {
        if (!other.is_struct()) return false;
        const auto& str = static_cast<const StructType&>(other);
        return name_ == str.name_; // Structural equality can be name-based for now
    }

    size_t size_bytes() const override {
        size_t total = 0;
        for (const auto& field : fields_) {
            total += field.type->size_bytes();
        }
        return total;
    }

    std::shared_ptr<Type> clone() const override {
        std::vector<Field> cloned_fields;
        for (const auto& field : fields_) {
            cloned_fields.push_back({field.name, field.type->clone()});
        }
        return std::make_shared<StructType>(name_, std::move(cloned_fields));
    }

    llvm::Type* llvm_type(llvm::LLVMContext& ctx) override {
        std::vector<llvm::Type*> types;
        for (auto& type : fields_){
            types.push_back(type.type->llvm_type(ctx));
        }
        return llvm::StructType::get(ctx,types,false); /*pack option later*/
    }
    
private:
    std::string name_;
    std::vector<Field> fields_;
};

// Type registry for managing built-in and user-defined types
class TypeRegistry {
public:
    static TypeRegistry& instance() {
        static TypeRegistry registry;
        return registry;
    }

    void register_type(const std::string& name, std::shared_ptr<Type> type) {
        types_[name] = type;
    }

    std::shared_ptr<Type> get_type(const std::string& name) const {
        auto it = types_.find(name);
        //logat("Finding: " + name + " has: " + (has_type(name) ? "yes" : "no"),"get_type");
        return (it != types_.end()) ? it->second : nullptr;
    }

    bool has_type(const std::string& name) const {
        return types_.find(name) != types_.end();
    }

        // Helper methods to get primitive types
        std::shared_ptr<Type> get_i8() const { return get_type("i8"); }
        std::shared_ptr<Type> get_i16() const { return get_type("i16"); }
        std::shared_ptr<Type> get_i32() const { return get_type("i32"); }
        std::shared_ptr<Type> get_i64() const { return get_type("i64"); }
        // unsigned
        std::shared_ptr<Type> get_u8() const { return get_type("u8"); }
        std::shared_ptr<Type> get_u16() const { return get_type("u16"); }
        std::shared_ptr<Type> get_u32() const { return get_type("u32"); }
        std::shared_ptr<Type> get_u64() const { return get_type("u64"); }

        std::shared_ptr<Type> get_f32() const { return get_type("f32"); }
        std::shared_ptr<Type> get_f64() const { return get_type("f64"); }
        std::shared_ptr<Type> get_bool() const { return get_type("Bool"); }
        std::shared_ptr<Type> get_string() const { return get_type("RawString"); }
        std::shared_ptr<Type> get_void() const { return get_type("void"); }
        std::shared_ptr<Type> get_none() const { return get_type("void"); }

private:
    TypeRegistry() {
        // Register primitive types
        register_type("i8", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::I8));
        register_type("i16", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::I16));
        register_type("i32", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::I32));
        register_type("i64", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::I64));
        register_type("u8", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::U8));
        register_type("u16", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::U16));
        register_type("u32", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::U32));
        register_type("u64", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::U64));
        register_type("f32", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::F32));
        register_type("f64", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::F64));
        register_type("Bool", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::BOOL));
        register_type("Character", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::CHAR));
        register_type("RawString", std::make_shared<PrimitiveType>(PrimitiveType::Primitive::STRING));
        register_type("void", std::make_shared<VoidType>());
        register_type("none", std::make_shared<VoidType>());
    }

    std::unordered_map<std::string, std::shared_ptr<Type>> types_;
};


#endif // TYPE_SYSTEM_HPP