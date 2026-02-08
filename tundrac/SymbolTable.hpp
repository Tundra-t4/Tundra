#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include "Symbol.hpp"
#include "ScopeManager.hpp"
#include "TypeSystem.hpp"
#include "AST.hpp"
#include <memory>
#include <stdexcept>
#include <sstream>

// Error reporting structure
struct SemanticError {
    SemanticError(std::string message, std::shared_ptr<ASTNode> node, size_t line, size_t column): message(message), node(node), line(line), column(column) {}
    std::string message;
    std::shared_ptr<ASTNode> node;
    size_t line;
    size_t column;
    
    std::string to_string() const {
        std::ostringstream oss;
        oss << "Semantic Error at line " << line << ", col " << column << ": " << message;
        return oss.str();
    }
};

// SymbolTable - Main symbol table for semantic analysis
// This is a proper compiler component that performs:
// - Symbol resolution
// - Type checking
// - Scope management
// - Lifetime/ownership tracking
class SymbolTable {
public:
    SymbolTable() : scope_manager_(), has_errors_(false) {
        initialize_builtin_symbols();
    }

    // Main entry point for symbol table construction
    void analyze(std::shared_ptr<BlockNode> root) {
        if (!root) {
            report_error("Root block is null", nullptr);
            return;
        }
        analyze_block(root, true);
    }

    // Error handling
    bool has_errors() const { return has_errors_; }
    const std::vector<SemanticError>& errors() const { return errors_; }
    
    void print_errors(std::ostream& os = std::cerr) const {
        for (const auto& error : errors_) {
            os << error.to_string() << std::endl;
        }
    }

    // Access to scope manager
    ScopeManager& scope_manager() { return scope_manager_; }
    const ScopeManager& scope_manager() const { return scope_manager_; }

private:
    ScopeManager scope_manager_;
    std::vector<SemanticError> errors_;
    bool has_errors_;
    TypeRegistry& type_registry_ = TypeRegistry::instance();

    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    
    void initialize_builtin_symbols() {
        // Built-in types are already in TypeRegistry
        
        // Add built-in constants
        auto true_sym = std::make_shared<Symbol>(
            "true", SymbolKind::CONSTANT,
            type_registry_.get_bool(), nullptr, SymbolFlags::CONST
        );
        scope_manager_.declare("true", true_sym);
        
        auto false_sym = std::make_shared<Symbol>(
            "false", SymbolKind::CONSTANT,
            type_registry_.get_bool(), nullptr, SymbolFlags::CONST
        );
        scope_manager_.declare("false", false_sym);
        
        // Add built-in types as symbols
        for (std::string type_name : {"i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64",
                                       "f32", "f64", "Bool", "Character", "RawString","void","none"}) {
            logat("Getting : " + type_name,"init_builtins");
            auto type = type_registry_.get_type(type_name);
            
            if (type) {
                logat("Got : " + type_name,"init_builtins");
                auto type_sym = std::make_shared<Symbol>(
                    type_name, SymbolKind::TYPE, type, nullptr, SymbolFlags::CONST
                );
                scope_manager_.declare(type_name, type_sym);
            }
        }
    }

    // ========================================================================
    // ERROR REPORTING
    // ========================================================================
    
    void report_error(const std::string& message, std::shared_ptr<ASTNode> node,std::vector<std::string> notes={},std::vector<std::string> hint={}) {
        has_errors_ = true;
        if (lpos.find(node->id) == lpos.end()){
            errors_.push_back(SemanticError(message,node,0,0));
            return;
        }
        displayError(message,lpos[node->id][0],lpos[node->id][1],lpos[node->id][2],gerr(notes),false,hint);
    }

    void report_warning(const std::string& message, std::shared_ptr<ASTNode> node,std::vector<std::string> notes={},std::vector<std::string> hint={}) {
        has_errors_ = true;
        displayError(message,lpos[node->id][0],lpos[node->id][1],lpos[node->id][2],gerr(notes),true,hint);
    }

    // ========================================================================
    // ANALYSIS FUNCTIONS
    // ========================================================================
    
