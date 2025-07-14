#include "base.hpp"
#include "AST.hpp"
#ifndef MACROTYPES
#define MACROTYPES
int MACRO_ID = 0;

class MacroInstruction{
    public:
    MacroInstruction(): id(MACRO_ID++) {}
    virtual ~MacroInstruction() = default;
    int id;
    void setMSID(int inp){MSID=inp;}
    int MSID;
    int NID;
    void setNID(int inp){NID=inp;}
};


class MacroBlock : public MacroInstruction {
    public:
    MacroBlock(std::vector<std::shared_ptr<MacroInstruction>> items): items(items){}
    std::vector<std::shared_ptr<MacroInstruction>> items;

    private:
};

class MacroSwitch : public MacroInstruction {
    public:
    MacroSwitch(std::shared_ptr<MacroInstruction> value, tsl::ordered_map<std::string,std::shared_ptr<MacroBlock>> cmps):value(value), cmps(cmps){}
    tsl::ordered_map<std::string,std::shared_ptr<MacroBlock>> cmps;
    std::shared_ptr<MacroInstruction> value;

    private:
};

class MacroAccess : public MacroInstruction {
    public:
    MacroAccess(std::string value): value(value){}
    std::string value;

    private:
};

class MacroCall : public MacroInstruction {
    public:
    MacroCall(std::string value, std::vector<std::string> args): value(value), args(args) {}
    std::string value;
    std::vector<std::string> args;

    private:
};

class MacroCallBlock : public MacroInstruction {
    public:
    MacroCallBlock(std::string value, std::vector<std::string> args, std::shared_ptr<MacroInstruction> block): value(value), args(args), block(block) {}
    std::string value;
    std::vector<std::string> args;
    std::shared_ptr<MacroInstruction> block;

    private:
};

class MacroIdentifier : public MacroInstruction {
    public:
    MacroIdentifier(std::string value): value(value){}
    std::string value;

    private:
};

class MacroExpression : public MacroInstruction {
    public:
    MacroExpression(std::shared_ptr<ASTNode> value): value(value){}
    std::shared_ptr<ASTNode> value;

    private:
};

class MacroAssign : public MacroInstruction {
    public:
    MacroAssign(std::string ident, std::shared_ptr<MacroInstruction> value): value(value), ident(ident) {}
    std::string ident;
    std::shared_ptr<MacroInstruction> value;

    private:
};

class MacroError : public MacroInstruction {
    public:
    MacroError(std::string content): content(content) {}
    std::string content;

    private:
};



#endif
