#include "base.hpp"
#include "interpreterc.hpp"
#include "Scope.hpp"
#include "Tokens.hpp"
#include "Lexer.hpp"


#ifndef AST
#define AST

// AST NODES

class MappedFunctionNode : public ASTNode {
public:
    MappedFunctionNode(tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> value,std::shared_ptr<ASTNode> body, tsl::ordered_map<int, std::string>tick,tsl::ordered_map<std::string, std::string> internals,bool isselfptr=false) : value(value),body(body),tick(tick),internals(internals),isselfptr(isselfptr) {}
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> getValue() { return value; }
    std::shared_ptr<ASTNode> getBody() const { return body; }
    tsl::ordered_map<int, std::string> gettick(){
        return tick;
    }
    tsl::ordered_map<std::string, std::string> getinternals(){
        return internals;
    }
    void setself(std::shared_ptr<ASTNode> v){
        this->value["self"] = v;
    }
    void setNoneRet(bool b){
        this->isnoneret = b;
    }
    bool isselfptr= false;
    bool isnoneret=false;
private:
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> value;
    std::shared_ptr<ASTNode>body;
    tsl::ordered_map<int, std::string> tick;
    tsl::ordered_map<std::string, std::string> internals;

};

class UnMappedFunctionNode : public ASTNode {
public:
    UnMappedFunctionNode(tsl::ordered_map<int,std::string> value,std::shared_ptr<ASTNode> body) : value(value),body(body) {}
    tsl::ordered_map<int,std::string> getValue() const { return value; }
    std::shared_ptr<ASTNode> getBody() const { return body; }

private:
    tsl::ordered_map<int,std::string> value;
    std::shared_ptr<ASTNode>body;

};

class IntLiteralNode : public ASTNode {
public:
    IntLiteralNode(int32_t value) : value(value) {}

    IntLiteralNode(const std::string& str_value) {
        try {
            value = std::stoi(str_value);  
        } catch (const std::invalid_argument& e) {

            throw std::runtime_error("Invalid integer literal: " + str_value);
        } catch (const std::out_of_range& e) {

            throw std::runtime_error("Integer literal out of range: " + str_value);
        }
    }
    int getValue() const { return value; }

    void setValue(int value){
        this->value = value;
    }

private:
    int value;
};

class MemAccNode : public ASTNode {
public:
    MemAccNode(std::shared_ptr<ASTNode> value,std::shared_ptr<ASTNode> nxt,bool assign= false, std::shared_ptr<ASTNode> assignv = std::make_shared<ASTNode>()) : value(value), nxt(nxt),assign(assign),assignv(assignv) {}
    std::shared_ptr<ASTNode> getValue() { return value; }
    std::shared_ptr<ASTNode> getNxt() { return nxt; }
    bool getAssign(){return assign;}
    void setAssign(bool b){assign = b;}
    std::shared_ptr<ASTNode> getAssignv() { return assignv; }

private:
    std::shared_ptr<ASTNode> value;
    std::shared_ptr<ASTNode> nxt;
    bool assign = false;
    std::shared_ptr<ASTNode> assignv;
};

class ChainNode : public ASTNode {
    public:
    ChainNode(std::vector<std::shared_ptr<ASTNode>> vec):vec(vec){}
    std::vector<std::shared_ptr<ASTNode>> getChain(){
        return vec;
    }
    private:
    std::vector<std::shared_ptr<ASTNode>> vec;
};

class ImportNode : public ASTNode {
    public:
    ImportNode(std::string name,std::vector<std::string> from):name(name), from(from){}
    std::string getName(){
        return name;
    }
    std::vector<std::string> getFrom(){
        return from;
    }
    void setFrom(std::vector<std::string> f){
        this->from = f;
    }
    private:
    std::string name;
    std::vector<std::string> from;
};

class StringLiteralNode : public ASTNode {
public:
    StringLiteralNode(const std::string& value) : value(value) {}
    const std::string& getValue() const { return value; }

private:
    std::string value;
};

class MacroNode : public ASTNode {
public:
    MacroNode()  {}


};

class ONode : public ASTNode {
public:
    ONode(const Object& value) : value(value) {}
    Object getValue() { return value; }
    void setNVE() {value.setNVE(true);}

private:
    Object value;
};

