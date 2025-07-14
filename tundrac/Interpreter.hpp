#include "base.hpp"
#include "interpreterc.hpp"
#include "Scope.hpp"
#include "Tokens.hpp"
#include "AST.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#ifndef INTERPRETER
#define INTERPRETER

class Interpreter{
public:
    Interpreter() {
        scopes.push_back(std::make_shared<Scope>(true));  // Create the global (main) scope
    }
    std::shared_ptr<Scope> getCurrentScope() const {
        if (!scopes.empty()) {
            return scopes.back();
        }
        return nullptr;
    }
    void CleanScope(){
        std::shared_ptr<Scope> cur = getCurrentScope();
        if (cur->isVar("self") == true){
            auto v = cur->getVariable("self");
            cur->freeall();
            scopes.pop_back();
            if (v->selfup != ""){
                update(v->selfup,v->getValue());
            }
        } else {
            cur->freeall();
            scopes.pop_back();
        }
        
    }
    void interpret(std::shared_ptr<BlockNode> rootBlock) {
        interpretBlock(rootBlock, /* isMainBlock= */ true);
    }
    


    
    std::vector<std::shared_ptr<Scope>> scopes;

    std::string interpretString(std::shared_ptr<StringLiteralNode> node){
        node = std::dynamic_pointer_cast<StringLiteralNode>(node);
        return node->getValue();
    }

    std::any interpretInteger(std::shared_ptr<IntLiteralNode> node){
        node = std::dynamic_pointer_cast<IntLiteralNode>(node);
        
        return std::any(node->getValue());
    }

    std::any interpretBlock(std::shared_ptr<BlockNode> block, bool isMainBlock,std::string loop="",tsl::ordered_map<std::string,std::any> inserts={}) {
        auto ParentScope = getCurrentScope();
        std::shared_ptr<Scope> localScope;
        if (loop == "loopinit" || loop == "" || (ParentScope->getLoop() == true && ParentScope->getSign() == false)){
            //println("INIT LOOP.");
        
        localScope = std::make_shared<Scope>(isMainBlock,ParentScope);
        if (loop == "loopinit" || ParentScope->getLoop() == true){
            localScope->setLoop();
            localScope->setSign();
        }
        scopes.push_back(localScope);
        } else {
            localScope = ParentScope;
        }
        int a = 0;
        int len = block->StatementsLen();
        //println("entering true");

        if (isMainBlock) {
            //std::cout << "isMainBlock: " << isMainBlock << std::endl;
            std::function<void(std::string)> pln = println;
            localScope->addVariable("__compileprint__", std::make_shared<Variable>(CFunction(pln)));
            localScope->addVariable("true", std::make_shared<Variable>(true));
            //localScope->addVariable("bool", std::make_shared<Variable>(true));
            localScope->addVariable("false", std::make_shared<Variable>(false));
            localScope->addVariable("none", std::make_shared<Variable>(nullification{}));
            localScope->addVariable("null", std::make_shared<Variable>(Pointer(new PtrVal(nullptr))));
            // various Compile-time OS-based variables
            llvm::Triple defaultOSakanative;
            if (!AP.has("target")){
                defaultOSakanative = llvm::Triple(llvm::sys::getDefaultTargetTriple());
            } else if (AP.values["target"][0] == "native"){
                defaultOSakanative = llvm::Triple(llvm::sys::getDefaultTargetTriple());
            } else {
                defaultOSakanative = llvm::Triple(AP.values["target"][0]);
            }
            std::string OSN = defaultOSakanative.getOSTypeName(defaultOSakanative.getOS()).str();
            localScope->addVariable("OperatingSystem", std::make_shared<Variable>(OSN));
            dummyinstructions["OperatingSystem"] = std::make_shared<resource_instruction>("RawString",OSN);
            std::string ARCHN = defaultOSakanative.getArchName().str();
            localScope->addVariable("Architecture", std::make_shared<Variable>(ARCHN));
            dummyinstructions["Architecture"] = std::make_shared<resource_instruction>("RawString",ARCHN);
            std::string SPOSN = defaultOSakanative.getOSName().str();
            localScope->addVariable("CannonicalOS", std::make_shared<Variable>(SPOSN));
            dummyinstructions["CannonicalOS"] = std::make_shared<resource_instruction>("RawString",SPOSN);
            std::string Vendor = defaultOSakanative.getVendorTypeName(defaultOSakanative.getVendor()).str();
            localScope->addVariable("Vendor", std::make_shared<Variable>(Vendor));
            dummyinstructions["Vendor"] = std::make_shared<resource_instruction>("RawString",Vendor);
            if (AP.has("-showOSinfo")){
                println("OperatingSystem: " + OSN);
                println("Architecture: " + ARCHN);
                println("CannonicalOS: " + SPOSN);
                println("Vendor: " + Vendor);
            }
            if (compileflag){
            compile = false;
            }
            if (ruleset.find("typing") == ruleset.end()){
                if (AP.has("typing")){
                    ruleset["typing"] = AP.values["typing"][0];
                } else {
                throw std::runtime_error("Typing must be declared through ruleset, under the main scope or specfied as an argument");
                }
            }
            if (compileflag){
            compile = true;
            }
            dummyinstructions["true"] = std::make_shared<resource_instruction>("Bool",true);
            dummyinstructions["false"] = std::make_shared<resource_instruction>("Bool",false);
            dummyValues["GenericPointer"] = Pointer(new PtrVal(Object("Character","f")));
            for (const auto&  v: dummyValues){
                //println("++++++");
                if (v.first != "nil" && v.first != "GenericPointer"){

                    dummyinstructions[v.first] = std::make_shared<resource_instruction>(v.first,v.second);
                } else if (v.first == "GenericPointer"){
                    dummyinstructions[v.first] = std::make_shared<resource_instruction>("RawString",std::string("GenericPointer"));
                }else {
                    dummyinstructions[v.first] = std::make_shared<resource_instruction>("Void",v.second);
                }
                

                if (v.first != "i32" && v.first != "RawString" && v.first != "Bool" && v.first != "nil" && v.first != "GenericPointer"){
                    localScope->addVariable(v.first, std::make_shared<Variable>(Object(v.second,v.first)));
                } else {
                    localScope->addVariable(v.first, std::make_shared<Variable>(v.second));
                }
            }

        }
        //println("passed true");
        for (auto &i: inserts ){
            //std::cout << "inserting!" << i.first;
            if (i.first == "self"){ // add @|mut

                auto vec = std::any_cast<std::vector<std::any>>(i.second);
                auto varr = std::make_shared<Variable>(vec[0],true);

                varr->setselfup(std::any_cast<std::string>(vec[1]));

                localScope->addVariable(i.first,varr);
            }

            else if (getTypeName(i.second) == "i32" || getTypeName(i.second) == "RawString"){
            localScope->addVariable(i.first,std::make_shared<Variable>(i.second));
            } else if (converters.find(getTypeName(i.second)) != converters.end()) {
                localScope->addVariable(i.first,std::make_shared<Variable>(Object(i.second,getTypeName(i.second))));
            } else {
                if (i.first == "self"){ // add @|mut

                    auto vec = std::any_cast<std::vector<std::any>>(i.second);
                    auto varr = std::make_shared<Variable>(vec[0],true);

                    varr->setselfup(std::any_cast<std::string>(vec[1]));

                    localScope->addVariable(i.first,varr);
                } else {
                    localScope->addVariable(i.first,std::make_shared<Variable>(i.second));
                }
            }
        }
        setscope(getCurrentScope());

        std::any ret;
        std::any con;
        for (const auto& statement : block->getStatements()) {
            std::any pcon = con;
            con = interpretStatement(statement);
            if (inT<int>(statement->id,isbadnode)){
                if (pcon.type() != typeid(void)){
                
                ret = pcon;
                continue;
                }
            }
            if (isMainBlock){
                if (AP.has("compile")){
                
                //add(statement); notary's legacy
                }
            }
            
            if (statement == block->getStatements()[block->getStatements().size()-1] && !inT<int>(statement->id,isbadnode)) {
                ret = con;
            }
            if (con.type() == typeid(Break)){
                if (localScope->getLoop() != true){
                    throw std::runtime_error("Break can only be used in loop/while/for.");
                }
                //std::cout << "RETURNING BREAK" << std::endl;
                ret = con;
                //setscope(getCurrentScope());
                return ret;
            }
            if (con.type() == typeid(Return)){
                //std::cout << "RETURNING RETURN" << std::endl;
                ret = con;
                //setscope(getCurrentScope());
                return ret;
            }
            a++;
        }
        setscope(getCurrentScope());

        if (!isMainBlock && loop != "loopinit" && loop != "loop") {
            //println("Killed loop");
            //println(loop);
            // Clean up local scope variables
            CleanScope();
            
        } else if (isMainBlock == true){
            getCurrentScope()->freeall();
        }
        
        return ret;
    }

