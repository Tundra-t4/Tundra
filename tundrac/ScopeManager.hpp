#ifndef SCOPE_MANAGER_HPP
#define SCOPE_MANAGER_HPP

#include "Symbol.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <stdexcept>
#include <optional>

// Scope represents a lexical scope in the program
class Scope {
public:
    explicit Scope(Scope* parent = nullptr, size_t depth = 0)
        : parent_(parent)
        , depth_(depth)
        , is_loop_scope_(false)
        , is_function_scope_(false)
    {}

    // Symbol management
    bool declare(const std::string& name, std::shared_ptr<Symbol> symbol) {
        if (symbols_.find(name) != symbols_.end()) {
            return false; // Symbol already declared in this scope
        }
        symbol->set_scope_depth(depth_);
        symbols_[name] = symbol;
        return true;
    }

    // Lookup in this scope only
    std::shared_ptr<Symbol> lookup_local(const std::string& name) const {
        auto it = symbols_.find(name);
        return (it != symbols_.end()) ? it->second : nullptr;
    }

    // Lookup in this scope and parent scopes
    std::shared_ptr<Symbol> lookup(const std::string& name) const {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            return it->second;
        }
        
        if (parent_) {
            return parent_->lookup(name);
        }
        
        return nullptr;
    }

    // Update an existing symbol
    bool update(const std::string& name, std::shared_ptr<Symbol> symbol) {
        auto existing = lookup(name);
        if (!existing) {
            return false; // Symbol not found
        }
        
        if (!existing->is_mutable()) {
            throw std::runtime_error("Cannot modify immutable symbol: " + name);
        }
        
        // Find which scope contains this symbol and update it
        if (symbols_.find(name) != symbols_.end()) {
            symbols_[name] = symbol;
            return true;
        } else if (parent_) {
            return parent_->update(name, symbol);
        }
        
        return false;
    }

    // Check if symbol exists in this scope or parent scopes
    bool exists(const std::string& name) const {
        return lookup(name) != nullptr;
    }

    // Get all symbols in this scope
    const std::unordered_map<std::string, std::shared_ptr<Symbol>>& symbols() const {
        return symbols_;
    }

    // Scope properties
    Scope* parent() const { return parent_; }
    size_t depth() const { return depth_; }
    
    bool is_loop_scope() const { return is_loop_scope_; }
    void set_loop_scope(bool value) { is_loop_scope_ = value; }
    
    bool is_function_scope() const { return is_function_scope_; }
    void set_function_scope(bool value) { is_function_scope_ = value; }

    // Get all symbols including parent scopes (for debugging/analysis)
    std::vector<std::shared_ptr<Symbol>> all_symbols() const {
        std::vector<std::shared_ptr<Symbol>> result;
        for (const auto& [name, symbol] : symbols_) {
            result.push_back(symbol);
        }
        if (parent_) {
            auto parent_symbols = parent_->all_symbols();
            result.insert(result.end(), parent_symbols.begin(), parent_symbols.end());
        }
        return result;
    }

private:
    Scope* parent_;
    size_t depth_;
    bool is_loop_scope_;
    bool is_function_scope_;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols_;
};

// ScopeManager manages the scope stack and provides scope operations
class ScopeManager {
public:
    ScopeManager() : current_scope_(nullptr), current_depth_(0) {
        // Create global scope
        push_scope();
    }

    ~ScopeManager() {
        // Clean up all scopes
        while (current_scope_) {
            pop_scope();
        }
    }

    // Scope stack operations
    void push_scope() {
        auto new_scope = std::make_unique<Scope>(current_scope_, current_depth_);
        current_scope_ = new_scope.get();
        scope_stack_.push_back(std::move(new_scope));
        ++current_depth_;
    }

    void pop_scope() {
        if (scope_stack_.empty()) {
            throw std::runtime_error("Cannot pop from empty scope stack");
        }
        
        // Cleanup: check for unreleased owned resources
        for (const auto& [name, symbol] : current_scope_->symbols()) {
            if (symbol->is_owned() && symbol->has_references()) {
                // Warning: owned resource still has references when going out of scope
                // In a full implementation, this would trigger lifetime checking errors
            }
        }
        
        scope_stack_.pop_back();
        current_scope_ = scope_stack_.empty() ? nullptr : scope_stack_.back().get();
        if (current_depth_ > 0) --current_depth_;
    }

    // Symbol operations on current scope
    bool declare(const std::string& name, std::shared_ptr<Symbol> symbol) {
        if (!current_scope_) {
            throw std::runtime_error("No active scope");
        }
        return current_scope_->declare(name, symbol);
    }

    std::shared_ptr<Symbol> lookup(const std::string& name) const {
        if (!current_scope_) {
            return nullptr;
        }
        return current_scope_->lookup(name);
    }

    std::shared_ptr<Symbol> lookup_local(const std::string& name) const {
        if (!current_scope_) {
            return nullptr;
        }
        return current_scope_->lookup_local(name);
    }

    bool update(const std::string& name, std::shared_ptr<Symbol> symbol) {
        if (!current_scope_) {
            throw std::runtime_error("No active scope");
        }
        return current_scope_->update(name, symbol);
    }

    bool exists(const std::string& name) const {
        return lookup(name) != nullptr;
    }

    // Scope properties
    Scope* current_scope() const { return current_scope_; }
    size_t current_depth() const { return current_depth_; }
    bool is_global_scope() const { return current_depth_ <= 1; }

    // Helper methods
    void mark_loop_scope() {
        if (current_scope_) {
            current_scope_->set_loop_scope(true);
        }
    }

    void mark_function_scope() {
        if (current_scope_) {
            current_scope_->set_function_scope(true);
        }
    }

    bool in_loop_scope() const {
        Scope* scope = current_scope_;
        while (scope) {
            if (scope->is_loop_scope()) return true;
            scope = scope->parent();
        }
        return false;
    }

    bool in_function_scope() const {
        Scope* scope = current_scope_;
        while (scope) {
            if (scope->is_function_scope()) return true;
            scope = scope->parent();
        }
        return false;
    }

    // Get global scope
    Scope* global_scope() const {
        return scope_stack_.empty() ? nullptr : scope_stack_[0].get();
    }

    // Statistics/debugging
    size_t total_symbols() const {
        size_t count = 0;
        if (current_scope_) {
            auto symbols = current_scope_->all_symbols();
            count = symbols.size();
        }
        return count;
    }

private:
    std::vector<std::unique_ptr<Scope>> scope_stack_;
    Scope* current_scope_;
    size_t current_depth_;
};

#endif // SCOPE_MANAGER_HPP