class LoopNode : public ASTNode {
public:
    LoopNode(const std::shared_ptr<ASTNode>& value,const std::shared_ptr<ASTNode> times=std::make_shared<ASTNode>()) : body(value),times(times) {}
    const std::shared_ptr<ASTNode> getValue() const { return body; }
    const std::shared_ptr<ASTNode> getTimes() const {return times;}

private:
    std::shared_ptr<ASTNode> body;
    std::shared_ptr<ASTNode> times;
};

class GiveOwnershipNode: public ASTNode {
    public:
    GiveOwnershipNode(std::shared_ptr<ASTNode> expr): expr(expr){}
    std::shared_ptr<ASTNode> getExpr() {return expr;}
    
    private:
    std::shared_ptr<ASTNode> expr;
};

class ModifyPtrNode: public ASTNode {
    public:
    ModifyPtrNode(std::shared_ptr<ASTNode> lhs,std::shared_ptr<ASTNode> expr): lhs(lhs),expr(expr){}
    std::shared_ptr<ASTNode> getExpr() {return expr;}
    std::shared_ptr<ASTNode> getLHS() {return lhs;}
    
    private:
    std::shared_ptr<ASTNode> lhs;
    std::shared_ptr<ASTNode> expr;
};

class PointerNode : public ASTNode {
    public:
    PointerNode(std::shared_ptr<ASTNode> expr,bool mut=false,bool isborrow=false): expr(expr),mut(mut),isborrow(isborrow){}
    std::shared_ptr<ASTNode> getExpr(){return expr;}
    bool getMut(){return mut;}
    bool getBorrow(){return isborrow;}
    private:
    std::shared_ptr<ASTNode> expr;
    bool mut;
    bool isborrow;
};

class DerefNode : public ASTNode {
    public:
    DerefNode(std::shared_ptr<ASTNode>expr): expr(expr){}
    std::shared_ptr<ASTNode> getExpr(){return expr;}
    private:
    std::shared_ptr<ASTNode> expr;
};

class TupleNode : public ASTNode {
public:
    TupleNode(const tsl::ordered_map<int,std::shared_ptr<ASTNode>>& value,std::string type = "",std::shared_ptr<ASTNode> size=std::make_shared<ASTNode>()) : value(value),type(type),size(size) {}
    tsl::ordered_map<int,std::shared_ptr<ASTNode>> getValue() { return value; }
    std::string getType() {return type;}
    std::shared_ptr<ASTNode> getSize() {return size;}
    
private:
    tsl::ordered_map<int,std::shared_ptr<ASTNode>> value;
    std::string type;
    std::shared_ptr<ASTNode> size;
};

class ListNode : public ASTNode {
public:
    ListNode(const tsl::ordered_map<int,std::shared_ptr<ASTNode>>& value) : value(value) {}
    tsl::ordered_map<int,std::shared_ptr<ASTNode>> getValue() { return value; }
    

private:
    tsl::ordered_map<int,std::shared_ptr<ASTNode>> value;
};

class TypeSafeListNode : public ASTNode {
public:
    TypeSafeListNode(const tsl::ordered_map<int,std::shared_ptr<ASTNode>>& value,std::shared_ptr<ASTNode> type) : value(value),type(type) {}
    tsl::ordered_map<int,std::shared_ptr<ASTNode>> getValue() { return value; }
    std::shared_ptr<ASTNode>  getType() { return type; }
    
private:
    tsl::ordered_map<int,std::shared_ptr<ASTNode>> value;
    std::shared_ptr<ASTNode>  type;
};

class TSLInitNode : public ASTNode {
public:
    TSLInitNode(const std::shared_ptr<ASTNode>& value,const std::shared_ptr<ASTNode>& body)
        : value(value),body(body) {}
    const std::shared_ptr<ASTNode>& getExpr() const { 
        
        return value; }
    const std::shared_ptr<ASTNode>& getBody() const { return body; }

private:
    const std::shared_ptr<ASTNode> value;
    const std::shared_ptr<ASTNode> body;
};

class IndexNode : public ASTNode {
public:
    IndexNode(const std::shared_ptr<ASTNode>& value,std::shared_ptr<ASTNode> index) : value(value), index(index){}
    std::shared_ptr<ASTNode> getValue() { return value; }
    std::shared_ptr<ASTNode> getIndex() { return index; }

private:
    std::shared_ptr<ASTNode> value;
    std::shared_ptr<ASTNode> index;
};