    std::shared_ptr<Type> analyze_block(std::shared_ptr<BlockNode> block, bool is_main = false) {
        scope_manager_.push_scope();
        
        std::shared_ptr<Type> last_type = type_registry_.get_void();
        
        for (const auto& stmt : block->getStatements()) {
            last_type = analyze_node(stmt);
        }
        
        if (!is_main) {
            scope_manager_.pop_scope();
        }
        
        return last_type;
    }
    std::shared_ptr<Type> analyze_node(std::shared_ptr<ASTNode> node){
        auto type = _analyze_node(node);
        node->typeinfo = type;
        return type;
    }
    std::shared_ptr<Type> _analyze_node(std::shared_ptr<ASTNode> node) {
        if (!node) {
            return type_registry_.get_void();
        }

        // Type-based dispatch to specific analysis functions
        if (auto assign = std::dynamic_pointer_cast<AssignNode>(node)) {
            return analyze_assign(assign);
        }
        else if (auto strong_assign = std::dynamic_pointer_cast<StrongAssignNode>(node)) {
            return analyze_strong_assign(strong_assign);
        }
        else if (auto ident = std::dynamic_pointer_cast<IdentifierNode>(node)) {
            return analyze_identifier(ident);
        }
        else if (auto int_lit = std::dynamic_pointer_cast<IntLiteralNode>(node)) {
            return analyze_int_literal(int_lit);
        }
        else if (auto str_lit = std::dynamic_pointer_cast<StringLiteralNode>(node)) {
            return analyze_string_literal(str_lit);
        }
        else if (auto binop = std::dynamic_pointer_cast<BinOP>(node)) {
            return analyze_binop(binop);
        }
        else if (auto call = std::dynamic_pointer_cast<CallNode>(node)) {
            return analyze_call(call);
        }
        else if (auto if_node = std::dynamic_pointer_cast<IFNode>(node)) {
            return analyze_if(if_node);
        }
        else if (auto while_node = std::dynamic_pointer_cast<WhileNode>(node)) {
            return analyze_while(while_node);
        }
        else if (auto ret = std::dynamic_pointer_cast<RetNode>(node)) {
            return analyze_return(ret);
        }
        else if (auto func = std::dynamic_pointer_cast<MappedFunctionNode>(node)) {
            return analyze_function(func);
        }
        else if (auto struct_decl = std::dynamic_pointer_cast<StructDeclNode>(node)) {
            return analyze_struct_decl(struct_decl);
        }
        else if (auto struct_inst = std::dynamic_pointer_cast<StructInstanceNode>(node)) {
            return analyze_struct_instance(struct_inst);
        }
        else if (auto ptr = std::dynamic_pointer_cast<PointerNode>(node)) {
            return analyze_pointer(ptr);
        }
        else if (auto deref = std::dynamic_pointer_cast<DerefNode>(node)) {
            return analyze_deref(deref);
        }
        else if (auto block = std::dynamic_pointer_cast<BlockNode>(node)) {
            return analyze_block(block, false);
        }
        else if (auto externdecl = std::dynamic_pointer_cast<ExternNode>(node)) {
            return analyze_extern(externdecl);
        }
        else if (auto errornode = std::dynamic_pointer_cast<ErrorNode>(node)) {
            return analyze_ErrorNode(errornode);
        }

        
        
        // Add more node types as needed
        return type_registry_.get_void();
    }

    // ========================================================================
    // SPECIFIC NODE ANALYSIS
    // ========================================================================
    
    std::shared_ptr<Type> analyze_assign(std::shared_ptr<AssignNode> node) {
        auto var_name = node->getVarName();
        auto value_type = analyze_node(node->getValue());


        
        // Check if variable already exists
        auto existing = scope_manager_.lookup_local(var_name);
        if (existing) {
            // Updating existing variable
            if (!existing->is_mutable()) {
                report_error("Cannot reassign to immutable variable '" + var_name + "'", node);
                return type_registry_.get_void();
            }
            
            // Type checking
            if (!existing->type()->equals(*value_type)) {
                report_error("Type mismatch in assignment to '" + var_name + "': expected " +
                           existing->type()->to_string() + ", got " + value_type->to_string(), node);
            }
            
            // Update symbol
            auto updated_symbol = std::make_shared<Symbol>(
                var_name, SymbolKind::VARIABLE, value_type, node,
                existing->is_mutable() ? SymbolFlags::MUTABLE : SymbolFlags::CONST
            );
            scope_manager_.update(var_name, updated_symbol);
        } else {
            // New variable declaration
            SymbolFlags flags = node->isMutable() ? SymbolFlags::MUTABLE : SymbolFlags::CONST;
            auto symbol = std::make_shared<Symbol>(
                var_name, SymbolKind::VARIABLE, value_type, node, flags
            );
            
            if (!scope_manager_.declare(var_name, symbol)) {
                report_error("Variable '" + var_name + "' already declared in this scope", node);
            }
        }
        
        return value_type;
    }