    std::any interpretAST(std::shared_ptr<ASTNode> node) {
        //println("NODUS");
        //logat("Interpreting statement ^","Interpreter.iS");
        if (auto blockNode = std::dynamic_pointer_cast<BlockNode>(node)) {
            logat("Interpreting Block","Interpreter.iS");
            return interpretBlock(blockNode, /* isMainBlock= */ false);
        } else if (auto CN = std::dynamic_pointer_cast<CallNode>(node)) {
            logat("Interpreting Call","Interpreter.iS");
            return interpretCall(CN);
        } else if (auto SIN = std::dynamic_pointer_cast<StructInstanceNode>(node)) {
            logat("Interpreting StructInstance","Interpreter.iS");
            return interpretStructInstance(SIN);
        } else if (auto IFN = std::dynamic_pointer_cast<IFNode>(node)) {
            logat("Interpreting If","Interpreter.iS");
            return interpretIf(IFN);
        } else if (auto AN = std::dynamic_pointer_cast<AssertionNode>(node)) {
            logat("Interpreting Assert","Interpreter.iS");
            return interpretAssert(AN);
        } else if (auto BN = std::dynamic_pointer_cast<BreakNode>(node)) {
            logat("Interpreting Break","Interpreter.iS");
            return interpretBreak(BN);
        } else if (auto RN = std::dynamic_pointer_cast<RetNode>(node)) {
            logat("Interpreting Return","Interpreter.iS");
            return interpretRet(RN);
        } else if (auto WN = std::dynamic_pointer_cast<WhileNode>(node)) {
            logat("Interpreting While","Interpreter.iS");
            return interpretWhile(WN);
        } else if (auto lON = std::dynamic_pointer_cast<LoopNode>(node)) {
            logat("Interpreting Loop","Interpreter.iS");
            return interpretLoop(lON);
        } else if (auto ON = std::dynamic_pointer_cast<ONode>(node)) {
            logat("Interpreting Object","Interpreter.iS");
            return interpretONode(ON);
        } else if (auto IdentNode = std::dynamic_pointer_cast<IdentifierNode>(node)) {
            logat("Interpreting Identifier " + IdentNode->getValue(),"Interpreter.iS");
            return interpretIdent(IdentNode);
        } else if (auto MAN = std::dynamic_pointer_cast<MemAccNode>(node)) {
            logat("Interpreting MemberAccess","Interpreter.iS");
            return interpretMember(MAN);
        } else if (auto IN = std::dynamic_pointer_cast<IndexNode>(node)) {
            logat("Interpreting Index","Interpreter.iS");
            return interpretIndex(IN);
        } else if (auto TN = std::dynamic_pointer_cast<TupleNode>(node)) {
            logat("Interpreting Tuple","Interpreter.iS");
            return interpretTuple(TN);
        } else if (auto LN = std::dynamic_pointer_cast<ListNode>(node)) {
            logat("Interpreting List","Interpreter.iS");
            return interpretList(LN);
        } else if (auto TLN = std::dynamic_pointer_cast<TypeSafeListNode>(node)) {
            logat("Interpreting TSL","Interpreter.iS");
            return interpretTSL(TLN);
        } else if (auto TLIN = std::dynamic_pointer_cast<TSLInitNode>(node)) {
            logat("Interpreting TSLinit","Interpreter.iS");
            return interpretTSLI(TLIN);
        } else if (auto WN = std::dynamic_pointer_cast<StructDeclNode>(node)) {
            logat("Interpreting StructDecl","Interpreter.iS");
            return interpretStructDecl(WN);
        } else if (auto assignNode = std::dynamic_pointer_cast<AssignNode>(node)) {
            logat("Interpreting Assign","Interpreter.iS");
            return interpretAssign(assignNode);
        } else if (auto EAN = std::dynamic_pointer_cast<ExprAssignNode>(node)) {
            logat("Interpreting ExprAssign","Interpreter.iS");
            return interpretExprAssign(EAN);
        } else if (auto SassignNode = std::dynamic_pointer_cast<StrongAssignNode>(node)) {
            logat("Interpreting SA","Interpreter.iS");
            //println("BLokl");
            return interpretStrongAssign(SassignNode);
        } else if (auto StringNode = std::dynamic_pointer_cast<StringLiteralNode>(node)) {
            logat("Interpreting String","Interpreter.iS");
            //println("BLokz");
            return interpretString(StringNode);
        } else if (auto OpNode = std::dynamic_pointer_cast<BinOP>(node)) {
            logat("Interpreting OP","Interpreter.iS");
            //println("BLokx");
            return interpretBinOp(OpNode);
        } else if (auto IntNode = std::dynamic_pointer_cast<IntLiteralNode>(node)) {
            logat("Interpreting Int","Interpreter.iS");
            //println("BLoky");
            return interpretInteger(IntNode);
        } else if (auto PN = std::dynamic_pointer_cast<PointerNode>(node)) {
            logat("Interpreting Ptr","Interpreter.iS");
            //println("BLoky");
            return interpretPointer(PN);
        } else if (auto GON = std::dynamic_pointer_cast<GiveOwnershipNode>(node)) {
            logat("Interpreting gPtr","Interpreter.iS");
            //println("BLoky");
            return interpretOwner(GON);
        } else if (auto DN = std::dynamic_pointer_cast<DerefNode>(node)) {
            logat("Interpreting *Ptr","Interpreter.iS");
            //println("BLokXYZ");
            return interpretderef(DN);
        } else if (auto MPN = std::dynamic_pointer_cast<ModifyPtrNode>(node)) {
            logat("Interpreting mPtr","Interpreter.iS");
            //println("BLokXYZ");
            return interpretModPtr(MPN);
        } else if (auto DN = std::dynamic_pointer_cast<DropNode>(node)) {
            logat("Interpreting Drop","Interpreter.iS");
            //println("BLokXYZ");
            return interpretDrop(DN);
        } else if (auto UFN = std::dynamic_pointer_cast<UnMappedFunctionNode>(node)) {
            logat("Interpreting UFN","Interpreter.iS");
            //println("BLokXYZ");
            return interpretUFN(UFN);
        } else if (auto MFN = std::dynamic_pointer_cast<MappedFunctionNode>(node)) {
            logat("Interpreting MFN","Interpreter.iS");
            //println("BLokXYZ");
            return interpretMFN(MFN);
        } else if (auto DN = std::dynamic_pointer_cast<DecoratorNode>(node)) {
            logat("Interpreting Decorator (christmas!)","Interpreter.iS");
            return interpretDecor(DN);
        } else if (auto MN = std::dynamic_pointer_cast<ModNode>(node)) {
            logat("Interpreting Mod","Interpreter.iS");
            return interpretMod(MN);
        } else if (auto EN = std::dynamic_pointer_cast<EnumNode>(node)) {
            logat("Interpreting Enum","Interpreter.iS");
            return interpretEnum(EN);
        } else if (auto NUN = std::dynamic_pointer_cast<NUnararyNode>(node)) {
            logat("Interpreting NUnary","Interpreter.iS");
            return interpretNUnary(NUN);
        } else if (auto ILN = std::dynamic_pointer_cast<IfLetNode>(node)) {
            logat("Interpreting If Let","Interpreter.iS");
            return interpretIfLet(ILN);
        } else if (auto MN = std::dynamic_pointer_cast<MatchNode>(node)) {
            logat("Interpreting Match","Interpreter.iS");
            return interpretMatch(MN);
        } else if (auto DN = std::dynamic_pointer_cast<DeconsNode>(node)) {
            logat("Interpreting Match","Interpreter.iS");
            return interpretDecon(DN);
        } else if (auto SN = std::dynamic_pointer_cast<SearchableNode>(node)) {
            logat("Interpreting Searchable","Interpreter.iS");
            return interpretSearchable(SN);
        } else if (auto FN = std::dynamic_pointer_cast<FutureNode>(node)) {
            logat("Interpreting Future","Interpreter.iS");
            return interpretFuture(FN);
        } else if (auto CN = std::dynamic_pointer_cast<ClassDeclNode>(node)) {
            logat("Interpreting Class","Interpreter.iS");
            return interpretClass(CN);
        } else if (auto EN = std::dynamic_pointer_cast<ExternNode>(node)) {
            logat("Interpreting Extern","Interpreter.iS");
            return interpretExtern(EN);
        } else if (auto FP = std::dynamic_pointer_cast<Fakepass>(node)) {
            logat("Interpreting Fakepass","Interpreter.iS");
            return FP->getValue();
        } else if (auto CN = std::dynamic_pointer_cast<CastNode>(node)) {
            logat("Interpreting Cast","Interpreter.iS");
            return interpretcast(CN);
        } else if (auto PN = std::dynamic_pointer_cast<PubNode>(node)) {
            logat("Interpreting Pub","Interpreter.iS");
            return interpretPub(PN);
        } else if (auto SON = std::dynamic_pointer_cast<SizeOfNode>(node)) {
            logat("Interpreting SizeOf","Interpreter.iS");
            return interpretSizeOf(SON);
        } else if (auto TIDN = std::dynamic_pointer_cast<TypeIDNode>(node)) {
            logat("Interpreting typeid","Interpreter.iS");
            return interpretTypeid(TIDN);
        } else if (auto TIDN = std::dynamic_pointer_cast<TypeIDNode>(node)) {
            logat("Interpreting typeid","Interpreter.iS");
            return interpretTypeid(TIDN);
        } else if (auto ASMN = std::dynamic_pointer_cast<ASMNode>(node)) {
            logat("Interpreting asm","Interpreter.iS");
            return interpretASM(ASMN);
        } else if (auto PTIN = std::dynamic_pointer_cast<PtrtointNode>(node)) {
            logat("Interpreting PTIN","Interpreter.iS");
            return interpretPtr2Int(PTIN);
        } else if (auto ChN = std::dynamic_pointer_cast<ChainNode>(node)) {
            logat("Interpreting ChN","Interpreter.iS");
            return interpretChain(ChN);
        } else if (auto ImN = std::dynamic_pointer_cast<ImportNode>(node)) {
            logat("Interpreting ImN","Interpreter.iS");
            return interpretModule(ImN);
        } else if (auto SSN = std::dynamic_pointer_cast<StackSizeNode>(node)) {
            logat("Interpreting SSN","Interpreter.iS");
            return interpretStackSize(SSN);
        } else if (auto RN = std::dynamic_pointer_cast<RefNode>(node)) {
            logat("Interpreting Ref","Interpreter.iS");
            return interpretRef(RN);
        } else if (auto TN = std::dynamic_pointer_cast<TagNode>(node)) {
            logat("Interpreting Tag","Interpreter.iS");
            return interpretTag(TN);
        } else if (auto CN = std::dynamic_pointer_cast<ConventionNode>(node)) {
            logat("Interpreting Conv","Interpreter.iS");
            return interpretConv(CN);
        } else {
            
            if (typeid(node) == typeid(std::make_shared<ASTNode>())){
                //println("Warning: unknown node");
                return std::any();
            }
            throw std::runtime_error("Unknown node!");
            //println(demangle(typeid(node).name()));
            // disable with prod
            //println("Interpreter error: Unknown statement type\n");
            return std::any();
        }
    }
    std::any interpretStatement(std::shared_ptr<ASTNode> node){
        auto res = interpretAST(node);
        logat("Finished Interpreting Node","Interpreter.iS");
        return res;
    }
    std::any interpretTag(std::shared_ptr<TagNode> TN){
        auto mfn = std::any_cast<MappedFunction>(interpretStatement(TN->getValue()));
        auto tyid = std::any_cast<uint64_t>(std::any_cast<Object>(interpretStatement(std::make_shared<TypeIDNode>(instanceize(std::make_shared<Fakepass>(mfn.getargs()[mfn.gettick()[0]]),mfn.getargs()[mfn.gettick()[0]])))).GetStore());
        tsl::ordered_map<int,std::any> tts;
        tts[tyid] = mfn;
        return Tag(tts);
        
    }
    std::any interpretRef(std::shared_ptr<RefNode> Ref){
        return Pointer(new PtrVal(interpretStatement(Ref->getValue())));
    }
    std::any interpretStackSize(std::shared_ptr<StackSizeNode> SSN){
        return Object(static_cast<uint64_t>(0),"u64");
    }
    std::any interpretModule(std::shared_ptr<ImportNode> ImN){
        auto from = ImN->getFrom();
        auto m = Module(ImN->getName());
        for (auto& fm : m.imports){
            auto modname = split(fm.first,':');
            auto f = modname[1];
            if (modname[0] == "mfn"){
            tsl::ordered_map<std::string,std::any> argstack;
            tsl::ordered_map<int,std::string> tick;
            tsl::ordered_map<std::string,std::string> internals;
            std::vector<std::shared_ptr<ASTNode>> nodes;
            std::shared_ptr<ASTNode> retnode;
            auto args = split(fm.second,',');
            int ticker = 0;
            for (auto& arg : args){
                auto types = split(arg,':');
                std::string inte =  types[1];
                inte = replace(inte,m.hash + "COLON" + m.hash,":");
                inte = replace(inte,m.hash + "COMMA" + m.hash,",");
                auto lex = Lexer("{\n" + inte + "\n}");
                auto parsed = Parser(lex).parse();
                auto interpreter = Interpreter();
                if (types[0] != "-!retty"){
                    nodes.push_back(parsed->getStatements()[0]); // assume 0th instruction is it and with gettypeName it will be...
                } else {
                    retnode = parsed->getStatements()[0];
                }
                argstack[types[0]] = interpreter.interpretBlock(parsed,true);
                tick[ticker] = types[0];
                ticker += 1;
            }
            auto retty = argstack["-!retty"];
            argstack.erase("-!retty");
            nodes.push_back(retnode);
            auto blk = std::make_shared<BlockNode>();
            blk->addStatement(std::make_shared<RetNode>(retnode));

            m.addMethod(f,MappedFunction(blk,argstack,tick,retty,internals,false));
            if (from.size() != 0){
            if (from[0] == "*"){
                from.push_back(f);

            }
            }
            m.addNode(f,nodes);
            }

        }
        std::vector<std::string> temp;
        for (auto& f : from){
            if (f != "*"){
                temp.push_back(f);
            }
        }
        from = temp;
        //std::cout << "from " << debugvectorToString(from) << std::endl;
        ImN->setFrom(from);
        forwardscplex[ImN->id] = m;
        if (!from.empty()){
            std::vector<std::shared_ptr<ASTNode>> chain;

            for (auto& f : from){
                if (m.methods.find(f) != m.methods.end()){
                    chain.push_back(std::make_shared<AssignNode>(f,std::make_shared<Fakepass>(ModuleMethod(m.hash,f,m.methods[f])),false));
                } else {
                    throw std::runtime_error("Unknown import");
                }
            }
            interpretStatement(std::make_shared<ChainNode>(chain)); // this does our assigning for us
            return NoAssign{};
        }
        
        return m;
    }
    std::any interpretChain(std::shared_ptr<ChainNode> ChN){
        for (auto& c : ChN->getChain()){
            logat("Interpreting chain element","interpreter.iC");
            interpretStatement(c);
            logat("Interpreted chain element","interpreter.iC");
        }
        return std::any();
    }
    std::any interpretPtr2Int(std::shared_ptr<PtrtointNode> PTIN){
        return Object(static_cast<uintptr_t>(PTIN->getValue()->id),"i64").GetStore(); // garbage
    }

    std::any interpretConv(std::shared_ptr<ConventionNode> CN){
        std::vector<std::shared_ptr<ASTNode>> newpubs;
        for (auto& stat : CN->getPub()){
            auto craftednode = std::make_shared<PubNode>(stat);
            newpubs.push_back(craftednode);
            interpretStatement(craftednode);
        }
        CN->setPub(newpubs);
        return std::any();
        
    }
    std::any interpretASM(std::shared_ptr<ASMNode> assembl){

        //displayError("This operation is inherently unsafe and can only remain safe in managed conditions",lpos[assembl->][0],lpos[assembl->id][1],lpos[assembl->id][2]+2,gerr({"note","mark the function using the inline assembly with the unsafe decree to suppress this warning or mark the inline assembly with the safe decree if you believe it is safe."}) ,true);


        
        
        for (auto& a : assembl->assignables){
            interpretStatement(std::make_shared<AssignNode>(a.first,a.second));
        }
        return std::any();
    }
    std::any interpretSizeOf(std::shared_ptr<SizeOfNode> szn){
        std::any res = interpretStatement(szn->getValue());
        return Object(static_cast<uint64_t>(sizeof res),"u64");
    }

    std::any interpretTypeid(std::shared_ptr<TypeIDNode> tyid){
        auto inte = interpretStatement(tyid->getValue());
        uint64_t res =  static_cast<uint64_t>(std::type_index(inte.type()).hash_code());
        if (inte.type() == typeid(BorrowedPointer)){
            forwards[tyid->id] = nety(std::any_cast<BorrowedPointer>(inte).getValue());
            res = CPf(res, static_cast<uint64_t>(std::type_index(std::any_cast<BorrowedPointer>(inte).getValue().type()).hash_code()));

        }
        else if (inte.type() == typeid(Pointer)){
            forwards[tyid->id] = nety(std::any_cast<Pointer>(inte).getptr()->getValue());
            res = CPf(res , static_cast<uint64_t>(std::type_index(std::any_cast<Pointer>(inte).getptr()->getValue().type()).hash_code()));
        } else if (inte.type() == typeid(std::string)){
            forwards[tyid->id] = nety(inte);
            res = CPf(res , static_cast<uint64_t>(std::type_index(inte.type()).hash_code()));
        } else if (inte.type() == typeid(MappedFunction)){
            forwards[tyid->id] = nety(inte);
            res = CPf(res , static_cast<uint64_t>(std::type_index(inte.type()).hash_code()));
        } else {
            res = CPf(res,0);
        }
        
        return Object(res,"u64");
    }
    std::any interpretPub(std::shared_ptr<PubNode> PN){
        if (auto aas = std::dynamic_pointer_cast<AssignNode>(PN->getValue())){
        if (auto mfn = std::dynamic_pointer_cast<MappedFunctionNode>(aas->getValue())){
            if (AP.has("-bundle") || result == "package"){
                std::hash<std::string> hashery;
                exports["t4hash"] = std::to_string(hashery(AP.values["compile"][0]));
                std::string argStr = "";
                int i = 0;
                for (auto& arg: mfn->getValue()){
                    auto ty = getTypeName(interpretStatement(arg.second));
                    ty = replace(ty,":",exports["t4hash"] + "COLON" + exports["t4hash"]);
                    ty = replace(ty,",",exports["t4hash"] + "COMMA" + exports["t4hash"]);
                    argStr += ((i == 0 ? "" : ",") + arg.first + ":" + ty);
                    i++;
                }
                exports["mfn:" +  aas->getVarName()] = argStr;
            }
            return interpretStatement(PN->getValue());
        } else {
            displayError("Keyword pub/public only supports functions",lpos[PN->id][0],lpos[PN->id][1],lpos[PN->id][2],gerr({"[O]","pub fn foo(...){...}","[X]","pub foo =\"bar\""}));
        }
        }
    }
    std::any interpretcast(std::shared_ptr<CastNode> CN){
        auto value = interpretStatement(CN->getValue());

        auto dest = interpretStatement(CN->getDest());
        logat("Casting to: " + getTypeName(dest) , "Interpreter.ic");
        forwards[CN->id] = nety(dest);
        tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> inserts = {};
        if (value.type() == typeid(Pointer)){
            auto ptr = std::any_cast<Pointer>(value);
            PtrVal* pv = ptr.getptr();
            
            
            tsl::ordered_map<int,std::shared_ptr<ASTNode>> args;
            Pointer p(nullptr);
            if (converters.find(replace(getTypeName(dest),"&","")) == converters.end()){
                 
                if (getTypeName(dest)[0] == '&'){
                   
                    p = Pointer(new PtrVal(dest)); // this just tricks Tundra and is not a good idea
                } else {
                    return dest;
                }
            } else {
                //return dest;
                p = Pointer(new PtrVal(converters[replace(getTypeName(dest),"&","")](pv->getValue()))); // make a try {} catch ... so if invalid conversions are made....

            }
            
            p.pointsto(ptr.getpt());
            return p;
        } else if (value.type() == typeid(BorrowedPointer)){
            auto bptr = std::any_cast<BorrowedPointer>(value);
            tsl::ordered_map<int,std::shared_ptr<ASTNode>> args;
            args[0] = CN->getValue();
            bptr.editValue(interpretStatement(std::make_shared<CallNode>(args,std::make_shared<IdentifierNode>(replace(getTypeName(dest),"&","")),inserts)));
            return bptr;
        } else if (value.type() == typeid(CEXTFunction)){
            auto cxfn = std::any_cast<CEXTFunction>(value);
            
            
            return CEXTFunction(std::any_cast<MappedFunction>(dest).getretty(),std::any_cast<MappedFunction>(dest));
        } else if (dummyValues.find(getTypeName(value)) != dummyValues.end()){
            auto ty = getTypeName(dest);
            if (value.type() == typeid(Object)){
                if (ty == "i32"){
                    return convertToInt32(std::any_cast<Object>(value).value);
                } else if (ty == "RawString"){
                    return convertToString(std::any_cast<Object>(value).value);
                }
                auto obj =Object(std::any_cast<Object>(value).value,ty);
                obj.GetStore();

                return obj;
            } else {
                if (ty == "i32"){
                    return convertToInt32(value);
                } else if (ty == "RawString"){
                    return convertToString(value);
                }
                auto obj = Object(value,ty);
                obj.GetStore();
                return obj;
            }
        } else {
            
            displayError("Given type " + demangle(value.type().name()) +  " is unable to be casted as it is not a pointer, only pointer types may be casted to one another using the 'as' keyword",lpos[CN->id][0],lpos[CN->id][1],lpos[CN->id][2],gerr({"note","Pointers may be casted through: <ptr> as <dest>","[O]","my_ptr = &10; myi64ptr = my_ptr as &i64"}));
        }
        return std::any();
    }