class BinOP : public ASTNode {
public:
    BinOP(std::shared_ptr<ASTNode> left, std::shared_ptr<ASTNode> right, const std::string& exp) : exp(exp),right(right),left(left) {}
    const std::string& getValue() const { return exp; }
    const std::shared_ptr<ASTNode> getleft() const {return left;}
    const std::shared_ptr<ASTNode> getright() const {return right;}
private:
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;
    std::string exp;
};

class AssignNode : public ASTNode {
public:
    AssignNode(const std::string& varName, std::shared_ptr<ASTNode> value,bool mut=false)
        : varName(varName), value(value), mut(mut) {}

    const std::string& getVarName() const { return varName; }
    std::shared_ptr<ASTNode> getValue() const { return value; }
    bool getMut() const {return mut;}
    void setMut(bool v) {this->mut=v;}

private:
    std::string varName;
    std::shared_ptr<ASTNode> value;
    bool mut;
};

class ExprAssignNode : public ASTNode {
public:
    ExprAssignNode(std::shared_ptr<ASTNode> varName, std::shared_ptr<ASTNode> value,bool mut=false)
        : varName(varName), value(value), mut(mut) {}

    std::shared_ptr<ASTNode> getVarName() const { return varName; }
    std::shared_ptr<ASTNode> getValue() const { return value; }
    bool getMut() const {return mut;}
    void setMut(bool v) {this->mut=v;}

private:
    std::shared_ptr<ASTNode> varName;
    std::shared_ptr<ASTNode> value;
    bool mut;
};

class StrongAssignNode : public ASTNode {
public:
    StrongAssignNode(const std::string& varName, std::shared_ptr<ASTNode> value,std::shared_ptr<ASTNode> strongtype,bool mut=false)
        : varName(varName), value(value), strongtype(strongtype), mut(mut) {}

    const std::string& getVarName() const { return varName; }
    const std::shared_ptr<ASTNode> getType() const { return strongtype; }
    std::shared_ptr<ASTNode> getValue() const { return value; }
    bool getMut() const {return mut;}
    void setMut(bool v) {this->mut = v;}

private:
    std::string varName;
    std::shared_ptr<ASTNode> strongtype;
    std::shared_ptr<ASTNode> value;
    bool mut;
};


class IdentifierNode : public ASTNode {
public:
    IdentifierNode(const std::string value)
        : value(value) {}
    std::string getValue() const { return value; }

private:
    std::string value;
};


class ConventionNode : public ASTNode {
public:
    ConventionNode(std::string value,std::vector<std::shared_ptr<ASTNode>> pubs)
        : value(value), pubs(pubs) {}
    std::string getValue()   { return value; }
    std::vector<std::shared_ptr<ASTNode>> getPub() { return pubs;}
    void setPub(std::vector<std::shared_ptr<ASTNode>> sp){
        this->pubs = sp;
    }

private:
    std::string value;
    std::vector<std::shared_ptr<ASTNode>>pubs;
};

class RefNode : public ASTNode {
public:
    RefNode(std::shared_ptr<ASTNode>  value)
        : value(value) {}
    std::shared_ptr<ASTNode> getValue() const { return value; }

private:
    std::shared_ptr<ASTNode> value;
};

class TagNode : public ASTNode {
public:
    TagNode(std::shared_ptr<ASTNode>  value)
        : value(value) {}
    std::shared_ptr<ASTNode> getValue() const { return value; }

private:
    std::shared_ptr<ASTNode> value;
};

class SizeOfNode : public ASTNode {
public:
    SizeOfNode(std::shared_ptr<ASTNode>  value)
        : value(value) {}
    std::shared_ptr<ASTNode> getValue() const { return value; }

private:
    std::shared_ptr<ASTNode> value;
};

class StackSizeNode : public ASTNode {
public:
    StackSizeNode(){}
    

private:
};

class PtrtointNode : public ASTNode {
public:
    PtrtointNode(std::shared_ptr<ASTNode>  value)
        : value(value) {}
    std::shared_ptr<ASTNode> getValue() const { return value; }

private:
    std::shared_ptr<ASTNode> value;
};

// tsl::ordered_map<std::string,std::shared_ptr<ASTNode>>
class ASMNode : public ASTNode {
public:
    ASMNode(std::string ASMStr,std::vector<std::string> regs,std::unordered_map<std::string,std::shared_ptr<ASTNode>> in,std::unordered_map<std::string,std::shared_ptr<ASTNode>>out,std::unordered_map<std::string,std::shared_ptr<ASTNode>>inout,std::unordered_map<std::string,std::shared_ptr<ASTNode>>assignables)
        : ASMStr(ASMStr),regs(regs), in(in), out(out),inout(inout),assignables(assignables) {}