    std::shared_ptr<Type> analyze_strong_assign(std::shared_ptr<StrongAssignNode> node) {
        // Strong assignment requires explicit type annotation
        // Similar to regular assign but with type checking against annotation
        return nullptr; // Simplified for now
    }

    std::shared_ptr<Type> analyze_identifier(std::shared_ptr<IdentifierNode> node) {
        auto name = node->getValue();
        auto symbol = scope_manager_.lookup(name);
        
        if (!symbol) {
            report_error("Undefined identifier '" + name + "'", node);
            return type_registry_.get_void();
        }
        
        // Track reference for lifetime analysis
        symbol->add_reference();
        
        return symbol->type();
    }


    std::shared_ptr<Type> analyze_int_literal(std::shared_ptr<IntLiteralNode> node) {
        // Default integer literals to i32
        return type_registry_.get_i32();
    }

    std::shared_ptr<Type> analyze_string_literal(std::shared_ptr<StringLiteralNode> node) {
        return type_registry_.get_string();
    }

    std::shared_ptr<Type> analyze_ErrorNode(std::shared_ptr<ErrorNode> node){
        return type_registry_.get_void();
    }

    std::shared_ptr<Type> analyze_binop(std::shared_ptr<BinOP> node) {
        auto left_type = analyze_node(node->getLeft());
        auto right_type = analyze_node(node->getRight());
        
        // Type checking for binary operations
        if (!left_type->equals(*right_type)) {
            report_error("Type mismatch in binary operation: " +
                       left_type->to_string() + " and " + right_type->to_string(), node);
        }
        
        auto op = node->getValue();
        
        // Comparison operators return bool
        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
            return type_registry_.get_bool();
        }
        