    std::any interpretExtern(std::shared_ptr<ExternNode> EN){
        auto fns = EN->getValue();
        auto fnnames = EN->getfnnames();
        for (int i = 0; i != fns.size();i++){
            if (auto mfn = std::dynamic_pointer_cast<MappedFunctionNode>(fns[i])){
                auto retty = mfn->getValue()["-!retty"];
                auto res = interpretStatement(retty);

                if (dummyValues.find(getTypeName(res)) != dummyValues.end()){
                    if (getCurrentScope()->isVar(fnnames[i]) == false){
                        getCurrentScope()->addVariable(fnnames[i],std::make_shared<Variable>(CEXTFunction(res,std::any_cast<MappedFunction>(interpretStatement(mfn)))));
                    } else {
                        throw std::runtime_error("Extern declared functions may not overwrite another variable name even if mutable");
                    }
                

                } else {
                     if (getCurrentScope()->isVar(fnnames[i]) == false){
                        getCurrentScope()->addVariable(fnnames[i],std::make_shared<Variable>(CEXTFunction(res,std::any_cast<MappedFunction>(interpretStatement(mfn)))));
                     } else {
                        throw std::runtime_error("Extern declared functions may not overwrite another variable name even if mutable");
                    }
                }
            
            } else {
                throw std::runtime_error("Extern closures may only have functions defined within them.");
            }
        }
        return std::any();

    }
    std::any interpretClass(std::shared_ptr<ClassDeclNode> CN){
        auto fns = CN->getValue();
        auto cons = CN->getCons();
        tsl::ordered_map<std::string, std::any> fnsa;
        tsl::ordered_map<std::string, std::any> consa;
        for (const auto& k : fns){
            fnsa[k.first] = interpretStatement(k.second);
        }
        for (const auto& k2 : cons){
            consa[k2.first] = interpretStatement(k2.second);
        }
        return Class(fnsa,consa);

    }
    std::any interpretFuture(std::shared_ptr<FutureNode> FN){
        auto id = FN->getValue();
        return std::make_shared<Future>(id,getCurrentScope());
    }
    std::any interpretSearchable(std::shared_ptr<SearchableNode> SN){
        tsl::ordered_map<int,std::any> retmap;
        auto map = SN->getChecks();
        for (auto& k : map){
            retmap[k.first] = interpretStatement(k.second);
        }
        return Searchable(SN->getValue(),retmap);
    }
    std::any interpretDecon(std::shared_ptr<DeconsNode> DN){
        auto res = interpretStatement(DN->getValue());
        auto inodes = DN->getIdents();
        tsl::ordered_map<std::string, std::any>inserts;
        if (res.type() == typeid(EnumKey)){

            auto EK = std::any_cast<EnumKey>(res);
            if (EK.getValue().size() < inodes.size()){
                throw std::runtime_error("Size Mismatch!");
            }
            auto vec = EK.getValue();
            for(int i = 0; i != inodes.size();i++){
                inserts[inodes[i]] = vec[i];
            }

        } else if (res.type() == typeid(TypeSafeList)){
            auto tsl = std::any_cast<TypeSafeList>(res);
            if (tsl.getlist().size() < inodes.size()){
                throw std::runtime_error("Size Mismatch!");
            }
            auto vec = tsl.getlist();
            for(int i = 0; i != inodes.size();i++){
                inserts[inodes[i]] = vec[i];
            }
        } else {
            throw std::runtime_error("Un supported deconstruction");
        }
        return inserts;
    }
    std::any interpretMatch(std::shared_ptr<MatchNode> MN){

        auto checkto = interpretStatement(MN->getValue());

        for (const auto& k: MN->getns()){
            if (auto DN = std::dynamic_pointer_cast<DeconsNode>(k.first)){


                if (std::any_cast<bool>(interpretStatement(std::make_shared<BinOP>(MN->getValue(),DN->getValue(),"=="))) == true){
                    DN->setValue(MN->getValue());
                    return interpretBlock(std::dynamic_pointer_cast<BlockNode>(k.second),false,"",std::any_cast<tsl::ordered_map<std::string, std::any>>(interpretStatement(DN)));
                }
            } else {
            if (std::any_cast<bool>(interpretStatement(std::make_shared<BinOP>(MN->getValue(),k.first,"=="))) == true){
                return interpretStatement(k.second);
            }
            }
        }
        if (MN->getElse()){
            return interpretStatement(MN->getElse());
        }
        throw std::runtime_error("No conditions of match were met, please use _ to set a default");
    }
    std::any interpretIfLet(std::shared_ptr<IfLetNode> ILN){
        auto vn = ILN->getValue();
        if (auto an = std::dynamic_pointer_cast<AssignNode>(vn)){
        auto res = interpretStatement(an->getValue());

        if (res.type() == typeid(nullification)){
            return interpretStatement(ILN->getElse());
        } else if (res.type() == typeid(EnumKey) && std::any_cast<EnumKey>(res).getName() == "none"){
            return interpretStatement(ILN->getElse());
        } else if (res.type() == typeid(EnumKey) && std::any_cast<EnumKey>(res).getValue().size() == 1){
            tsl::ordered_map<std::string,std::any> ins;
            ins[an->getVarName()] = std::any_cast<EnumKey>(res).getValue()[0];
            return interpretBlock(std::dynamic_pointer_cast<BlockNode>(ILN->getBody()),false,"",ins);
        } else {
            tsl::ordered_map<std::string,std::any> ins;
            ins[an->getVarName()] = res;
            return interpretBlock(std::dynamic_pointer_cast<BlockNode>(ILN->getBody()),false,"",ins);            
        }
        } else {
            throw std::runtime_error("if let <assign> ...");
        }
    }
    std::any interpretNUnary(std::shared_ptr<NUnararyNode> NUN){
        std::any res = interpretStatement(NUN->getValue());
        if (res.type() == typeid(EnumKey) && std::any_cast<EnumKey>(res).getName() != "none" && std::any_cast<EnumKey>(res).getValue().size() == 1){
            return std::any_cast<EnumKey>(res).getValue()[0]; // fix this for better handling
        } else if (res.type() == typeid(EnumKey) && std::any_cast<EnumKey>(res).getName() == "none"){
            return interpretStatement(NUN->getElse());
        } else if (res.type() == typeid(nullification)){
            return interpretStatement(NUN->getElse());
        } else {
            return res;
        }
        return std::any();
    }
    std::any interpretEnum(std::shared_ptr<EnumNode> EN){
        auto v = EN->getValue();
        std::unordered_map<std::string,EnumKey> keys;
        for (const auto& k : v){
            std::vector<std::any> l = {};
            for (const auto& kv : k.second){
                auto expr = interpretStatement(kv);
                l.push_back(expr);
            }
            keys.emplace(k.first, EnumKey("", l,Enum({}),k.first));
        }
        std::unordered_map<std::string,EnumKey> based;
        auto ret = Enum(keys);
        for (auto& e : keys){
            std::string type;
            std::string ef = e.first;
            if (std::any_cast<EnumKey>(e.second).getValue().size() == 0){
                type = "fixed";
            } else {
                type = "variant";
            }
            based.emplace(ef, EnumKey(type, e.second.getValue(),ret,ef));

        }
        return Enum(based);
    }

    std::any interpretDrop(std::shared_ptr<DropNode> DN){
        auto res = DN->getValue();
        if (auto IN = std::dynamic_pointer_cast<IdentifierNode>(res)){
            getCurrentScope()->removeVariable(IN->getValue());
        }
        return std::any();
    }
    std::any interpretMod(std::shared_ptr<ModNode> MN){
        tsl::ordered_map<std::string,std::any> cplex;
        cplex["type"] = std::string("");
        forwardscplex[MN->id] = cplex;
        if (auto IN = std::dynamic_pointer_cast<IdentifierNode>(MN->getValue())){
            auto base = interpretStatement(IN);
            if (base.type() == typeid(StructInstance)){
                auto SI = std::any_cast<StructInstance>(base);
                auto checkers = SI.getValue();
                auto cons = SI.getCons();
                auto fns = SI.getFns();
                for (const auto& state: MN->getStates()){
                    if (auto AN = std::dynamic_pointer_cast<AssignNode>(state) ){
                        std::any i;
                        if (auto mfnn = std::dynamic_pointer_cast<MappedFunctionNode>(AN->getValue())){
                            auto args = mfnn->getValue();
                            if (args.find("self") != args.end()){
                                mfnn->setself(MN->getValue());
                            }
                            i = interpretStatement(mfnn);
                        } else {
                            i = interpretStatement(AN->getValue());
                        }


                        if (i.type() == typeid(MappedFunction)){
                            if (checkers.find(AN->getVarName()) != checkers.end()){
                                throw std::runtime_error("Overwriting existing fields within a struct is not allowed");
                            }
                            if (fns.find(AN->getVarName()) != fns.end()){
                                throw std::runtime_error("Overwriting existing methods within a struct is not allowed");
                            }
                            if (cons.find(AN->getVarName()) != cons.end()){
                                throw std::runtime_error("Overwriting constants within a struct is not allowed");
                            }


                            fns[AN->getVarName()] = i;

                            getCurrentScope()->addVariable(IN->getValue(),std::make_shared<Variable>(StructInstance(SI.getValue(),cons,fns)));

                        } else {
                            if (checkers.find(AN->getVarName()) != checkers.end()){
                                throw std::runtime_error("Overwriting existing fields within a struct is not allowed");
                            }
                            if (fns.find(AN->getVarName()) != fns.end()){
                                throw std::runtime_error("Overwriting existing methods within a struct is not allowed");
                            }
                            if (cons.find(AN->getVarName()) != cons.end()){
                                throw std::runtime_error("Overwriting constants within a struct is not allowed");
                            }
                            cons[AN->getVarName()] = i;
                            getCurrentScope()->addVariable(IN->getValue(),std::make_shared<Variable>(StructInstance(SI.getValue(),cons,fns)));
                        }
                        
                    } else {
                        throw std::runtime_error("Mod(ifications) for structs only support constants through assigning or methods through functions");
                    }
                }
                getCurrentScope()->addVariable(IN->getValue(),std::make_shared<Variable>(StructInstance(SI.getValue(),cons,fns)));
            } else if (base.type() == typeid(StructDecl)){
                auto SD = std::any_cast<StructDecl>(base);
                auto checkers = SD.getValue();
                auto cons = SD.getConsts();
                auto fns = SD.getFns();
                for (const auto& state: MN->getStates()){
                    if (auto AN = std::dynamic_pointer_cast<AssignNode>(state) ){
                        std::any i;
                        if (auto mfnn = std::dynamic_pointer_cast<MappedFunctionNode>(AN->getValue())){
                            auto args = mfnn->getValue();
                            if (args.find("self") != args.end()){
                                mfnn->setself(MN->getValue());
                            }
                            i = interpretStatement(mfnn);
                        } else {
                            i = interpretStatement(AN->getValue());
                        }

                        if (i.type() == typeid(MappedFunction)){
                            if (checkers.find(AN->getVarName()) != checkers.end()){
                                throw std::runtime_error("Overwriting existing fields within a struct is not allowed");
                            }
                            if (fns.find(AN->getVarName()) != fns.end()){
                                throw std::runtime_error("Overwriting existing methods within a struct is not allowed");
                            }
                            if (cons.find(AN->getVarName()) != cons.end()){
                                throw std::runtime_error("Overwriting constants within a struct is not allowed");
                            }
                            auto mfn = std::any_cast<MappedFunction>(i);
                            auto args = mfn.getargs();
                            if (args.find("self") != args.end()){
                                args["self"] = SD;
                            }

                            fns[AN->getVarName()] = mfn;

                            getCurrentScope()->addVariable(IN->getValue(),std::make_shared<Variable>(StructDecl(SD.getValue(),cons,fns)));
                        } else {
                            if (checkers.find(AN->getVarName()) != checkers.end()){
                                throw std::runtime_error("Overwriting existing fields within a struct is not allowed");
                            }
                            if (fns.find(AN->getVarName()) != fns.end()){
                                throw std::runtime_error("Overwriting existing methods within a struct is not allowed");
                            }
                            if (cons.find(AN->getVarName()) != cons.end()){
                                throw std::runtime_error("Overwriting constants within a struct is not allowed");
                            }
                            
                            cons[AN->getVarName()] = i;
                            getCurrentScope()->addVariable(IN->getValue(),std::make_shared<Variable>(StructDecl(SD.getValue(),cons,fns)));
                        }
                    } else {
                        throw std::runtime_error("Mod(ifications) for structs only support constants through assigning or methods through functions");
                    }
                }
                getCurrentScope()->addVariable(IN->getValue(),std::make_shared<Variable>(StructDecl(SD.getValue(),cons,fns)));

            } else if (base.type() == typeid(Tag)){
                
                cplex["type"] = std::string("tag");
                forwardscplex[MN->id] = cplex;
                Tag t = std::any_cast<Tag>(base);
                auto map = t.tagtypes;
                for (auto& state: MN->getStates()){
                    if (auto AN = std::dynamic_pointer_cast<AssignNode>(state)){
                    auto res = interpretStatement(AN->getValue());
                    if (res.type() == typeid(MappedFunction)){
                        auto mfn = std::any_cast<MappedFunction>(res);
                        auto tyid = std::any_cast<uint64_t>(std::any_cast<Object>(interpretStatement(std::make_shared<TypeIDNode>(instanceize(std::make_shared<Fakepass>(mfn.getargs()[mfn.gettick()[0]]),mfn.getargs()[mfn.gettick()[0]])))).GetStore());
                        map[tyid] = mfn;
                    } else {
                        throw std::runtime_error("Unextendable type for tags"); // make this better (syntax)
                    }
                    } else {
                        throw std::runtime_error("Unextendable type for tags");
                    }
                }
                getCurrentScope()->addVariable(IN->getValue(),std::make_shared<Variable>(Tag(map)));
            }   else {
                throw std::runtime_error("Type is currently unsupported for mod(ification)");
            }
        } else {
            throw std::runtime_error("Mod can only be used with an identifier");
        }
        return std::any();
    }