    std::string ASMStr;
    std::vector<std::string> regs;
    std::unordered_map<std::string,std::shared_ptr<ASTNode>> in;
    std::unordered_map<std::string,std::shared_ptr<ASTNode>> out;
    std::unordered_map<std::string,std::shared_ptr<ASTNode>> inout;
    std::unordered_map<std::string,std::shared_ptr<ASTNode>> assignables;
private:
};
class TypeIDNode : public ASTNode {
public:
    TypeIDNode(std::shared_ptr<ASTNode>  value)
        : value(value) {}
    std::shared_ptr<ASTNode> getValue() const { return value; }

private:
    std::shared_ptr<ASTNode> value;
};

class CastNode : public ASTNode {
public:
    CastNode(std::shared_ptr<ASTNode> value,std::shared_ptr<ASTNode> dest)
        : value(value), dest(dest) {}
    std::shared_ptr<ASTNode> getValue() { return value; }
    std::shared_ptr<ASTNode> getDest() { return dest; }

private:
    std::shared_ptr<ASTNode> value;
    std::shared_ptr<ASTNode> dest;
};

class PubNode : public ASTNode {
public:
    PubNode(std::shared_ptr<ASTNode> value)
        : value(value){}
    std::shared_ptr<ASTNode> getValue() { return value; }


private:
    std::shared_ptr<ASTNode> value;

};



class ExternNode : public ASTNode {
public:
    ExternNode(std::vector<std::shared_ptr<ASTNode>> value,std::vector<std::string> fnnames,std::vector<std::string> isvariadic)
        : value(value), fnnames(fnnames), isvariadic(isvariadic) {}
    std::vector<std::shared_ptr<ASTNode>> getValue() { return value; }
    std::vector<std::string> getfnnames() {return fnnames;}
    std::vector<std::string> getvdic() {return isvariadic;}

private:
    std::vector<std::shared_ptr<ASTNode>> value;
    std::vector<std::string> fnnames;
    std::vector<std::string> isvariadic;
};

class Fakepass : public ASTNode {
public:
    Fakepass(std::any value)
        : value(value) {}
    std::any getValue() const { return value; }

private:
    std::any value;
};

class FutureNode : public ASTNode {
  public:
      FutureNode(const std::string& value)
          : value(value) {}
      std::string getValue() const { return value; }

  private:
      std::string value;
  };

class InternalsNode : public ASTNode {
  public:
      InternalsNode(const std::string& value,const std::string& pub)
          : value(value),pub(pub){}
      std::string getValue() const { return value; }
      std::string getPub() const { return pub; }


  private:
      std::string value;

      std::string pub;
  };

class SearchableNode : public ASTNode {
public:
    SearchableNode(const std::string& value,tsl::ordered_map<int,std::shared_ptr<ASTNode>> tochecks)
        : value(value), tochecks(tochecks) {}
    std::string getValue() const { return value; }
    tsl::ordered_map<int,std::shared_ptr<ASTNode>> getChecks() const {return tochecks;}

private:
    std::string value;
    tsl::ordered_map<int,std::shared_ptr<ASTNode>> tochecks;
};

class MatchNode : public ASTNode {
public:
    MatchNode(const std::shared_ptr<ASTNode> value,tsl::ordered_map<std::shared_ptr<ASTNode>,std::shared_ptr<ASTNode>> ns,std::shared_ptr<ASTNode> elser=std::make_shared<ASTNode>())
        : value(value), ns(ns), elser(elser) {}
    std::shared_ptr<ASTNode> getValue() const { return value; }
    std::shared_ptr<ASTNode> getElse() const { return elser; }
    tsl::ordered_map<std::shared_ptr<ASTNode>,std::shared_ptr<ASTNode>> getns(){return ns;}

private:
    std::shared_ptr<ASTNode> elser;
    std::shared_ptr<ASTNode> value;
    tsl::ordered_map<std::shared_ptr<ASTNode>,std::shared_ptr<ASTNode>>ns;
};

class DeconsNode : public ASTNode {
public:
    DeconsNode(std::shared_ptr<ASTNode>value, std::vector<std::string> idents)
        : value(value), idents(idents) {}
    std::shared_ptr<ASTNode> getValue() { return value; }