        // Arithmetic operators return the operand type
        return left_type;
    }

    std::shared_ptr<Type> analyze_call(std::shared_ptr<CallNode> node) {
        auto callee = analyze_node(node->getCallee());

        
        if (!callee->is_function()) {
            report_error("is not a function", node);
            return type_registry_.get_void();
        }
        
        auto func_type = std::dynamic_pointer_cast<FunctionType>(callee);

        
        // Check argument count
        const auto& param_types = func_type->param_types();
        auto args = node->getArgs();
        
        if (!func_type->is_variadic() && args.size() != param_types.size()) {
            report_error("Function expects " +
                       std::to_string(param_types.size()) + " arguments, got " +
                       std::to_string(args.size()), node);
        }
        
        // Check argument types
        for (int i = 0; i < std::min(args.size(), param_types.size()); ++i) {
            auto arg_type = analyze_node(args[i]);
            if (!arg_type->equals(*param_types[i])) {
                report_error("Argument " + std::to_string(i + 1) + " type mismatch: expected " +
                           param_types[i]->to_string() + ", got " + arg_type->to_string(), node);
            }
        }
        
        return func_type->return_type();
    }

    std::shared_ptr<Type> analyze_if(std::shared_ptr<IFNode> node) {
        auto cond_type = analyze_node(node->getMainBranch()->get_condition());
        
        if (!cond_type->equals(*type_registry_.get_bool())) {
            report_error("If condition must be of type Bool, got " + cond_type->to_string(), node);
        }
        
        auto then_type = analyze_node(node->getMainBranch()->getBody());
        /* ignore elifs for now */
        if (node->getElseBranch()) {
            auto else_type = analyze_node(node->getElseBranch()->getBody());
            // Both branches should return same type for if-expression
            if (!then_type->equals(*else_type)) {
                report_error("If branches return different types: " +
                           then_type->to_string() + " and " + else_type->to_string(), node);
            }
        }
        
        return then_type;
    }

    std::shared_ptr<Type> analyze_while(std::shared_ptr<WhileNode> node) {
        auto cond_type = analyze_node(node->getLoopExpr()->get_condition());
        
        if (!cond_type->equals(*type_registry_.get_bool())) {
            report_error("While condition must be of type Bool, got " + cond_type->to_string(), node);
        }
        
        scope_manager_.push_scope();
        scope_manager_.mark_loop_scope();
        analyze_node(node->getLoopExpr()->getBody());
        scope_manager_.pop_scope();
        
        return type_registry_.get_void();
    }

    std::shared_ptr<Type> analyze_return(std::shared_ptr<RetNode> node) {
        if (!scope_manager_.in_function_scope()) {
            report_error("Return statement outside of function", node);
        }
        
        return analyze_node(node->getValue());
    }

    std::shared_ptr<Type> analyze_function(std::shared_ptr<MappedFunctionNode> node) {
        // Build function type
        std::vector<std::shared_ptr<Type>> param_types;
        for (const auto& [name, type_any] : node->getParameters()) {
            param_types.push_back(analyze_node(type_any)); // Placeholder
        }
        
        auto return_type = analyze_node(node->ReturnType); // Extract from node
        logat("return type is: " + std::to_string(static_cast<int>(return_type->kind())),"ST");
        auto func_type = std::make_shared<FunctionType>(param_types, return_type,node->isvdic);
        
        // Declare function symbol
        if (node->isAssigned){
            auto func_symbol = std::make_shared<Symbol>(
                node->AssignedName, SymbolKind::FUNCTION, func_type, node, SymbolFlags::NONE
            );
            scope_manager_.declare(node->AssignedName, func_symbol);
        }
        if (node->getBody() || SCAST<BlockNode>(node->getBody())->StatementsLen() == 0){
        // Analyze function body in new scope
        scope_manager_.push_scope();
        scope_manager_.mark_function_scope();
        
        // Add parameters to scope
        for (const auto& [name, type_any] : node->getParameters()) {
            auto param_type = analyze_node(type_any);
            auto param_symbol = std::make_shared<Symbol>(
                name, SymbolKind::PARAMETER, param_type, nullptr, SymbolFlags::CONST
            );
            scope_manager_.declare(name, param_symbol);
        }


            analyze_node(node->getBody());
            scope_manager_.pop_scope();
        }
        
        return func_type;
    }

    std::shared_ptr<Type> analyze_extern(std::shared_ptr<ExternNode> node){
        for (int i = 0; i < node->getValue().size();i++){
            auto func = node->getValue()[i];
            func->isAssigned = true;
            func->AssignedName = node->getfnnames()[i];
            SCAST<MappedFunctionNode>(func)->isvdic = node->getfnnames()[i] == node->getvdic()[i];
            analyze_node(func);
        }
        return type_registry_.get_void();
    }

    std::shared_ptr<Type> analyze_struct_decl(std::shared_ptr<StructDeclNode> node) {
        // Build struct type
        std::vector<StructType::Field> fields;
        // Extract fields from node
        
        auto struct_type = std::make_shared<StructType>(node->getName(), fields);
        type_registry_.register_type(node->getName(), struct_type);
        
        auto struct_symbol = std::make_shared<Symbol>(
            node->getName(), SymbolKind::STRUCT, struct_type, node, SymbolFlags::CONST
        );
        scope_manager_.declare(node->getName(), struct_symbol);
        
        return struct_type;
    }

    std::shared_ptr<Type> analyze_struct_instance(std::shared_ptr<StructInstanceNode> node) {
        auto struct_type = analyze_node(node->getBase());
        
        if (!struct_type || !struct_type->is_struct()) {
            report_error("Undefined struct type", node);
            return type_registry_.get_void();
        }
        
        return struct_type;
    }

    std::shared_ptr<Type> analyze_pointer(std::shared_ptr<PointerNode> node) {
        auto pointee_type = analyze_node(node->getPointee());
        auto ptr_type = std::make_shared<PointerType>(pointee_type, false);
        
        // Mark ownership transfer
        if (auto ident = std::dynamic_pointer_cast<IdentifierNode>(node->getPointee())) {
            auto symbol = scope_manager_.lookup(ident->getValue());
            if (symbol) {
                symbol->add_flag(SymbolFlags::OWNED);
            }
        }
        
        return ptr_type;
    }

    std::shared_ptr<Type> analyze_deref(std::shared_ptr<DerefNode> node) {
        auto ptr_type = analyze_node(node->getPointer());
        
        if (!ptr_type->is_pointer()) {
            report_error("Attempting to dereference non-pointer type: " + ptr_type->to_string(), node);
            return type_registry_.get_void();
        }
        
        auto pointer_type = std::dynamic_pointer_cast<PointerType>(ptr_type);
        return pointer_type->pointee();
    }
};

#endif // SYMBOL_TABLE_HPP