    std::any interpretExprAssign(std::shared_ptr<ExprAssignNode> EAN){
        if (auto IN = std::dynamic_pointer_cast<IndexNode>(EAN->getVarName())){
            tsl::ordered_map<std::string,std::any> cplex;
            cplex["ty"] = std::string("arr");

            auto base = interpretStatement(IN->getValue());
            if (base.type() == typeid(Pointer)){
                if ((!isdeclunsafe && !inT<int>(IN->id,hasbeenwarned)) && !isdeclsafe){
                if (auto idntowarn = std::dynamic_pointer_cast<IdentifierNode>(IN->getValue())){
                    displayError("Unsafe pointer index access of '" + idntowarn->getValue() + "'",lpos[IN->getValue()->id][0],lpos[IN->getValue()->id][1],lpos[IN->getValue()->id][2]+2,gerr({"note","mark the function calling this with the unsafe decree to suppress this warning or mark the function with the safe decree if you believe it is safe."}) ,true);
                } else {
                    displayError("Unsafe pointer index access ",lpos[IN->getValue()->id][0],lpos[IN->getValue()->id][1],lpos[IN->getValue()->id][2]+2,gerr({"note","mark the function calling this with the unsafe decree to suppress this warning or mark the function with the safe decree if you believe it is safe."}) ,true);
                }
                }
                auto ptr = std::any_cast<Pointer>(base);
                int ind = std::any_cast<int>(interpretStatement(IN->getIndex()));
                auto val = interpretStatement(EAN->getValue());
                //ptr.getptr()->setValue(val);
                std::vector<std::shared_ptr<type_instruction>> forward {nety(ptr.getptr()->getValue()),nety(ptr.getptr()->getValue())};
                cplex["ty"] = std::string("ptr");
                cplex["arrity"] = forward;
                forwardscplex[IN->id] = cplex;
                if (auto IDN = std::dynamic_pointer_cast<IdentifierNode>(IN->getValue())){
                    
                    getCurrentScope()->addVariable(IDN->getValue(),std::make_shared<Variable>(ptr));
                    
                }  else if (auto MEM = std::dynamic_pointer_cast<MemAccNode>(IN->getValue())){
                    
                    interpretStatement(std::make_shared<MemAccNode>(MEM->getValue(),MEM->getNxt(),true,std::make_shared<Fakepass>(ptr)));
                    
                } else {
                    throw std::runtime_error("List assigning can only be used with identifiers, not lists");
                }
            }
            else if (base.type() == typeid(TypeSafeList)){

                auto tsl = std::any_cast<TypeSafeList>(base);
                int ind = std::any_cast<int>(interpretStatement(IN->getIndex()));
                auto val = interpretStatement(EAN->getValue());
                tsl.set(ind,val);
                std::vector<std::shared_ptr<type_instruction>> forward {nety(tsl),nety(tsl.get(ind))};
                forwardscplex[IN->id] = forward;
                if (auto IDN = std::dynamic_pointer_cast<IdentifierNode>(IN->getValue())){
                    
                    getCurrentScope()->addVariable(IDN->getValue(),std::make_shared<Variable>(tsl));
                    
                }  else if (auto MEM = std::dynamic_pointer_cast<MemAccNode>(IN->getValue())){
                    
                    interpretStatement(std::make_shared<MemAccNode>(MEM->getValue(),MEM->getNxt(),true,std::make_shared<Fakepass>(tsl)));
                    
                } else {
                    throw std::runtime_error("List assigning can only be used with identifiers, not lists");
                }
            } else if (base.type() == typeid(List)){
                auto list = std::any_cast<List>(base);
                list.add(interpretStatement(EAN->getValue()));
                if (auto IDN = std::dynamic_pointer_cast<IdentifierNode>(IN->getValue())){
                    getCurrentScope()->addVariable(IDN->getValue(),std::make_shared<Variable>(list));
                } else {
                    throw std::runtime_error("List assigning can only be used with identifiers, not lists");
                }
            }
        } else {
            throw std::runtime_error("This type is not assignable");
        }
        return std::any();
    }

    std::any interpretDecor(std::shared_ptr<DecoratorNode> DN){
        if (auto CN = std::dynamic_pointer_cast<CallNode>(DN->getClr())){
            if (auto AN = std::dynamic_pointer_cast<AssignNode>(DN->getFn())){
                if (auto FN = std::dynamic_pointer_cast<MappedFunctionNode>(AN->getValue())){
                    auto v = CN->getExpr();
                    tsl::ordered_map<int, std::shared_ptr<ASTNode>> out;
                    tsl::ordered_map<int, std::shared_ptr<ASTNode>> output; // needed because the ordered map tracks order of insertion

                    if (v.size() == 0){
                        out[0] = FN;
                    } else {
                        for (const auto& k : v){
                            out[k.first+1] = k.second;
                        }
                        out[0] = FN;
                    }
                    for (int i = 0; i != out.size();i++){
                        output[i] = out[i];
                    }
                    CN->setExpr(output);
                    auto res = interpretStatement(CN);
                    bool switcher = true;
                    if (res.type() == typeid(MappedFunction)){
                        if (std::any_cast<MappedFunction>(res).issignature){
                            switcher = false;
                            issig.push_back(CN->id); // unofficial use
                        }
                    }
                    if (switcher){
                    
                    auto var = Variable(res,AN->getMut());
                    std::shared_ptr<Scope> c = getCurrentScope();
                    c->addVariable(AN->getVarName(),std::make_shared<Variable>(var));
                    }


                    
                } else {
                    throw std::runtime_error("Decorators must be above only functions");
                }
            }
        } else {
            throw std::runtime_error("Decorators can only be called");
        }
        return std::any();
    }

    std::any interpretTSLI(std::shared_ptr<TSLInitNode> TSLI){
        bool szu;
        int size;
        if (auto IN = std::dynamic_pointer_cast<IdentifierNode>(TSLI->getBody())){
            if (IN->getValue() == "?"){
                szu = true;
            }
        }
        std::any sz;
        if (!szu){
            sz = interpretStatement(TSLI->getBody());
        } else {
            sz = 1;
        }
        
        if (getTypeName(sz) != "i32"){
            throw std::runtime_error("Need i32 for tsl init size");
        }
        size = std::any_cast<int32_t>(sz);
        forwardscplex[TSLI->id] = size;

        if (size < 0){
            throw std::runtime_error("Invalid size for typesafelist");
        }
        
        auto fill = interpretStatement(instanceize(TSLI->getExpr(),interpretStatement(TSLI->getExpr())));
        auto tsl = TypeSafeList({},fill,size);
        
        while (size != 0){
            tsl.add(fill);
            if (AP.has("compile")){
                break;
            }
            size -= 1;
        }
        if (szu){
            tsl.setSZU(true);
        }
        return tsl;
    }
    std::any interpretAssert(std::shared_ptr<AssertionNode> AN){
        auto boola = interpretStatement(AN->getExpr());
        if (boola.type() != typeid(bool)){
            throw std::runtime_error("Assert requires a boolean result");
        }
        bool b = std::any_cast<bool>(boola);
        if (!b){
            throw std::runtime_error("Assertion error");
        }
        return nullification{};
    }
    std::shared_ptr<ASTNode> instanceize(std::shared_ptr<ASTNode> node, std::any res){
        logat("instanceizing for: " + getTypeName(res),"Interpreter.ie");
        if (res.type() == typeid(StructDecl)){
            auto SD = std::any_cast<StructDecl>(res);
            return std::make_shared<Fakepass>(StructInstance(SD.getValue(),SD.getConsts(),SD.getFns()));
        }
        if (res.type() == typeid(Pointer)){
            auto ptr = std::any_cast<Pointer>(res);
            return std::make_shared<Fakepass>(Pointer(new PtrVal(interpretStatement(instanceize(std::make_shared<Fakepass>(ptr.getptr()->getValue()),ptr.getptr()->getValue())))));
        }
        if (res.type() == typeid(BorrowedPointer)){
            auto ptr = std::any_cast<BorrowedPointer>(res);
            return std::make_shared<Fakepass>(BorrowedPointer(ptr.getptr(),ptr.isMut()));
        }
        if (res.type() == typeid(void)){
            if (auto idn = std::dynamic_pointer_cast<IdentifierNode>(node)){
            if (idn->getValue() == "self"){
                throw std::runtime_error("GOT SELF AS VOID");
            }
            throw std::runtime_error("BAD INSTANCE");
            }
        }
        return node;
    }
    std::any interpretMFN(std::shared_ptr<MappedFunctionNode> MFN,bool first=false){
    if (ruleset["typing"] == "dynamic"){
        throw std::runtime_error("Decleration of mapped functions can only be used with a static typing ruleset");
    }      
    auto args = MFN->getValue();
    tsl::ordered_map<std::string,std::any> argus;


    std::any retty = interpretStatement(args["-!retty"]);
    forwardscplex2[MFN->id] = retty.type() == typeid(nullification);
    args.erase("-!retty");
    for (const auto& k: args){


        argus[k.first] = interpretStatement(k.second);

    }
    auto mappedfn = MappedFunction(MFN->getBody(),argus,MFN->gettick(),retty,MFN->getinternals(),MFN->isselfptr);
    tsl::ordered_map<int,std::shared_ptr<ASTNode>> argstack;
    int i = 0;
    for (auto& t : mappedfn.gettick()){
        if (mappedfn.gettick()[t.first] != "self"){
        auto inst = instanceize(MFN->getValue()[t.second],interpretStatement(MFN->getValue()[t.second]));
        if (inst){
        argstack[i] = inst;
        }
        i++;
        } else {
            auto selfinst = instanceize(MFN->getValue()[t.second],interpretStatement(MFN->getValue()[t.second]));
            //if (mappedfn.isselfptr){
                //selfinst = std::make_shared<Fakepass>(Pointer(new PtrVal(selfinst)));
            //}
            mappedfn.setself(std::dynamic_pointer_cast<Fakepass>(selfinst)->getValue());
        }
        

        

    }
    tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> inserts{};
    auto BN = std::dynamic_pointer_cast<BlockNode>(MFN->getBody());
    if (BN && BN->StatementsLen() != 0 && !AP.has("-nofnchecks") && !inT<int>(MFN->id,issig)){


    
    interpretStatement(std::make_shared<CallNode>(argstack,std::make_shared<Fakepass>(mappedfn),inserts)); // improve this!!


    }
    forwardscplex[MFN->id] = retty;
    if (inT<int>(MFN->id,issig)){
        mappedfn.issignature = true;
    }
    
    return mappedfn;
    }
    std::any interpretWrapper(std::shared_ptr<Fakepass> FN){
        return FN->getValue();
    }

    std::any interpretUFN(std::shared_ptr<UnMappedFunctionNode> UFN){

        if (ruleset["typing"] == "static"){
            throw std::runtime_error("Decleration of unmapped functions can only be used with a dynamic typing ruleset");
        }
        auto args = UFN->getValue();
        return UnMappedFunction(UFN->getBody(),args);
    }
    std::any interpretModPtr(std::shared_ptr<ModifyPtrNode> MPN){
        std::any ptr = interpretStatement(MPN->getLHS());
        if (ptr.type() != typeid(Pointer) && ptr.type()!= typeid(BorrowedPointer)){
            throw std::runtime_error("Modification of pointers is allowed only for Pointers and Mutable BorrowedPointers");
        }
        if (ptr.type() == typeid(Pointer)){
            std::any_cast<Pointer>(ptr).editValue(interpretStatement(MPN->getExpr()));
        } else if (ptr.type() == typeid(BorrowedPointer)){
            std::any_cast<BorrowedPointer>(ptr).editValue(interpretStatement(MPN->getExpr()));
        }
        return nullification{};
    }
    std::any interpretOwner(std::shared_ptr<GiveOwnershipNode> GON){
        //println("IO");
        std::any res = interpretStatement(GON->getExpr());
        //println("EXP");
        if (res.type() != typeid(Pointer)){
            throw std::runtime_error("You can only give ownership of a pointer");
        }
        Pointer p = std::any_cast<Pointer>(res);
        if (p.getpt() != ""){
            getCurrentScope()->removeVariable(p.getpt());
            getCurrentScope()->addVariable(p.getpt(),std::make_shared<Variable>(BorrowedPointer(p.getptr(),false),false));

        }
        //println(p.getpt());
        p.pointsto("");
        
        return p;
        
    }
    std::any interpretPointer(std::shared_ptr<PointerNode> PN){
        if (PN->getBorrow() == true){
            std::any res = interpretStatement(PN->getExpr());
            if (res.type() != typeid(Pointer)){
                throw std::runtime_error("Borrowing of pointers can only be done for pointer objects");
            }

            
            
            return BorrowedPointer(std::any_cast<Pointer>(res).getptr(),PN->getMut());

            
        }
        
        auto ptr = Pointer(new PtrVal(interpretStatement(PN->getExpr())));
        ptr.setMut(PN->getMut());
        return ptr;

    }
    std::string gettype(std::string s){
        if (converters.find(s) == converters.end() && s != "_struct" && s != "Struct" && s != "tup" && s != "list" && s != "tsl"){
            return getTypeName(interpretStatement(std::make_shared<IdentifierNode>(s)));
        } else {
            return s;
        }
    }
    std::string keyfind(tsl::ordered_map<std::string,int> m, int key){
        for (auto& pair:m){
            if (pair.second == key){
                return pair.first;
            }
        }
    }