    std::vector<std::string> getIdents() const {return idents;}

    void setValue(std::shared_ptr<ASTNode> val){
        this->value = val;
    }

private:
    std::shared_ptr<ASTNode> value;
    std::vector<std::string> idents;
};

class NUnararyNode : public ASTNode {
public:
    NUnararyNode(const std::shared_ptr<ASTNode> value, const std::shared_ptr<ASTNode> elser)
        : value(value), elser(elser) {}
    std::shared_ptr<ASTNode> getValue() const { return value; }
    std::shared_ptr<ASTNode> getElse() const { return elser; }

private:
    std::shared_ptr<ASTNode> value;
    std::shared_ptr<ASTNode> elser;
};

class IfLetNode : public ASTNode {
public:
    IfLetNode(const std::shared_ptr<ASTNode> value, const std::shared_ptr<ASTNode> body,const std::shared_ptr<ASTNode> elser)
        : value(value), body(body), elser(elser) {}
    std::shared_ptr<ASTNode> getValue() const { return value; }
    std::shared_ptr<ASTNode> getBody() const { return body; }
    std::shared_ptr<ASTNode> getElse() const { return elser; }

private:
    std::shared_ptr<ASTNode> value;
    std::shared_ptr<ASTNode> body;
    std::shared_ptr<ASTNode> elser;
};

class EnumNode : public ASTNode {
public:
    EnumNode(const tsl::ordered_map<std::string,std::vector<std::shared_ptr<ASTNode>>> value)
        : value(value) {}
    const tsl::ordered_map<std::string,std::vector<std::shared_ptr<ASTNode>>> getValue() const { return value; }

private:
    const tsl::ordered_map<std::string,std::vector<std::shared_ptr<ASTNode>>> value;
};

class DropNode : public ASTNode {
public:
    DropNode(std::shared_ptr<ASTNode> value)
        : value(value) {}
    std::shared_ptr<ASTNode> getValue() const { return value; }

private:
    std::shared_ptr<ASTNode> value;
};

class ModNode : public ASTNode {
public:
    ModNode(std::shared_ptr<ASTNode> value,std::vector<std::shared_ptr<ASTNode>> states)
        : value(value), states(states) {}
    std::shared_ptr<ASTNode> getValue() const { return value; }
    std::vector<std::shared_ptr<ASTNode>> getStates() const {return states;}

private:
    std::shared_ptr<ASTNode> value;
    std::vector<std::shared_ptr<ASTNode>> states;
};



class ExpressionNode : public ASTNode {
public:
    ExpressionNode(const std::shared_ptr<ASTNode>& value,const std::shared_ptr<ASTNode>& body)
        : value(value),body(body) {}
    const std::shared_ptr<ASTNode>& getExpr() const { 
        
        return value; }
    const std::shared_ptr<ASTNode>& getBody() const { return body; }

private:
    const std::shared_ptr<ASTNode> value;
    const std::shared_ptr<ASTNode> body;
};

class DecoratorNode : public ASTNode {
public:
    DecoratorNode(const std::shared_ptr<ASTNode>& value,const std::shared_ptr<ASTNode>& body)
        : value(value),body(body) {}
    const std::shared_ptr<ASTNode>& getClr() const { 
        
        return value; }
    const std::shared_ptr<ASTNode>& getFn() const { return body; }

private:
    const std::shared_ptr<ASTNode> value;
    const std::shared_ptr<ASTNode> body;
};

class AssertionNode : public ASTNode {
public:
    AssertionNode(std::shared_ptr<ASTNode> value)
        : value(value){}
    std::shared_ptr<ASTNode> getExpr() const { 
        
        return value; }


private:
    std::shared_ptr<ASTNode> value;

};

class TONode : public ASTNode {
public:
    TONode(const std::shared_ptr<ASTNode>& value)
        : value(value) {}
    const std::shared_ptr<ASTNode>& getExpr() const { 
        
        return value; }
    

private:
    const std::shared_ptr<ASTNode> value;
    
};