    std::any interpretCall(std::shared_ptr<CallNode> CN){
        //std::cout << "Callr";
        //std::cout.flush();
        //std::cout << demangle(typeid(CN->getBody()).name());

        
        tsl::ordered_map<std::string,std::any> cplex;
        if (auto ccid = std::dynamic_pointer_cast<IdentifierNode>(CN->getBody())){
            auto cid = ccid->getValue();
            if (converters.find(cid) != converters.end()){
            auto argstack = CN->getExpr();
            if (argstack.size() > 1){
                throw std::runtime_error("Converters can only take 1 argument.");
            }
            // << "converters" << std::endl;

            
            return Object(interpretStatement(argstack[0]),cid).GetStore();
            
            
        }
        }
        logat("Getting caller","Interpreter.iC");
        
        auto caller = interpretStatement(CN->getBody());

        logat("Caller: " + getTypeName(caller) + "(" + std::to_string(CN->id) + ")","Interpreter.iC");
        
        //std::cout << "goose";

        
        if (caller.type() == typeid(EnumKey)){
        try {

        auto clr = std::any_cast<EnumKey>(caller);
        auto argus = clr.getValue();
        auto argstack = CN->getExpr();
        if (clr.getType() == "fixed" ){
            if (argstack.size() != 0){
                throw std::runtime_error("Invalid no. of arguments, expected 0.");
            }
            return clr;
        } else {
        if (argstack.size() != argus.size()){
            throw std::runtime_error("Invalid no. of arguments, expected " + std::to_string(argus.size()) + ".");
        }
        std::vector<std::any> result;

        
        for (int i=0; i != argus.size(); i++){
            auto expr = interpretStatement(argstack[i]);
            if (compareAnyTy(argus[i],expr) == false){
                throw std::runtime_error("Typemismatched.");
            }
            result.push_back(expr);
        }
        auto EK = EnumKey(clr.getType(),result,clr.getBase(),clr.getName());
        return EK;
        } 
        } catch (std::bad_any_cast& e){
            throw std::runtime_error("Bad call: " + convertToString(e.what()));
        }
        } else if (caller.type() == typeid(Enum)){   
            auto clr = std::any_cast<Enum>(caller);
            auto argstack = CN->getExpr();
            auto keys = clr.getValue();

            if (keys.size() > 2){
                throw std::runtime_error("Implicit Enum Calls can only have 2 keys, one with arguments, one without");
            }
            EnumKey vk = EnumKey("",{});
            std::vector<std::any> result;
            int a = 0;
            bool fail = false;
            for (auto k : keys){
                if (k.second.getType()  == "variant" && !fail){
                    
                    auto vec = k.second.getValue();
                    for (int i=0; i != vec.size(); i++){
                        auto expr = interpretStatement(argstack[i]);
                        if (compareAnyTy(vec[i],expr) == true){
                            result.push_back(expr);
                        } else {
                            fail = true;
                            
                        }
                        if (!fail){
                            return EnumKey(k.second.getType(),result,k.second.getBase(),k.second.getName());
                        }
                        
                    }
                }
                if (fail && a != 0){
                    if (k.second.getType()  == "variant"){
                        throw std::runtime_error("If one key is a variant, the other one must be fixed");
                    }
                }
                if (k.second.getType() == "variant"){
                    a += 1;
                }
                
            }
            if (fail){
                for (auto k : keys){
                    if (k.second.getType() == "fixed"){
                        return k.second;
                    }
                }
            }
            return std::any();

            
        
        } else if (caller.type() == typeid(UnMappedFunction)){
        try {
        //std::cout << "Calling Unmapped!";
        auto clr = std::any_cast<UnMappedFunction>(caller);
        auto argstack = CN->getExpr();
        auto argus = clr.getargs();
        tsl::ordered_map<std::string,std::any> args;
        if (argstack.size() != argus.size()){

            throw std::runtime_error("Expected " + convertToString(argus.size()) + " arguments\nGot: " + convertToString(argstack.size()) + " arguments");
        }
        for (auto& v : argstack){
            
            args[argus[v.first]] = interpretStatement(v.second);
        }
        auto b = clr.getBody();
        
        std::any res = interpretBlock(std::dynamic_pointer_cast<BlockNode>(b),false,"",args);
        CleanScope();
        if (res.type() == typeid(Return)){
            res = std::any_cast<Return>(res).getValue();
        }
        return res;
        
        } catch (std::bad_any_cast& e){
            throw std::runtime_error("Bad call: " + convertToString(e.what()));
        }} else if (caller.type() == typeid(MappedFunction)){
        try {
        //std::cout << "Calling Mapped!";

        auto clr = std::any_cast<MappedFunction>(caller);
        cplex["type"] = std::string("mappedfunction");
        cplex["fty"] = nety(caller);
        cplex["mfn"] = clr;
        forwardscplex[CN->id] = cplex;
        auto argstack = CN->getExpr();
        auto argus = clr.getargs();
        auto tick = clr.gettick();
        auto intes = clr.getinternals();
        auto insts = CN->getinserts();
        int times = 0;

        tsl::ordered_map<std::string,std::any> args;
        tsl::ordered_map<int, std::shared_ptr<ASTNode>> saveforcompileastack = argstack;
        //auto regresstick = tick;
        auto saveinserts = CN->getinserts();

        
        for (auto& ins : saveinserts){
            int offset = argus.find("self") == argus.end() ? 0 : 1;
            bool insertcheck = false;
            auto check = *reversefind<int,std::string>(tick,ins.first,&insertcheck);
            argstack[*reversefind<int,std::string>(tick,ins.first)-offset] = ins.second;
            times += 1;
            
        }
        /*if (times > intes.size()){
            throw std::runtime_error("Unknown named argument");
        } what was my logic with this...*/

        if (argus.find("self") == argus.end()){
        if (argstack.size() != argus.size()){
            throw std::runtime_error("Expected " + convertToString(argus.size()) + " arguments\nGot: " + convertToString(argstack.size()) + " arguments");
        }
        } else {
            if (argstack.size() != (argus.size()-1)){
            throw std::runtime_error("Expected " + convertToString(argus.size()-1) + " arguments\nGot: " + convertToString(argstack.size()) + " arguments");
            }
            std::vector<std::any> vec = {argus["self"],clr.getselfupdate()};
            for (int i = 0; i != saveforcompileastack.size();i++){
                saveforcompileastack[i+1] = saveforcompileastack[i];
                saveforcompileastack.erase(i);

            }
            saveforcompileastack[0] = clr.getselforigin();



            //CN->setExpr(saveforcompileastack);
            args["self"] = vec;
            argus.erase("self");
            /*for (auto& t: tick){
                tick[t.first-1] = t.second;
                tick.erase(t.first);
            }*/
            //tick.erase(0);
            // alignment adjustments
            for (auto& a: argstack){
                argstack[a.first+1] = a.second;
                argstack.erase(a.first);
            }
        
            
        }

        const auto sz = argstack.size() + (args.find("self") != args.end() ? 1 : 0);


        for (int iid = 0; iid!= sz/**/;iid++){
            if (args.find("self") != args.end() && iid == 0){
            } else {





            args[tick[iid]] = interpretStatement(argstack[iid]);

            if (compareAnyTy(argus[tick[iid]],args[tick[iid]]) == false){
                throw std::runtime_error("Mismatched type for argument no. " + convertToString(iid+1) +  ": " + std::to_string(saveinserts.size()) + ": " + tick[iid]);
            } // from here
            }
            
            
            
        }
        auto b = clr.getBody();
        tsl::ordered_map<std::string,std::any> iargs;
        
        for (const auto& k : args){
            if (intes.find(k.first) != intes.end() && iargs.find(k.first) == iargs.end()){
                iargs[intes[k.first]] = k.second;
            } else {
                iargs[k.first] = k.second;
            }
            
        }



        auto S = getCurrentScope();
        bool prev=isdeclunsafe;
        bool safeprev=isdeclsafe;
        if (!std::dynamic_pointer_cast<Fakepass>(CN->getBody())){ // dont want it to trigger when ran from MFN
        if (issafedecl.find(b->id) != issafedecl.end()){
            safeprev = isdeclsafe;
            isdeclsafe = true;
        }

        if (isunsafedecl.find(b->id) != isunsafedecl.end()){

            prev = isdeclunsafe;

            if ((!prev && !isdeclsafe && (!inT<int>(CN->id,hasbeenwarned)))){
            if (auto idntowarn = std::dynamic_pointer_cast<IdentifierNode>(CN->getBody())){
                displayError("Unsafe reference to '" + idntowarn->getValue() + "'",lpos[CN->getBody()->id][0],lpos[CN->getBody()->id][1],lpos[CN->getBody()->id][2]+2,gerr({"note","mark the function calling this with the unsafe decree to suppress this warning or mark the function with the safe decree if you believe it is safe."}) ,true);
            } else {
                displayError("Unsafe reference",lpos[CN->getBody()->id][0],lpos[CN->getBody()->id][1],lpos[CN->getBody()->id][2]+2,gerr({"note","mark the function calling this with the unsafe decree to suppress this warning or mark the function with the safe decree if you believe it is safe."}) ,true);
            }
            }

            isdeclunsafe = true;
        }
        }


        if (!std::dynamic_pointer_cast<BlockNode>(b)){
            return std::any();
        }
        

        std::any res = interpretBlock(std::dynamic_pointer_cast<BlockNode>(b),false,"",iargs); // to here
        if (isunsafedecl.find(b->id) != isunsafedecl.end()){
            isdeclunsafe = prev;
        }
        if (issafedecl.find(b->id) != issafedecl.end()){
            isdeclsafe = safeprev;
        }
        if (getCurrentScope() != S){
            CleanScope();
        }
        if (res.type() == typeid(Return)){
            res = std::any_cast<Return>(res).getValue();
        }
        if (clr.getretty().type() == typeid(nullification)){
            res = nullification{};
        } else {

        if (compareAnyTy(clr.getretty(),res) == false){


            displayError("Type mismatch on return: Got " + demangle(res.type().name()),lpos[CN->id][0],lpos[CN->id][1],lpos[CN->id][2],gerr({"note","Please ensure your function returns the specified type"}));
            
        }
        }
        if (saveforcompileastack.size() != 0 && args.find("self") != args.end()){
            CN->setExpr(saveforcompileastack);
        }
        return res;
        
        } catch (std::bad_any_cast& e){
            if (auto idntolog = std::dynamic_pointer_cast<IdentifierNode>(CN->getBody())){
                logat("Failed on call for: " + idntolog->getValue(),"Interpreter.iC");
            }
            throw std::runtime_error("Bad call (MFN) : " + convertToString(e.what()));
        }} else if (caller.type() == typeid(Class)){
            auto clr = std::any_cast<Class>(caller);
            auto fns = clr.getValue();
            auto argstack = CN->getExpr();
            if (fns.find("new") == fns.end()){
                if (argstack.size() != 0){
                    throw std::runtime_error("Class expected 0 arguments");
                }
                return ClassInstance(fns,clr.getConsts());
            } else {
                auto ci = ClassInstance(fns,clr.getConsts());
                auto vptr = std::make_shared<Variable>(ci,false);
                
                

            }


        } else if (caller.type() == typeid(CEXTFunction)){
            auto clr = std::any_cast<CEXTFunction>(caller);
            
            if ((!isdeclunsafe && !inT<int>(CN->id,hasbeenwarned)) && !isdeclsafe){
            hasbeenwarned.push_back(CN->id);
            if (auto idntowarn = std::dynamic_pointer_cast<IdentifierNode>(CN->getBody())){
                //println(std::to_string(CN->id) + "::"+ debugvectorToString<int>(lpos[CN->getBody()->id])); <- fix bpos not being diff
                displayError("Unsafe reference to '" + idntowarn->getValue() + "'",lpos[CN->getBody()->id][0],lpos[CN->getBody()->id][1],lpos[CN->getBody()->id][2]+2,gerr({"note","mark the function calling this with the unsafe decree to suppress this warning or mark the function with the safe decree if you believe it is safe."}) ,true);
            } else {
                displayError("Unsafe reference",lpos[CN->getBody()->id][0],lpos[CN->getBody()->id][1],lpos[CN->getBody()->id][2]+2,gerr({"note","mark the function calling this with the unsafe decree to suppress this warning or mark the function with the safe decree if you believe it is safe."}) ,true);
            }
            }
            for (auto& arg: CN->getExpr()){
                interpretStatement(arg.second);
            }
            for (auto& arg: CN->getinserts()){
                interpretStatement(arg.second);
            }
            cplex["type"] = std::string("mappedfunction");
            cplex["fty"] = nety(clr.MFNlink); // make this better later
            cplex["mfn"] = clr.MFNlink;
            forwardscplex[CN->id] = cplex;

            

            
            return interpretStatement(instanceize(std::make_shared<Fakepass>(clr.getDV()),clr.getDV()));


    
        } else if (caller.type() == typeid(ModuleMethod)){ 
        try {
            
            
            auto clr = std::any_cast<ModuleMethod>(caller);
            for (auto& arg: CN->getExpr()){
                interpretStatement(arg.second);
            }
            for (auto& arg: CN->getinserts()){
                interpretStatement(arg.second);
            }
            cplex["type"] = std::string("mappedfunction");
            cplex["fty"] = nety(clr.mfn); 
            cplex["mfn"] = std::any_cast<MappedFunction>(clr.mfn);
            forwardscplex[CN->id] = cplex;
            
            
            return std::any_cast<MappedFunction>(clr.mfn).getretty(); // might just do an in-interpreter call
        } catch (std::bad_any_cast& e){
            throw std::runtime_error("Bad call MM: " + convertToString(e.what()) + "::" + demangle(caller.type().name()));
        }}  else if (caller.type() == typeid(TagFunction)){ 
        try {
            
            

            auto tf = std::any_cast<TagFunction>(caller);
            auto mfn = tf.MFNlink;
            cplex["type"] = std::string("tag");
            cplex["fty"] = nety(mfn); 
            cplex["mfn"] = std::any_cast<MappedFunction>(mfn);

            
            auto base = tf.AN;
            auto argstack = CN->getExpr();
            //auto specs = CN->get
            tsl::ordered_map<int, std::shared_ptr<ASTNode>> newas;
            for (auto& arg : argstack){
                newas[arg.first+1] = arg.second;
            }

            newas[0] = base;
            cplex["astack"] = newas; // gen unique-ids on interpret!!
            forwardscplex[CN->id] = cplex;
            

            
            return interpretStatement(std::make_shared<CallNode>(newas,std::make_shared<Fakepass>(mfn),CN->getinserts()));


        } catch (std::bad_any_cast& e){
            throw std::runtime_error("Bad call MM: " + convertToString(e.what()) + "::" + demangle(caller.type().name()));
        }} else if (caller.type() == typeid(CFunction)){ // legacy
        try {

            cplex["type"] = std::string("cfn");
            forwardscplex[CN->id] = cplex;
            
            auto clr = std::any_cast<CFunction>(caller);
            auto stat = CN->getExpr().find(0)->second;
            std::any res = interpretStatement(stat);
            if (res.type() == typeid(Object)){
                res = std::any_cast<Object>(res).GetStore();
            }
            
            clr.call(convertToString(res));
            return nullification{};
        } catch (std::bad_any_cast& e){
            throw std::runtime_error("Bad call C: " + convertToString(e.what()) + "::" + demangle(caller.type().name()));
        }} else {
            throw std::runtime_error("Unknown caller: " + demangle(caller.type().name()));
        }
        return std::any();
    
    }
    std::any interpretStructInstance(std::shared_ptr<StructInstanceNode> SIN){

        std::any bret = interpretStatement( SIN->getBase());
        if (bret.type() != typeid(StructDecl)){
            throw std::runtime_error("Expected struct, got: " + demangle(bret.type().name()));
        }
        auto base = std::any_cast<StructDecl>(bret);
        auto inpmap = SIN->getValue();
        auto outmap = base.getValue();
        tsl::ordered_map<std::string,std::any> fields;
        for (const auto& value: inpmap){
            auto out = interpretStatement(value.second);
            //println(convertToString(out));
            //println("_____");
            if (outmap.find(value.first) == outmap.end()){
                throw std::runtime_error("Unrecognised field: " + value.first);
            } else if (compareAnyTy(outmap[value.first],out) == false){
                throw std::runtime_error("Unexpected type: " + getTypeName(out) + "(" + demangle(out.type().name()) + ")" + "\nExpected: " + getTypeName(outmap[value.first]) +  "(" + demangle(outmap[value.first].type().name()) + ")");

            }
            fields[value.first] = out;
        }
        auto ret = StructInstance(fields,base.getConsts(),base.getFns());
        if (compileflag){
            ret.setSN(std::dynamic_pointer_cast<IdentifierNode>(SIN->getBase())->getValue());
        }
        forwardscplex[SIN->id] = 0;
        vidids.push_back(SIN->id);
        
        return ret;
        

    }