class CallNode : public ASTNode {
public:
    CallNode(tsl::ordered_map<int,std::shared_ptr<ASTNode>> value, std::shared_ptr<ASTNode> body,tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> inserts)
        : value(value),body(body),inserts(inserts) {}
     tsl::ordered_map<int,std::shared_ptr<ASTNode>> getExpr() { 
        //println(typeid(value).name());
        return value; }
     std::shared_ptr<ASTNode> getBody()  { return body; }
     void setBody(std::shared_ptr<ASTNode> b) {
        this->body = b;
    }
     void setExpr( tsl::ordered_map<int,std::shared_ptr<ASTNode>> v){
        this->value = v;
    }

    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> getinserts(){
        return inserts;
    }

private:
    tsl::ordered_map<int,std::shared_ptr<ASTNode>> value;
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> inserts;
    std::shared_ptr<ASTNode> body;
    
};


class StructDeclNode : public ASTNode {
    public:
    StructDeclNode( tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> value,tsl::ordered_map<std::string,std::shared_ptr<ASTNode>>  constants) : value(value), cons(constants) {}
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>>  getValue() {return value;}
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> getCons(){return cons;}
    private:
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>>  value;
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>>  cons;
};

class ClassDeclNode : public ASTNode {
    public:
    ClassDeclNode(const tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> value,tsl::ordered_map<std::string,std::shared_ptr<ASTNode>>  constants) : value(value), cons(constants) {}
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>>  getValue() {return value;}
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> getCons(){return cons;}
    private:
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>>  value;
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>>  cons;
};


class BreakNode : public ASTNode {
public:
    BreakNode(const std::shared_ptr<ASTNode> value)
        : value(value){}
    const std::shared_ptr<ASTNode> getValue() const { return value; }
private:
    const std::shared_ptr<ASTNode> value;
};

class RetNode : public ASTNode {
public:
    RetNode(const std::shared_ptr<ASTNode> value)
        : value(value){}
    const std::shared_ptr<ASTNode> getValue() const { return value; }
private:
    const std::shared_ptr<ASTNode> value;
};

class StructInstanceNode: public ASTNode {
    public:
    StructInstanceNode( tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> value,std::shared_ptr<ASTNode> base)
        : value(value), base(base) {}
     tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> getValue() { return value; }
    const std::shared_ptr<ASTNode> getBase() const {return base;}
    private:
     tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> value;
    std::shared_ptr<ASTNode> base;
};

class IFNode : public ASTNode {
public:
    IFNode(const std::shared_ptr<ExpressionNode>& value,const tsl::ordered_map<int,std::shared_ptr<ExpressionNode>> elses,const std::shared_ptr<ExpressionNode>& nott)
        : value(value), elses(elses), nott(nott) {}
    tsl::ordered_map<int,std::shared_ptr<ExpressionNode>> getElses() const { return elses; }
    std::shared_ptr<ExpressionNode> getMain() const { return value; }
    std::shared_ptr<ExpressionNode> getNot() const { return nott; }

private:
    std::shared_ptr<ExpressionNode> value;
    std::shared_ptr<ExpressionNode> nott;
    tsl::ordered_map<int,std::shared_ptr<ExpressionNode>> elses;
};

class WhileNode : public ASTNode {
public:
    WhileNode(const std::shared_ptr<ExpressionNode>& expr) : expr(expr) {}
    std::shared_ptr<ExpressionNode> getExpr() { return expr;}

private:
std::shared_ptr<ExpressionNode> expr;

};


class BlockNode : public ASTNode {
public:
    void addStatement(std::shared_ptr<ASTNode> statement) {
        statements.push_back(statement);
    }
    int StatementsLen() {
        return statements.size();
    }

    const std::vector<std::shared_ptr<ASTNode>>& getStatements() const {
        return statements;
    }

private:
    std::vector<std::shared_ptr<ASTNode>> statements;
};

class DoubleRefNode : public ASTNode {
public:
    DoubleRefNode(std::shared_ptr<std::shared_ptr<ASTNode>> ref): ref(ref) {
    }

    std::shared_ptr<std::shared_ptr<ASTNode>> getRef(){
        return ref;
    }



private:
    std::shared_ptr<std::shared_ptr<ASTNode>> ref;
};

class Future{
    public:
      Future(std::string ref,std::shared_ptr<Scope> cs): ref(ref),cs(cs) {}
      std::any get() {
          if (value.has_value()){
              return value;
          } else {
              if (justincase){
                  throw std::runtime_error("Corrupted future.");
              }
              this->value = cs->getVariable(ref)->getValue();
              this->justincase = true;
              return value;


          }
      }
    private:
      bool justincase = false;
      std::string ref;
      std::shared_ptr<Scope> cs;
      std::any value;
};



#endif