    std::any interpretStructDecl(std::shared_ptr<StructDeclNode> WN){
        auto x =  WN->getValue();
        auto y =  WN->getCons();
        tsl::ordered_map<std::string,std::any> keys;
        tsl::ordered_map<std::string,std::any> cons;
        for (const auto& key : x){
            keys[key.first] = interpretStatement(key.second);
        }
        for (const auto& kk : y){
            cons[kk.first] = interpretStatement(kk.second);
        }

        
            
        
        return StructDecl(keys,cons);
    }
    void update(std::string a,std::any res){
        if (getCurrentScope()->isVar(a)){
            
            if (compareAnyTy(getCurrentScope()->getVariable(a)->getValue(),res) == false){
                throw std::runtime_error("Mismatched member access edit type");
            }
        }
        if (a == "self"){
            auto varr = std::make_shared<Variable>(res);
            varr->setselfup(getCurrentScope()->getVariable("self")->selfup);
            getCurrentScope()->addVariable(a,varr);
        } else {
            getCurrentScope()->addVariable(a,std::make_shared<Variable>(res));
        }
    }
    std::any AccessMem(std::shared_ptr<MemAccNode> baseMAN,std::any base, std::string member,bool assign,std::shared_ptr<ASTNode> expr,std::string aas){
        logat("Accessing Member","Interpreter.AM");
        tsl::ordered_map<std::string,std::any> cplex;
        cplex["vid"] = 0;
        if (auto id = std::dynamic_pointer_cast<IdentifierNode>(baseMAN->getValue())){
            cplex["vid"] = getCurrentScope()->getVariable(id->getValue())->valueid;
        }
        if (base.type() == typeid(Tuple)){
            if (!isdigit(member[0])){
                cplex["type"] = std::string("tuple");
                cplex["inferred"] = std::make_shared<MemAccNode>(std::make_shared<MemAccNode>(baseMAN->getValue(),std::make_shared<StringLiteralNode>("0")),std::make_shared<StringLiteralNode>(member));
                forwardscplex[baseMAN->id] = cplex;
                return interpretStatement(std::any_cast<std::shared_ptr<MemAccNode>>(cplex["inferred"]));
            }
            cplex["type"] = std::string("tuple");
            cplex["index"] = std::stoi(member);
            cplex["tuple"] = nety(base);
            cplex["memtype"] = nety(std::any_cast<Tuple>(base).get(safeStringToInt64(member)));
            
            forwardscplex[baseMAN->id] = cplex;
            
            return std::any_cast<Tuple>(base).get(safeStringToInt64(member));
        } else if (base.type() == typeid(EnumKey)){
            return std::any_cast<EnumKey>(base).getValue()[(safeStringToInt64(member))];
        } else if (base.type() == typeid(StructInstance)){
            cplex["type"] = std::string("structI");
            auto s = std::any_cast<StructInstance>(base);
            cplex["structI"] = nety(s);
            auto m = s.getValue();
            
            if (m.find(member) == m.end()){
                auto c = std::any_cast<StructInstance>(base).getCons();
                if (c.find(member) == c.end()){
                auto f = std::any_cast<StructInstance>(base).getFns();
                if (f.find(member) == c.end()){
                    throw std::runtime_error("Field " + member + " not found in struct");
                } else {
                    auto fn = std::any_cast<MappedFunction>(f[member]);
                    auto args = fn.getargs();
                    if (assign){
                        f[member] = interpretStatement(expr);
                        update(aas, StructInstance(s.getValue(),s.getCons(),f));
                    }
                    if (args.find("self") == args.end()){
                        cplex["siaccesstype"] = std::string("method");
                        forwardscplex[baseMAN->id] = cplex;
                        return fn;
                    } else {
                        cplex["siaccesstype"] = std::string("method");
                        forwardscplex[baseMAN->id] = cplex;
                        args["self"] = std::any_cast<StructInstance>(base);
                        auto mfn = MappedFunction(fn.getBody(),args,fn.gettick(),fn.getretty(),fn.getinternals(),fn.isselfptr);
                        mfn.setselforigin(baseMAN->getValue());
                        mfn.setselfupdate(aas);

                        return mfn;
                    }
                }
                
                } else {
                    if (assign){
                        c[member] = interpretStatement(expr);
                        update(aas, StructInstance(s.getValue(),c,s.getFns()));
                    }
                    cplex["memtype"] = nety(c[member]);
                    cplex["siaccesstype"] = std::string("cons");
                    forwardscplex[baseMAN->id] = cplex;
                    

                    return c[member];
                }
            }
            if (assign){
                m[member] = interpretStatement(expr);
                update(aas, StructInstance(m,s.getCons(),s.getFns()));
            }
            cplex["siaccesstype"] = std::string("member");
            cplex["memtype"] = nety(m[member]);
            int id = 0;
            for (auto& mem : m){
                if (mem.first == member){
                    break;
                }
                id++;
            }
            cplex["id"] = id;

            forwardscplex[baseMAN->id] = cplex;
            return m[member];
        } else if (base.type() == typeid(ClassInstance)){
            auto ci = std::any_cast<ClassInstance>(base);
            auto m = ci.getValue();
            auto c = ci.getConsts();
            if (m.find(member) != m.end()){
                if (assign){
                m[member] = interpretStatement(expr);
                update(aas, ClassInstance(m,ci.getConsts()));
                }
                return m[member];
            }
            if (c.find(member) != c.end()){
                if (assign){
                c[member] = interpretStatement(expr);
                update(aas, ClassInstance(ci.getValue(),c));
                }
                return c[member];
            }
        } else if (base.type() == typeid(Enum)){
            auto m = std::any_cast<Enum>(base).getValue();
            if (m.find(member) == m.end()){
                throw std::runtime_error("Field " + member + " not found in enum");
            }

            return m.at(member);         
        } else if (base.type() == typeid(Module)){
            auto mod = std::any_cast<Module>(base);
            cplex["type"] = std::string("module");
            cplex["mod"] = mod;
            forwardscplex[baseMAN->id] = cplex;
            if (mod.methods.find(member) != mod.methods.end()){
                return mod.methods.find(member)->second;
            } else {
                throw std::runtime_error("Entry " + member + " not found in module");
            }
        }
        throw std::runtime_error("No Member Access found");
    }
    std::any interpretMember(std::shared_ptr<MemAccNode> MAN){
        //println("----");
        
        auto aas = MAN->getValue()->getPointsTO();
        std::any base = interpretStatement(MAN->getValue());
        std::string member = convertToString(interpretStatement(MAN->getNxt()));
        std::any aam;
        try {
        
            aam = AccessMem(MAN,base,member,MAN->getAssign(),MAN->getAssignv(),aas);
        } catch (std::runtime_error& e){
            
            if (getCurrentScope()->isVar(member) && getCurrentScope()->getVariable(member)->value.type() == typeid(Tag)){

                auto tyid = std::any_cast<uint64_t>(std::any_cast<Object>(interpretStatement(std::make_shared<TypeIDNode>(std::make_shared<Fakepass>(base)))).GetStore());
                Tag t = std::any_cast<Tag>(interpretStatement(std::make_shared<IdentifierNode>(member)));
                if (t.tagtypes.find(tyid) != t.tagtypes.end()){
                    tsl::ordered_map<std::string,std::any> cplex;
                    cplex["vid"] = 0;
                    if (auto id = std::dynamic_pointer_cast<IdentifierNode>(MAN->getValue())){
                        cplex["vid"] = getCurrentScope()->getVariable(id->getValue())->valueid;
                    }
                    cplex["type"] = std::string("tag");
                    
                    auto mfn = std::any_cast<MappedFunction>(t.tagtypes[tyid]);
                    cplex["callertype"] = nety(mfn.getargs()[mfn.gettick()[0]]); // first arg
                    //mfn.setself(base); // we'll use this functionality
                    aam = TagFunction(mfn,MAN->getValue());
                    forwardscplex[MAN->id] = cplex;
                } else {
                    throw std::runtime_error("Used Tag does not support type");
                }

            } else {
                throw std::runtime_error(e.what());
            }

        }
        //forwardscplex[MAN->getValue()->id] = forwardscplex[MAN->id];
        //std::cout << "transfered " << MAN->id << " to " << MAN->getValue()->id << ":"<<MAN->getNxt()->id<<std::endl;
        return aam;
    }
    std::any interpretRet(std::shared_ptr<RetNode> RN){
        std::any expr = interpretStatement(RN->getValue());
        //std::cout << "V: " << convertToString(expr) << std::endl;
        if (expr.type() == typeid(std::make_shared<ASTNode>())){
            // default return
            return Return(nullification{});
        }
        //std::cout << "EXPRESSO";
        return Return(expr);
    }
    std::any interpretBreak(std::shared_ptr<BreakNode> BN){
        //std::cout << "BROKEN!" << std::endl;
        std::any expr = interpretStatement(BN->getValue());
        //std::cout << "V: " << convertToString(expr) << std::endl;
        if (expr.type() == typeid(std::make_shared<ASTNode>())){
            // default break
            return Break(nullification{});
        }
        //std::cout << "EXPRESSO";
        return Break(expr);
    }
    std::any interpretWhile(std::shared_ptr<WhileNode> WN){
        auto expr = interpretStatement(WN->getExpr()->getExpr());
        if (expr.type() != typeid(bool)){
            throw std::runtime_error("While syntax is while <bool/condition> {//block of code to run}");
        }
        bool whilebool = std::any_cast<bool>(expr);
        std::any ret;
        auto body = std::dynamic_pointer_cast<BlockNode>(WN->getExpr()->getBody());
        if (whilebool == true){
            ret = interpretBlock(body,false,"loopinit");
            try {
            whilebool = std::any_cast<bool>(interpretStatement(WN->getExpr()->getExpr()));
            } catch (std::bad_any_cast){
                throw std::runtime_error("While syntax is while <bool/condition> {//block of code to run}");
            }

        }
        std::shared_ptr<Scope> current = getCurrentScope();
        while (whilebool == true){
            if (checkbreak(ret) == true){
                ret = std::any_cast<Break>(ret).getValue();
                break;
            }
            ret = interpretBlock(body,false,"loop");
            try {
            whilebool = std::any_cast<bool>(interpretStatement(WN->getExpr()->getExpr()));
            } catch (std::bad_any_cast){
                throw std::runtime_error("While syntax is while <bool/condition> {//block of code to run}\nThis includes editing of variables in the while loop.");
            }
            if (AP.has("compile")){
                break;
            }
        }
        while (getCurrentScope() != current) {
            //println("X");
            CleanScope();
            
            //println("Y");
        }
        //println("A");
        CleanScope();
         // need to patch4nested

        return ret;
    }
    bool checkbreak(std::any type){
        //std::cout << "CHECKER";
        if (type.type() == typeid(Break)){
            return true;
        }
        return false;

    }
    std::any interpretLoop(std::shared_ptr<LoopNode> lON){
        //println("lloping");
        
        auto times = interpretStatement(lON->getTimes());
        //println(typeid(times).name());

        if (times.type() != typeid(int32_t) && times.type() != typeid(std::make_shared<ASTNode>())){
            throw std::runtime_error("Raw integers must be used for loop times");
        }
        int time;
        if (times.type() == typeid(std::make_shared<ASTNode>())){
            time = -1;
        } else {
        time = std::any_cast<int32_t>(times);
        }
        
        if (time != -1 && time < 0){
            throw std::runtime_error("Negative amount of times to loop is disallowed");
        }
        std::any ret;

        auto body = std::dynamic_pointer_cast<BlockNode>(lON->getValue());
        //println("PRE.");
        ret = interpretBlock(body,false,"loopinit");
        if (time != -1){
        time -= 1;
        }
        std::shared_ptr<Scope> current = getCurrentScope();
        //println(convertToString(time));
        if (time != -1){
            while (time != 0){
                if (checkbreak(ret) == true){
                    ret = std::any_cast<Break>(ret).getValue();
                    break;
                }
                
                ret = interpretBlock(body,false,/*loop=*/"loop");
                time -= 1;
            }
        } else {
            while (true){
                if (checkbreak(ret) == true){
                    ret = std::any_cast<Break>(ret).getValue();
                    break;
                }
                ret = interpretBlock(body,false,"loop");
            }
        
        }
        // it's up to loop to clean the scope
        //println(".p");
        
        while (getCurrentScope() != current) {
            //println("X");
            CleanScope();
            
            //println("Y");
        }
        //println("A");
        CleanScope();
         // need to patch4nested
        //println("Done loop.");
        return ret;

    }

    std::any interpretList(std::shared_ptr<ListNode> LN){
        auto lmap = LN->getValue();
        int a = 0;
        tsl::ordered_map<int,std::any> tlmap;
        for (int i = 0; i < lmap.size(); ++i) {
            tlmap[a] = interpretStatement(lmap[a]);
            a += 1;

        }
        
        return List(tlmap);
    }
    std::any interpretTuple(std::shared_ptr<TupleNode> TN){
        //println("TN");
        std::string type = TN->getType();
        auto tumap = TN->getValue();
        int a = 0;
        int32_t sz;
        auto size = interpretStatement(TN->getSize());
        //println("ETN");


        sz = -1;
        
        tsl::ordered_map<int,std::any> tlmap;
        for (int i = 0; i < tumap.size(); ++i) {
            if (type != "none" && type != ""){
                std::any ta = interpretStatement(tumap[a]);
                
                if (getTypeName(ta) != type){
                    throw std::runtime_error("Expected type: " + type + "\nGot: " + getTypeName(ta));
                }
                tlmap[a] = ta;
            } else {
            tlmap[a] = interpretStatement(tumap[a]);
            }
            a += 1;

        }
        if (tlmap.size() > sz && sz != -1){
            throw std::runtime_error("Declared tuple size: " + convertToString(sz) + "\nTuple size: " + convertToString(tlmap.size()));
        }
        if (sz == -1){
            sz = tlmap.size();
        }
        // add dummy values for later
        return Tuple(tlmap,type);
    }
    std::any interpretIndex(std::shared_ptr<IndexNode> IN){
        // TypeSafeList + Tuple!!! DO TMRW.
        std::any l = interpretStatement(IN->getValue());
        tsl::ordered_map<std::string,std::any> cplex;
        cplex["ty"] = std::string("arr");

        if (l.type() == typeid(Pointer)){
            if ((!isdeclunsafe && !inT<int>(IN->id,hasbeenwarned)) && !isdeclsafe){
            if (auto idntowarn = std::dynamic_pointer_cast<IdentifierNode>(IN->getValue())){
                displayError("Unsafe pointer index access of '" + idntowarn->getValue() + "'",lpos[IN->getValue()->id][0],lpos[IN->getValue()->id][1],lpos[IN->getValue()->id][2]+2,gerr({"note","mark the function calling this with the unsafe decree to suppress this warning or mark the function with the safe decree if you believe it is safe."}) ,true);
            } else {
                displayError("Unsafe pointer index access ",lpos[IN->getValue()->id][0],lpos[IN->getValue()->id][1],lpos[IN->getValue()->id][2]+2,gerr({"note","mark the function calling this with the unsafe decree to suppress this warning or mark the function with the safe decree if you believe it is safe."}) ,true);
            }
            }
            cplex["ty"] = std::string("ptr");
            hasbeenwarned.push_back(IN->id);
            std::vector<std::shared_ptr<type_instruction>> forward {nety(std::any_cast<Pointer>(l).getptr()->getValue()),nety(std::any_cast<Pointer>(l).getptr()->getValue())};
            cplex["arrity"] = forward;
            forwardscplex[IN->id] = cplex;
            return std::any_cast<Pointer>(l); // would return addr to index
        }
        if (l.type() != typeid(List) && l.type() != typeid(TypeSafeList)){
            throw std::runtime_error("For now, indexes can only be used for lists. Map support soon.\nGot: " + demangle(l.type().name()));
        } if (l.type() == typeid(List)){
        List lobj = std::any_cast<List>(l);
        std::any i = interpretStatement(IN->getIndex());
        if (i.type() == typeid(Object)){
            Object obj = std::any_cast<Object>(i);
            if (obj.getType() == "i32"){
                i = obj.GetStore();
            }
        }
        if (i.type() != typeid(int32_t)){
            std::string errstr = "Used type: " + demangle(i.type().name());
            //println(errstr);
            throw std::runtime_error("Please use default integers or i32 for list indexing.");
        }
        int32_t index = std::any_cast<int32_t>(i);
        
        return lobj.get(index);
        } else if (l.type() == typeid(TypeSafeList)){
        
        TypeSafeList lobj = std::any_cast<TypeSafeList>(l);
        std::any i = interpretStatement(IN->getIndex());
        if (i.type() == typeid(Object)){
            Object obj = std::any_cast<Object>(i);
            if (obj.getType() == "i32"){
                i = obj.GetStore();
            }
        }
        if (i.type() != typeid(int32_t)){
            std::string errstr = "Used type: " + demangle(i.type().name());
            //println(errstr);
            throw std::runtime_error("Please use default integers or i32 for list indexing.");
        }
        int32_t index = std::any_cast<int32_t>(i);  
        std::vector<std::shared_ptr<type_instruction>> forward {nety(lobj),nety(lobj.get(index))};
        cplex["arrity"] = forward;
        forwardscplex[IN->id] = cplex;
        return lobj.get(index);
        }
        return std::any();

    }

    std::any interpretONode(std::shared_ptr<ONode> ON){
        Object x = ON->getValue();
        return x.GetStore();
    }

    std::any interpretStrongAssign(std::shared_ptr<StrongAssignNode> SANode){
        std::string var = SANode->getVarName();
        std::any type = interpretStatement(SANode->getType());
        
        std::any value = interpretStatement(SANode->getValue());
        if (compareAnyTy(type,value) == false){
                throw std::runtime_error("Incorrectly annotated type");
        }
        std::shared_ptr<Variable> vari = std::make_shared<Variable>(value,SANode->getMut());
        if (vari) {
            if (auto currentScope = getCurrentScope()) {
                currentScope->addVariable(var, vari); // issue here
            } else {
                std::cerr << "Runtime error: No current scope found\n";
            }
        }

        return nullification{};


        
    }
    std::any interpretIf(std::shared_ptr<IFNode> IfNode){
        //println("IF!");
        auto main = interpretStatement(IfNode->getMain()->getExpr());
        //println(demangle(typeid(IfNode->getMain()->getBody()).name()));


        
        if (main.type() != typeid(bool)){
            
            throw std::runtime_error("Expressions can only evaluate to a boolean (true or false) for if statements");
        }
        std::any firsttrue;
        std::any iftype;
        bool mainbool = std::any_cast<bool>(main);
        if (mainbool == true){
            //println("RETTING TRUE");
            //println(convertToString(interpretStatement(IfNode->getMain()->getBody())));
            if (AP.has("compile")){
                firsttrue = interpretStatement(IfNode->getMain()->getBody());
                iftype = firsttrue;
            } else {
            return interpretStatement(IfNode->getMain()->getBody());
            }
        } else if (AP.has("compile")){
            iftype = interpretStatement(IfNode->getMain()->getBody());
        }
        auto umap = IfNode->getElses();
        int a = 0;
        for (int i = 0; i < umap.size(); ++i) {
        auto stat = interpretStatement(umap[a]->getExpr());
        if (stat.type() != typeid(bool)){
            throw std::runtime_error("Expressions can only evaluate to a boolean (true or false) for elif statements");
        }
        bool elifbool = std::any_cast<bool>(stat);
        if (elifbool == true){
            if (AP.has("compile")){
                if (!firsttrue.has_value()){
                    firsttrue = interpretStatement(umap[a]->getBody());
                    if (inT<int>(IfNode->id,isused)){
                    if (firsttrue.type() != iftype.type()){
                        displayError("Else if condition returns a value of different type than if condition",lpos[umap[a]->getExpr()->id][0],lpos[umap[a]->getExpr()->id][1],lpos[umap[a]->getExpr()->id][2],{});
                    }
                    } 
                } else {
                    auto res = interpretStatement(umap[a]->getBody());
                    if (inT<int>(IfNode->id,isused)){
                    if (res.type() != iftype.type()){
                        displayError("Else if condition returns a value of different type than if condition",lpos[umap[a]->getExpr()->id][0],lpos[umap[a]->getExpr()->id][1],lpos[umap[a]->getExpr()->id][2],{});
                    }
                    }
                }
            } else {
            return interpretStatement(umap[a]->getBody());
            }
        } else if (AP.has("compile")){
            auto res = interpretStatement(umap[a]->getBody());
            if (inT<int>(IfNode->id,isused)){
            if (res.type() != iftype.type()){
                displayError("Else if condition returns a value of different type than if condition",lpos[umap[a]->getExpr()->id][0],lpos[umap[a]->getExpr()->id][1],lpos[umap[a]->getExpr()->id][2],{});
            }
            }
        }
        a += 1;
        }
        if (IfNode->getNot()){
            if (AP.has("compile")){
                if (!firsttrue.has_value()){
                    firsttrue = interpretStatement(IfNode->getNot()->getBody());
                    if (inT<int>(IfNode->id,isused)){
                    if (firsttrue.type() != iftype.type()){
                        displayError("Else condition returns a value of different type than if condition",lpos[IfNode->getNot()->getBody()->id][0],lpos[IfNode->getNot()->getBody()->id][1],lpos[IfNode->getNot()->getBody()->id][2]+2,gerr({})); // MAKE_BETTER
                    }
                    }
                } else {
                    auto res = interpretStatement(IfNode->getNot()->getBody());
                    if (inT<int>(IfNode->id,isused)){
                    if (res.type() != iftype.type()){
                        displayError("Else condition returns a value of different type than if condition",lpos[IfNode->getNot()->getBody()->id][0],lpos[IfNode->getNot()->getBody()->id][1],lpos[IfNode->getNot()->getBody()->id][2]+2,gerr({})); // MAKE_BETTER
                    }
                    }
                }
            } else {
            return interpretStatement(IfNode->getNot()->getBody());
            }

        } // interpret everything then set true value
        if (firsttrue.has_value()){
            return firsttrue;
        }
        return nullification{};
    }
    std::any interpretTSL(std::shared_ptr<TypeSafeListNode> TSLN){
        auto tlmap = TSLN->getValue();
        int a = 0;
        tsl::ordered_map<int,std::any> ttlmap;
        for (int i = 0; i < tlmap.size(); ++i) {
            std::any tadd = interpretStatement(tlmap[a]);
            auto tsln = interpretStatement(TSLN->getType());
            
            if (compareAnyTy(tadd,tsln) == false){ // patch here.
                //println(TSLN)

                throw std::runtime_error("Type of value added to TypeSafe list should be: " + getTypeName(tsln) + ".\nType got: " + getTypeName(tadd));
                
            }
            ttlmap[a] = tadd;
            a += 1;

        }
        //println("DONE TSL!");
        return TypeSafeList(ttlmap,interpretStatement(TSLN->getType()),ttlmap.size());
    }

    std::any interpretIdent(std::shared_ptr<IdentifierNode> IdentNode) {
        std::string var = IdentNode->getValue();
        
        auto currentScope = getCurrentScope();
        //println("Pre");
        std::shared_ptr<Variable> varobj = currentScope->getVariable(var);
        logat("Ident: " + varobj->getType(),"Interpret.iI");
        if (varobj->getType() == "int") {
            return varobj->getInt();
        } else if (varobj->getType() == "RawString") {
            return varobj->getString();
        } else if (varobj->getType() == "bool") {
            return varobj->getBool();
        } else if (varobj->getType() == "none") {
            return varobj->getNull();
        } else if (varobj->getType() == "nil") {
            return varobj->getnullv();
        } else if (varobj->getType() == "list") {
            return varobj->getList();
        } else if (varobj->getType() == "tuple") {
            return varobj->getTuple();
        } else if (varobj->getType() == "TSL") {
            return varobj->getTSL();
        } else if (varobj->getType() == "_struct") {
            return varobj->getStruct();
        } else if (varobj->getType() == "struct") {
            return varobj->getStructI();
        } else if (varobj->getType() == "enum") {
            return varobj->getEnum();
        } else if (varobj->getType() == "ek") {
            return varobj->getEK();
        } else if (varobj->getType() == "ptr") {
            //println("REF'D PTR");
            //exit(0);
            Pointer p = varobj->getPTR();
            p.pointsto(var);
            return p;
        } else if (varobj->getType() == "bptr") {
            return varobj->getBPTR();
        } else if (varobj->getType() == "UFN") {
            return varobj->getUFN();
        } else if (varobj->getType() == "MFN") {
            return varobj->getMFN();
        } else if (varobj->getType() == "CFN") {
            return varobj->getCFN();
        } else if (varobj->getType() == "Class") {
            return varobj->getClass();
        } else if (varobj->getType() == "ClassI") {
            return varobj->getClassI();
        } else if (varobj->getType() == "CEFN") {
            return varobj->getCEFN();
        } else if (varobj->getType() == "MLE") {
            return varobj->getModule();
        } else if (varobj->getType() == "tag") {
            return varobj->getTag();
        } else if (varobj->getType() == "tagF") {
            return varobj->getTagF();
        } else if (varobj->getType() == "MODM") {
            forwardscplex[IdentNode->id] = std::string(varobj->getModuleM().hash + varobj->getModuleM().vname);
            return varobj->getModuleM();
        } else if (varobj->getType() == "obj") {
            Object X = varobj->getObj();
            
            
            auto v =  X.GetStore();
            return v;
        }
        

        return nullification{};
    }

    
    std::any interpretBinOp(std::shared_ptr<BinOP> binOpNode) {
        std::any left = interpretStatement(binOpNode->getleft());
        std::any right = interpretStatement(binOpNode->getright());
        std::string op = binOpNode->getValue();
        std::vector<std::string> eqop = {"==","!=",">=","<=","<",">"};
        std::vector<std::string> inttypeNames = {
        "i8", "i16", "i32", "i64",    // Signed integer types
        "u8", "u16", "u32", "u64",    // Unsigned integer types
        "f32", "f64",                 // Floating-point types
        "isize","usize"
        };

        if (left.type() == typeid(Object)){
            left = std::any_cast<Object>(left).GetStore();
        }
        if (right.type() == typeid(Object)){
            right = std::any_cast<Object>(right).GetStore();
        }
        if (in(op,eqop)){

        if (left.type() != right.type() && (dummyValues.find(getTypeName(left)) == dummyValues.end() || dummyValues.find(getTypeName(right)) == dummyValues.end())){
        if (op == "=="){
            if (right.type() == typeid(Searchable) && left.type() == typeid(TypeSafeList)){
                auto tsl = std::dynamic_pointer_cast<TypeSafeListNode>(binOpNode->getleft());
                auto se = std::dynamic_pointer_cast<SearchableNode>(binOpNode->getright());
                if (se->getChecks().size() > tsl->getValue().size()){
                    throw std::runtime_error("gTC too high");
                }
                auto map = se->getChecks();
                if (se->getValue() == "rend"){

                for (int i = 0;i != map.size();i++){
                    if (std::any_cast<bool>(interpretStatement(std::make_shared<BinOP>(tsl->getValue()[i],map[i],"=="))) == false){
                        return false;
                    }
                }
                return true;
                }
                if (se->getValue() == "lend"){

                for (int i = tsl->getValue().size()-1;i != ((tsl->getValue().size()-1)-map.size());i--){


                    if (std::any_cast<bool>(interpretStatement(std::make_shared<BinOP>(tsl->getValue()[i],map[(tsl->getValue().size()-1)-i],"=="))) == false){
                        return false;
                    }
                }
                return true;
                }


            }
            if (left.type() == typeid(Searchable) && right.type() == typeid(TypeSafeList)){
                auto tsl = std::dynamic_pointer_cast<TypeSafeListNode>(binOpNode->getright());
                auto se = std::dynamic_pointer_cast<SearchableNode>(binOpNode->getleft());
                if (se->getChecks().size() > tsl->getValue().size()){
                    throw std::runtime_error("gTC too high");
                }
                auto map = se->getChecks();
                if (se->getValue() == "rend"){

                for (int i = 0;i != map.size();i++){
                    if (std::any_cast<bool>(interpretStatement(std::make_shared<BinOP>(tsl->getValue()[i],map[i],"=="))) == false){
                        return false;
                    }
                }
                return true;
                }
                if (se->getValue() == "lend"){

                for (int i = tsl->getValue().size()-1;i != 0;i--){
                    if (std::any_cast<bool>(interpretStatement(std::make_shared<BinOP>(tsl->getValue()[i],map[(tsl->getValue().size()-1)-i],"=="))) == false){
                        return false;
                    }
                }
                return true;
                }


            }
            return false;
        } else if (op == "!="){
            return true;
        }
        throw std::runtime_error("Comparing of two different types for: " + op + "\n" + getTypeName(left) + op + getTypeName(right));
        }
        if (left.type() == right.type() && (dummyValues.find(getTypeName(left)) == dummyValues.end())){

        if (op == "=="){
            return compareAnyTy(left,right) || compareAnyTy(right,left);
        } else if (op == "!="){
            return !(compareAnyTy(left,right) || compareAnyTy(right,left));
        }
        
                
        }}
        

        

        // Helper function to convert std::any to double


        try {
            if (in(getTypeName(left),inttypeNames)){

                double leftVal = std::any_cast<double>(convertToDouble(left));
                double rightVal = std::any_cast<double>(convertToDouble(right));
                

                double result;
                if (op == "+") {
                    result = leftVal + rightVal;
                    
                } else if (op == "-") {
                    result = leftVal - rightVal;
                } else if (op == "*") {
                    result = leftVal * rightVal;
                } else if (op == "/") {
                    if (rightVal == 0) {
                        std::cerr << "Runtime error: Division by zero\n";
                        return nullification{};
                    }
                    result = leftVal / rightVal;
                    //return Object(result,"f64");
                } else if (op == "==") {
                    return leftVal == rightVal;
                } else if (op == "!=") {
                    return leftVal != rightVal;
                } else if (op == ">") {
                    return leftVal > rightVal;
                } else if (op == "<") {
                    return leftVal < rightVal;
                } else if (op == ">=") {
                    return leftVal >= rightVal;
                } else if (op == "<=") {
                    return leftVal <= rightVal;
                } else if (op == "<<"){
                    result = static_cast<int64_t>(leftVal) << static_cast<int64_t>(rightVal);
                } else if (op == ">>"){
                    result = static_cast<int64_t>(leftVal) << static_cast<int64_t>(rightVal);
                } else if (op == "b|"){
                    result = static_cast<int64_t>(leftVal) | static_cast<int64_t>(rightVal);
                } else if (op == "b&"){
                    result = static_cast<int64_t>(leftVal) & static_cast<int64_t>(rightVal);
                } else if (op == "b^"){
                    result = static_cast<int64_t>(leftVal) ^ static_cast<int64_t>(rightVal);
                } 


                // Wrap result in Object and return perhaps hierachy?
                if (getTypeName(left) == "f64" || getTypeName(right) == "f64"){
                    return Object(result,"f64").GetStore();
                }
                if (getTypeName(left) == "i32" && getTypeName(right) == "i32"){
                    return static_cast<int>(result);
                }
                
                return Object(double(result), getTypeName(left)).GetStore();
            }

            // Handle string and character types
            if (left.type() == typeid(std::string) || left.type() == typeid(const char*) ||
                left.type() == typeid(char) || left.type() == typeid(wchar_t) ||
                left.type() == typeid(std::wstring)) {

                std::string leftStr;
                std::string rightStr;


                // Convert left to std::string
                if (left.type() == typeid(std::string)) {
                    leftStr = std::any_cast<std::string>(left);
                } else if (left.type() == typeid(const char*)) {
                    leftStr = std::string(std::any_cast<const char*>(left));
                } else if (left.type() == typeid(char)) {
                    leftStr = std::string(1, std::any_cast<char>(left));
                } else if (left.type() == typeid(std::wstring)) {
                    std::wstring wstr = std::any_cast<std::wstring>(left);
                    leftStr = std::string(wstr.begin(), wstr.end());
                }

                // Convert right to std::string
                if (right.type() == typeid(std::string)) {
                    rightStr = std::any_cast<std::string>(right);
                } else if (right.type() == typeid(const char*)) {
                    rightStr = std::string(std::any_cast<const char*>(right));
                } else if (right.type() == typeid(char)) {
                    rightStr = std::string(1, std::any_cast<char>(right));
                } else if (right.type() == typeid(std::wstring)) {
                    std::wstring wstr = std::any_cast<std::wstring>(right);
                    rightStr = std::string(wstr.begin(), wstr.end());
                }

                std::any result;
                if (op == "==") {

                    result = leftStr == rightStr;
                } else if (op == "!=") {
                    result = leftStr != rightStr;
                } else if (op == "+"){
                    result = leftStr + rightStr;
                }
                // Wrap result in Object and return
                return result;
                //return Object(std::any_cast<bool>(result), "bool");
            }

            // Handle logical operations for bool
            if ((left.type() == typeid(bool) || right.type() == typeid(bool))) {
                bool leftBool = std::any_cast<bool>(left);
                bool rightBool = std::any_cast<bool>(right);

                std::any result;
                if (op == "&&") {
                    result = leftBool && rightBool;
                } else if (op == "||") {
                    result = leftBool || rightBool;
                } else if (op == "==") {
                    result = leftBool == rightBool;
                } else if (op == "!=") {
                    result = leftBool != rightBool;
                }


                // Wrap result in Object and return
                return result;
            }
            if (left.type() == typeid(nullification)){
                if (right.type() != typeid(nullification) && op == "=="){
                    return false;
                } else if (right.type() == typeid(nullification) && op == "=="){
                    return true;
                } else if (right.type() != typeid(nullification) && op == "!="){
                    return true;
                } else if (right.type() == typeid(nullification) && op == "!="){
                    return false;
                }
            }
            if (right.type() == typeid(nullification)){
                if (left.type() != typeid(nullification) && op == "=="){
                    return false;
                } else if (left.type() == typeid(nullification) && op == "=="){
                    return true;
                } else if (left.type() != typeid(nullification) && op == "!="){
                    return true;
                } else if (left.type() == typeid(nullification) && op == "!="){
                    return false;
                }
            }
            if (left.type() == right.type() && left.type() == typeid(TypeEnumKey)){
                if (op == "=="){
                    try {
                    return std::any_cast<TypeEnumKey>(left).getId() == std::any_cast<TypeEnumKey>(right).getId();
                } catch (std::bad_any_cast){
                    return false;
                }
                }

            }

        } catch (const std::bad_any_cast& e) {
            std::cerr << "Bad any cast: " << e.what() << std::endl;
        }

        
        std::cerr << "Interpreter error: Invalid binary operation\n" << getTypeName(left) << op << getTypeName(right);
        return nullification{};
    }

    std::any interpretderef(std::shared_ptr<DerefNode> DN){
        std::any pobptr = interpretStatement(DN->getExpr());
        if (pobptr.type() == typeid(Pointer)){
            
            
            std::any tr = (*std::any_cast<Pointer>(pobptr).getptr()).getValue();
            forwards[DN->id] = nety(std::any_cast<Pointer>(pobptr).getptr()->getValue());
            
            
            
            return tr;

        } else if (pobptr.type() == typeid(BorrowedPointer)){
           //println("PRE-DIS");
            ////println(convertToString((*std::any_cast<BorrowedPointer>(pobptr).getptr()).getValue()));
            //println("WORKS?");
            if(std::any_cast<BorrowedPointer>(pobptr).getptr() == nullptr){
                throw std::runtime_error("????");
            }
            forwards[DN->id] = nety(std::any_cast<BorrowedPointer>(pobptr).getptr()->getValue());
            return (*(std::any_cast<BorrowedPointer>(pobptr).getptr())).getValue();
        }
        else if (auto idn = std::dynamic_pointer_cast<IdentifierNode>(DN->getExpr())){
            if (idn->getValue() == "self"){
                println("Moving forward");
                forwards[DN->id] = nety(pobptr);
                println("Moved forward");
                return pobptr;
            } else {
                throw std::runtime_error("Dereferencing is only supported for Pointers or BorrowedPointers\nGot: " + demangle(pobptr.type().name()));
            }
        }
        throw std::runtime_error("Dereferencing is only supported for Pointers or BorrowedPointers\nGot: " + demangle(pobptr.type().name()));
    }


    std::any interpretAssign(std::shared_ptr<AssignNode> node) {
        //println("beginning assign!");

        auto varName = node->getVarName();
        auto valueNode = node->getValue();
        int vid = 0;
        //println("POST EXEC ASSIGN");
        if (auto id = std::dynamic_pointer_cast<IdentifierNode>(valueNode)){
            vid = getCurrentScope()->getVariable(id->getValue())->valueid;
        }

        std::any result = interpretStatement(valueNode);
        if (result.type() == typeid(NoAssign)){
            return nullification{};
        }


        //println("Beginning assign");
        //println("MUTSTATUS: ");
        //println(convertToString(node->getMut()));
        std::shared_ptr<Variable> var;
        if (result.type() == typeid(int32_t)) {
            var = std::make_shared<Variable>(std::any_cast<int32_t>(result),node->getMut());
        } else if (result.type() == typeid(std::string)) {
            var = std::make_shared<Variable>(std::any_cast<std::string>(result),node->getMut());
        } else if (result.type() == typeid(bool)) {
            var = std::make_shared<Variable>(std::any_cast<bool>(result),node->getMut());
        } else if (result.type() == typeid(nullification)) {
            var = std::make_shared<Variable>(nullification{},node->getMut());
        }/**/ else if (result.type() == typeid(List)) {
            var = std::make_shared<Variable>(std::any_cast<List>(result),node->getMut());
        } else if (result.type() == typeid(TypeSafeList)) {
            TypeSafeList tsl = std::any_cast<TypeSafeList>(result);
            var = std::make_shared<Variable>(tsl,node->getMut());
        } else if (result.type() == typeid(Tuple)) {
            var = std::make_shared<Variable>(std::any_cast<Tuple>(result),node->getMut());
        } else if (result.type() == typeid(StructDecl)) {
            var = std::make_shared<Variable>(std::any_cast<StructDecl>(result),node->getMut());
        } else if (result.type() == typeid(StructInstance)) {
            auto SI = std::any_cast<StructInstance>(result);
            var = std::make_shared<Variable>(SI,node->getMut());
        } else if (result.type() == typeid(Enum)) {
            var = std::make_shared<Variable>(std::any_cast<Enum>(result),node->getMut());
        } else if (result.type() == typeid(EnumKey)) {
            var = std::make_shared<Variable>(std::any_cast<EnumKey>(result),node->getMut());
        } else if (result.type() == typeid(Pointer)) {
            Pointer ptr = std::any_cast<Pointer>(result);
            if (ptr.getpt() != varName && ptr.getpt() != ""){

            
            
            ptrnowptsto[ptr.getpt()] = varName;
            getCurrentScope()->removeVariable(ptr.getpt(),false);
            }
            var = std::make_shared<Variable>(ptr,node->getMut());
        } else if (result.type() == typeid(BorrowedPointer)) {
            BorrowedPointer ptr = std::any_cast<BorrowedPointer>(result);
            var = std::make_shared<Variable>(ptr,node->getMut());
        } else if (result.type() == typeid(MappedFunction)) {
            var = std::make_shared<Variable>(std::any_cast<MappedFunction>(result),node->getMut());
        } else if (result.type() == typeid(Class)) {
            var = std::make_shared<Variable>(std::any_cast<Class>(result),node->getMut());
        } else if (result.type() == typeid(ClassInstance)) {
            var = std::make_shared<Variable>(std::any_cast<ClassInstance>(result),node->getMut());
        } else if (result.type() == typeid(std::shared_ptr<Future>)) {
            throw std::runtime_error("Future is not assignable!");
        } else if (result.type() == typeid(CEXTFunction)) {
            var = std::make_shared<Variable>(std::any_cast<CEXTFunction>(result),node->getMut());
        } else if (result.type() == typeid(Module)) {
            var = std::make_shared<Variable>(std::any_cast<Module>(result),node->getMut());
        } else if (result.type() == typeid(ModuleMethod)) {
            var = std::make_shared<Variable>(std::any_cast<ModuleMethod>(result),node->getMut());
        } else if (result.type() == typeid(Tag)) {
            var = std::make_shared<Variable>(std::any_cast<Tag>(result),node->getMut());
        } else if (result.type() == typeid(TagFunction)) {
            var = std::make_shared<Variable>(std::any_cast<TagFunction>(result),node->getMut());
        } else if (result.type() == typeid(Object)) {

            auto obj = std::any_cast<Object>(result);
             // try to access value
            var = std::make_shared<Variable>(obj,node->getMut());
        } else {
            //println("Unrecognised type: " + demangle(result.type().name()));
            Object ovar = Object(result,getTypeName(result));
            ovar.GetStore(); // try to access value
            var = std::make_shared<Variable>(std::any_cast<Object>(ovar),node->getMut());
        }

        if (var) {
            if (auto currentScope = getCurrentScope()) {
                if (vid != 0){
                    var->setVID( vid);
                }

                currentScope->addVariable(varName, var);
                for (auto& id:vidids){
                    forwardscplex[id] = var->valueid;
                    vidids.clear();
                }
            } else {
                std::cerr << "Runtime error: No current scope found\n";
            }
        }
        

        return nullification{};
    }
    void insertscope(std::shared_ptr<Scope> ins){
        swapscope.push_back(getCurrentScope());

    }
    std::vector<std::shared_ptr<Scope>> swapscope;
    private:


};
#endif