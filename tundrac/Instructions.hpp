#include "base.hpp"
#include "interpreterc.hpp"
#include "Scope.hpp"
#include "Tokens.hpp"
#include "AST.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"
#ifndef INSTRUCTIONS
#define INSTRUCTIONS
class Instruct {
    public:
    std::shared_ptr<function_instruction> mainfn;
    std::shared_ptr<BlockNode> rootblock;
    bool isassignflag=false;
    Instruct(std::shared_ptr<BlockNode> rootblock): rootblock(rootblock){

    }

    std::shared_ptr<Instruction> handleILN(std::shared_ptr<IntLiteralNode> in){
        return std::make_shared<resource_instruction>(std::string("i32"),in->getValue());
    }
    std::shared_ptr<Instruction> handleObject(std::shared_ptr<ONode> obj){
        return std::make_shared<resource_instruction>(std::string(obj->getValue().getType()),obj->getValue().GetStore());
    }

    std::shared_ptr<Instruction> handleSLN(std::shared_ptr<StringLiteralNode> sln){
        return std::make_shared<resource_instruction>(std::string("RawString"),sln->getValue());
    }

    std::shared_ptr<block_instruction> handleBlock(std::shared_ptr<BlockNode> bn,bool ismain=false){
        std::vector<std::shared_ptr<Instruction>> insts;

        for (auto& stat : bn->getStatements()){

            auto ptr = instruct(stat);
            if (ptr){
                insts.push_back(ptr);
            }
        }
        return std::make_shared<block_instruction>(ismain,insts);
    }

    std::shared_ptr<Instruction> handleTag(std::string vn,std::shared_ptr<TagNode> TN){
        return std::make_shared<tag_instruction>(std::dynamic_pointer_cast<function_instruction>(handleMFN(vn,std::dynamic_pointer_cast<MappedFunctionNode>(TN->getValue()))));
    }

    std::shared_ptr<Instruction> handleAssign(std::shared_ptr<AssignNode> AN){
        bool prev = isassignflag;
        
        if (auto MFN = std::dynamic_pointer_cast<MappedFunctionNode>(AN->getValue())){
            return handleMFN(AN->getVarName(),MFN);
        }
        if (auto SDN = std::dynamic_pointer_cast<StructDeclNode>(AN->getValue())){
            return handleStructDecl(AN->getVarName(),SDN);
        }
        if (auto Mod = std::dynamic_pointer_cast<ImportNode>(AN->getValue())){
            return handleModule(Mod);
        }
        if (auto Tag = std::dynamic_pointer_cast<TagNode>(AN->getValue())){
            return handleTag(AN->getVarName(),Tag);
        }
        if (auto IFN = std::dynamic_pointer_cast<IFNode>(AN->getValue())){
            auto node = std::make_shared<assign_instruction>(AN->getVarName(),handleIf(IFN,true));
            isassignflag = prev;
            return node;
        }
        if (auto WN = std::dynamic_pointer_cast<WhileNode>(AN->getValue())){
            auto node = std::make_shared<assign_instruction>(AN->getVarName(),handleWhile(WN,true));
            isassignflag = prev;
            return node;
        }
        isassignflag = true;
        
        auto node = std::make_shared<assign_instruction>(AN->getVarName(),instruct(AN->getValue()),false);
        isassignflag = prev;
        return node;
    }
    // struct_instance_instruction


    std::shared_ptr<Instruction> handleIdentifier(std::shared_ptr<IdentifierNode> IN){
        logat("Receiving " + IN->getValue() ,"Ins.hI");
        if (dummyinstructions.find(IN->getValue()) != dummyinstructions.end()){
            logat("Received dummy " + IN->getValue() ,"Ins.hI");
            return dummyinstructions[IN->getValue()];
        }
        std::shared_ptr<load_instruction> linst;
        if (forwardscplex.find(IN->id) != forwardscplex.end()){
            linst = std::make_shared<load_instruction>(std::any_cast<std::string>(forwardscplex[IN->id]),isassignflag);
        } else {
            linst = std::make_shared<load_instruction>(IN->getValue(),isassignflag);
        }
        
        logat("Received " + linst->getName() ,"Ins.hI");
        return linst;
    }
    std::shared_ptr<Instruction> handleOperator(std::shared_ptr<BinOP> ON){
        return std::make_shared<operation_instruction>(instruct(ON->getleft()),instruct(ON->getright()),ON->getValue());
    }

    std::shared_ptr<Instruction> handleExtern(std::shared_ptr<ExternNode> EN){
        auto vecnode = EN->getValue();
        auto fnname = EN->getfnnames();
        auto isvdic = EN->getvdic();
        std::vector<std::shared_ptr<Instruction>> fninsts;
        for (int i = 0; i != vecnode.size();i++){
            auto fn = std::dynamic_pointer_cast<MappedFunctionNode>(vecnode[i]);
            std::vector<std::shared_ptr<Instruction>> args;
            auto fnnargs = fn->getValue();
            std::shared_ptr<Instruction> returnty = instruct(fnnargs["-!retty"]);
            fnnargs.erase("-!retty");
            for (auto & arg : fnnargs){
                args.push_back(instruct(arg.second));
            }
            fninsts.push_back(std::make_shared<declare_function_instruction>(fnname[i],args,returnty,in(fnname[i],isvdic)));

        }
        return std::make_shared<block_instruction>(false,fninsts); // MAKE THIS BETTER INTO A CHAIN INSTRUCTION
    }

    std::shared_ptr<Instruction> handleMFN(std::string name="",std::shared_ptr<MappedFunctionNode> MFN=nullptr){
        bool anon = false;
        if (name == ""){
            name = "anonfn" + std::to_string(bc);
            bc++;
            anon = true;
        }
        bool isnoneret = std::any_cast<bool>(forwardscplex2[MFN->id]);
 
        std::vector<std::shared_ptr<Instruction>> args;
        auto fnnargs = MFN->getValue();
        auto retty = fnnargs["-!retty"];
        auto intes = MFN->getinternals();
        std::shared_ptr<Instruction> returnty = instruct(fnnargs["-!retty"]);
        fnnargs.erase("-!retty");
        for (auto & arg : fnnargs){
            if (arg.first == "self" && MFN->isselfptr){
                args.push_back(std::make_shared<pointer_instruction>(instruct(arg.second)));
            } else {
            args.push_back(instruct(arg.second));
            }
        }
        std::vector<std::string> argnames;
        for (auto& argn : MFN->gettick()){
            if (intes.find(argn.second) != intes.end()){
                argnames.push_back(intes[argn.second]);
            } else {
            
                argnames.push_back(argn.second);
            }
        }
        std::shared_ptr<BlockNode> blk;
        if (anon && std::dynamic_pointer_cast<BlockNode>(MFN->getBody())->StatementsLen() == 0){
            blk = std::dynamic_pointer_cast<BlockNode>(MFN->getBody());
            blk->addStatement(std::make_shared<RetNode>(std::make_shared<IdentifierNode>(getTypeName(forwardscplex[MFN->id]))));
        } else {
            blk = std::dynamic_pointer_cast<BlockNode>(MFN->getBody());
        }
        
        return std::make_shared<function_instruction>(name,returnty,args,std::dynamic_pointer_cast<block_instruction>(instruct(blk)),false,false,false,argnames,"",isnoneret);
    }
    std::shared_ptr<Instruction> handleConv(std::shared_ptr<ConventionNode> CN){
        auto pm = mangle_rule;
        mangle_rule = CN->getValue();
        std::vector<std::shared_ptr<Instruction>> insts;
        for (auto& inst : CN->getPub()){
            insts.push_back(instruct(inst));
        }
        mangle_rule = pm;
        return std::make_shared<chain_instruction>(insts);
    }
    std::shared_ptr<Instruction> handlePub(std::shared_ptr<PubNode> PN){
        if (auto aas = std::dynamic_pointer_cast<AssignNode>(PN->getValue())){
            if (auto MFN = std::dynamic_pointer_cast<MappedFunctionNode>(aas->getValue())){
                auto name = aas->getVarName();
                std::vector<std::shared_ptr<Instruction>> args;
                auto fnnargs = MFN->getValue();
                auto internals = MFN->getinternals();
                std::shared_ptr<Instruction> returnty = instruct(fnnargs["-!retty"]);
                fnnargs.erase("-!retty");
                for (auto & arg : fnnargs){
                    args.push_back(instruct(arg.second));
                }
                std::vector<std::string> argnames;
                for (auto& argn : MFN->gettick()){
                    if (internals.find(argn.second) == internals.end()){
                        argnames.push_back(argn.second);
                    } else {
                        argnames.push_back(internals[argn.second]);
                    }
                }
                bool isnoneret = std::any_cast<bool>(forwardscplex2[MFN->id]);
                /*if (!AP.has("-bundle") && result != "package"){
                    return std::make_shared<function_instruction>(name,returnty,args,std::dynamic_pointer_cast<block_instruction>(instruct(MFN->getBody())),false,true,false,argnames,"",isnoneret);
                } else {*/
                std::hash<std::string> hashery{};
                if (mangle_rule == "t4"){
                    return std::make_shared<function_instruction>(name,returnty,args,std::dynamic_pointer_cast<block_instruction>(instruct(MFN->getBody())),false,true,false,argnames,std::to_string(hashery(AP.values["compile"][0])),isnoneret);
                } else if (mangle_rule == "C"){
                    return std::make_shared<function_instruction>(name,returnty,args,std::dynamic_pointer_cast<block_instruction>(instruct(MFN->getBody())),false,true,false,argnames,"",isnoneret);
                }
                //}
            }
        }
    }

    std::shared_ptr<Instruction> handleCall(std::shared_ptr<CallNode> CN){


        auto db = std::any_cast<tsl::ordered_map<std::string,std::any>>(forwardscplex[CN->id]);
        auto clr = std::any_cast<std::string>(db["type"]);
        if (clr == "mappedfunction" || clr == "tag"){

        std::vector<std::shared_ptr<Instruction>> args; // need to forward the mapped function
        tsl::ordered_map<int,std::shared_ptr<ASTNode>> finalargs; // sincerely think about this before doing it lmao
        tsl::ordered_map<std::string,std::shared_ptr<ASTNode>> finalstrargs; 
        auto MFN = std::any_cast<MappedFunction>(db["mfn"]);
        auto CNargs = CN->getExpr();
        if (clr == "tag"){
            CNargs = std::any_cast<tsl::ordered_map<int, std::shared_ptr<ASTNode>>>(db["astack"]);
        }
        auto inserts = CN->getinserts();
        auto tick = MFN.gettick();
        auto strarg = MFN.getargs();
        for (int i = 0;i != CNargs.size();i++){
            if (tick.find(i) != tick.end()){
                finalstrargs[tick[i]] = CNargs[i];
            } else {
                finalstrargs["arg" + std::to_string(i)] = CNargs[i];
            }
        }
        for (auto& insert : inserts){
            finalstrargs[insert.first] = std::move(insert.second);
        }
        int i = 0;
        for (auto& strarg: finalstrargs){
            bool checker = false;
            reversefind<int,std::string>(tick,strarg.first,&checker);
            if (checker){
                auto v = *reversefind<int,std::string>(tick,strarg.first);

                finalargs[v] = std::move(strarg.second);
            } else {
                finalargs[i] = std::move(strarg.second);
            }
            i++;
        }
        for (int i = 0;i != finalargs.size();i++){
            if (tick[i] == "self" && MFN.isselfptr){
                auto inst = instruct(finalargs[i]);
                if (auto li = std::dynamic_pointer_cast<load_instruction>(inst)){
                    li->setGiveBackPtr(true);
                    args.push_back(li);
                } else {
                    args.push_back(inst);
                }

                
            } else {
                args.push_back(instruct(finalargs[i]));
            }
            
        }
        //if (MFN) push variadic back
        return std::make_shared<call_instruction>(instruct(CN->getBody()),args,std::any_cast<std::shared_ptr<type_instruction>>(db["fty"]));
        }
        return std::make_shared<nop_instruction>();
    }
    std::shared_ptr<Instruction> handleDecorator(std::shared_ptr<DecoratorNode> DN){
        if (auto CN = std::dynamic_pointer_cast<CallNode>(DN->getClr())){
            if (auto AN = std::dynamic_pointer_cast<AssignNode>(DN->getFn())){
                if (auto FN = std::dynamic_pointer_cast<MappedFunctionNode>(AN->getValue())){
                    auto res = handleCall(CN);
                    if (!inT<int>(CN->id,issig)){
                    return std::make_shared<assign_instruction>(AN->getVarName(),res);
                    } else {
                        return res;
                    }
                }
            }
        }
    }

    std::shared_ptr<Instruction> handleRet(std::shared_ptr<RetNode> RN){
        return std::make_shared<return_instruction>(instruct(RN->getValue()));
    }


    std::shared_ptr<Instruction> handleIf(std::shared_ptr<IFNode> IFN,bool isassign = false){
        auto elifs = IFN->getElses();
        std::vector<std::shared_ptr<if_instruction>> insts = {};
        for (auto& elif : elifs){
            insts.push_back(std::make_shared<if_instruction>(instruct(elif.second->getExpr()),std::dynamic_pointer_cast<block_instruction>(instruct(elif.second->getBody()))));
        }
        if (IFN->getNot()){
        return std::make_shared<if_instruction>(instruct(IFN->getMain()->getExpr()),std::dynamic_pointer_cast<block_instruction>(instruct(IFN->getMain()->getBody())),std::dynamic_pointer_cast<block_instruction>(instruct(IFN->getNot()->getBody())),insts,isassign || inT<int>(IFN->id,isused));
        } else {
            return std::make_shared<if_instruction>(instruct(IFN->getMain()->getExpr()),std::dynamic_pointer_cast<block_instruction>(instruct(IFN->getMain()->getBody())),nullptr,insts,isassign || inT<int>(IFN->id,isused));
        }
    }
     std::shared_ptr<Instruction> handleWhile(std::shared_ptr<WhileNode> WN,bool isassign=false){
        return std::make_shared<while_instruction>(instruct(WN->getExpr()->getExpr()),std::dynamic_pointer_cast<block_instruction>(instruct(WN->getExpr()->getBody())),isassign || inT<int>(WN->id,isused));
     }


    std::shared_ptr<Instruction> handlePointer(std::shared_ptr<PointerNode> PN){
        if (!PN->getBorrow()){
            return std::make_shared<pointer_instruction>(instruct(PN->getExpr()),false);
        } else {
            return std::make_shared<borrowed_pointer_instruction>(instruct(PN->getExpr()));
        }
    }
    std::shared_ptr<Instruction> handleDereference(std::shared_ptr<DerefNode> DN){
        return std::make_shared<dereference_instruction>(instruct(DN->getExpr()),forwards[DN->id]);
    }

    std::shared_ptr<Instruction> handlePointerStore(std::shared_ptr<ModifyPtrNode> MPN){
        return std::make_shared<pointer_store_instruction>(instruct(MPN->getLHS()),instruct(MPN->getExpr()));
    }
    std::shared_ptr<Instruction> handleStructDecl(std::string name,std::shared_ptr<StructDeclNode> SDN){
        std::vector<std::shared_ptr<Instruction>> fields;
        for (auto& field : SDN->getValue()){
            fields.push_back(instruct(field.second));
        }
        return std::make_shared<struct_decl_instruction>(name,fields);
    }

    std::shared_ptr<Instruction> handleStructInstance(std::shared_ptr<StructInstanceNode> SIN){
        std::vector<std::shared_ptr<Instruction>> fields;
        std::vector<std::string> sfields;
        for (auto& field : SIN->getValue()){
            fields.push_back(instruct(field.second));
            sfields.push_back(field.first);
        }
        return std::make_shared<struct_instance_instruction>(instruct(SIN->getBase()),fields,sfields,false,std::any_cast<int>(forwardscplex[SIN->id]));
    }
    std::shared_ptr<Instruction> handleExprAssign(std::shared_ptr<ExprAssignNode> EAN){
        return std::make_shared<alt_assign_instruction>(instruct(EAN->getVarName()),instruct(EAN->getValue()));
    }
    std::shared_ptr<Instruction> handleMemberAccess(std::shared_ptr<MemAccNode> MAN,bool privassign=false){
        auto db = std::any_cast<tsl::ordered_map<std::string,std::any>>(forwardscplex[MAN->id]);
        std::string accessty = std::any_cast<std::string>(db["type"]);
        int vid = std::any_cast<int>(db["vid"]);
        if (MAN->getAssign()){
            MAN->setAssign(false); // we're done with wrapping it
            return std::make_shared<alt_assign_instruction>(handleMemberAccess(MAN,true),instruct(MAN->getAssignv()));
        }
        if (accessty == "structI"){
        std::string sity = std::any_cast<std::string>(db["siaccesstype"]);
        if (sity == "member"){
        std::vector<std::shared_ptr<type_instruction>> vec{std::any_cast<std::shared_ptr<type_instruction>>(db["structI"]),std::any_cast<std::shared_ptr<type_instruction>>(db["memtype"])};
        if (auto sln = std::dynamic_pointer_cast<StringLiteralNode>(MAN->getNxt())){
            return std::make_shared<access_struct_member_instruction>(instruct(MAN->getValue()),sln->getValue(),std::any_cast<int>(db["id"]),vec,privassign);
        } else  if (auto M2 = std::dynamic_pointer_cast<MemAccNode>(MAN->getNxt())){
            return std::make_shared<access_struct_member_instruction>(handleMemberAccess(M2),std::dynamic_pointer_cast<StringLiteralNode>(M2->getNxt())->getValue(),std::any_cast<int>(db["id"]),vec,privassign);
        }
        } else { // cons or method
        
        if (auto sln = std::dynamic_pointer_cast<StringLiteralNode>(MAN->getNxt())){
            return std::make_shared<access_extend_instruction>(instruct(MAN->getValue()),sln->getValue(),sity == "method",sity == "method" ? 0 : vid);
        } else  if (auto M2 = std::dynamic_pointer_cast<MemAccNode>(MAN->getNxt())){
            return std::make_shared<access_extend_instruction>(handleMemberAccess(M2),std::dynamic_pointer_cast<StringLiteralNode>(M2->getNxt())->getValue(),sity == "method",sity == "method" ? 0 : vid);
        }

        }
        } else if (accessty == "tuple"){
        if (db.find("inferred") != db.end()){
            return instruct(std::any_cast<std::shared_ptr<MemAccNode>>(db["inferred"]));
        }
        std::vector<std::shared_ptr<type_instruction>> vec{std::any_cast<std::shared_ptr<type_instruction>>(db["tuple"]),std::any_cast<std::shared_ptr<type_instruction>>(db["memtype"])};
        if (auto sln = std::dynamic_pointer_cast<StringLiteralNode>(MAN->getNxt())){
            return std::make_shared<access_tuple_member_instruction>(instruct(MAN->getValue()),std::any_cast<int>(db["index"]),vec);
        } else  if (auto M2 = std::dynamic_pointer_cast<MemAccNode>(MAN->getNxt())){
            return std::make_shared<access_tuple_member_instruction>(handleMemberAccess(M2),std::any_cast<int>(db["id"]),vec);
        }
        } else if (accessty == "module"){
        auto mod = std::any_cast<Module>(db["mod"]);
        
        if (auto sln = std::dynamic_pointer_cast<StringLiteralNode>(MAN->getNxt())){
            return std::make_shared<load_instruction>(mod.hash + sln->getValue());
        } // module within a module is a nogo jose;
        
        } else if (accessty == "tag"){
            auto sln = std::dynamic_pointer_cast<StringLiteralNode>(MAN->getNxt());
            return std::make_shared<get_tag_instruction>(std::dynamic_pointer_cast<load_instruction>(handleIdentifier(std::make_shared<IdentifierNode>(sln->getValue()))),std::any_cast<std::shared_ptr<type_instruction>>(db["callertype"]));
        }
    }

    std::shared_ptr<Instruction> handleTuple(std::shared_ptr<TupleNode> TN){
        std::vector<std::shared_ptr<Instruction>> elements;
        for (auto& v : TN->getValue()){
            elements.push_back(instruct(v.second));
        }
        return std::make_shared<tuple_instruction>(elements);
    }

    std::shared_ptr<Instruction> handleCast(std::shared_ptr<CastNode> CN){
        auto tinst = forwards[CN->id];
        return std::make_shared<cast_instruction>(instruct(CN->getValue()),tinst);
    }

    std::shared_ptr<Instruction> handleTSL(std::shared_ptr<TypeSafeListNode> TSLN){
        std::vector<std::shared_ptr<Instruction>> elems;
        for (auto& i : TSLN->getValue()){
            elems.push_back(instruct(i.second));
        }
        return std::make_shared<array_instruction>(elems,elems.size());
    }
    std::shared_ptr<Instruction> handleTSLInit(std::shared_ptr<TSLInitNode> TSLIN){
        std::vector<std::shared_ptr<Instruction>> elems;
        auto sz = std::any_cast<int32_t>(forwardscplex[TSLIN->id]);
        auto size = sz;

        return std::make_shared<init_array_instruction>(instruct(TSLIN->getExpr()),sz);
    }
    //std::vector<std::shared_ptr<type_instruction>>
    std::shared_ptr<Instruction> handleIndex(std::shared_ptr<IndexNode> IN){
        auto db = std::any_cast<tsl::ordered_map<std::string,std::any>>(forwardscplex[IN->id]);
        auto vec = std::any_cast<std::vector<std::shared_ptr<type_instruction>>>(db["arrity"]);
        std::string ty = std::any_cast<std::string>(db["ty"]);
        if (ty == "arr"){
        return std::make_shared<access_array_instruction>(instruct(IN->getValue()),vec,instruct(IN->getIndex()));
        } else if (ty == "ptr"){
            return std::make_shared<access_pointer_instruction>(instruct(IN->getValue()),vec,instruct(IN->getIndex()));
        }
    }
    std::shared_ptr<Instruction> handleExtend(std::shared_ptr<ModNode> MN){
        auto db = std::any_cast<tsl::ordered_map<std::string,std::any>>(forwardscplex[MN->id]);
        auto extty = std::any_cast<std::string>(db["type"]);
        if (extty == ""){
        std::vector<std::shared_ptr<Instruction>> exts;
        for (auto& ext: MN->getStates()){
            exts.push_back(instruct(ext));
        }
        return std::make_shared<extend_instruction>(instruct(MN->getValue()),exts);
        } else if (extty == "tag"){
            std::vector<std::shared_ptr<Instruction>> exts;
            for (auto& ext: MN->getStates()){
                exts.push_back(std::make_shared<tag_instruction>(std::dynamic_pointer_cast<function_instruction>(instruct(ext))));
            }
            return std::make_shared<chain_instruction>(exts);
        }
    }
    std::shared_ptr<Instruction> handleSZ(std::shared_ptr<SizeOfNode> SON){
        return std::make_shared<size_of_instruction>(instruct(SON->getValue()));
    }
    std::shared_ptr<Instruction> handleStackSize(std::shared_ptr<StackSizeNode> SSN){
        return std::make_shared<stack_size_instruction>();
    }
    std::shared_ptr<Instruction> handleRef(std::shared_ptr<RefNode> RN){
        auto inst = instruct(RN->getValue());
        inst->setGiveBackPtr(true);
        return inst;
    }
    std::shared_ptr<Instruction> handleTY(std::shared_ptr<TypeIDNode> TIDN){
        std::shared_ptr<type_instruction> ty = nullptr;
        if (forwards.find(TIDN->id) != forwards.end()){
            ty = forwards[TIDN->id];
        }
        return std::make_shared<type_id_instruction>(instruct(TIDN->getValue()),ty);
    }
    std::unordered_map<std::string,std::shared_ptr<Instruction>> asmUTIL(std::unordered_map<std::string,std::shared_ptr<ASTNode>> v){
        std::unordered_map<std::string,std::shared_ptr<Instruction>> ret;
        for (auto& x : v){
            ret[x.first] = instruct(x.second);
        }
        return ret;

    }
    std::shared_ptr<Instruction> handleASM(std::shared_ptr<ASMNode> ASMN){
        return std::make_shared<asm_instruction>(std::make_shared<resource_instruction>("ASMSTR",ASMN->ASMStr),ASMN->regs,asmUTIL(ASMN->in),asmUTIL(ASMN->out),asmUTIL(ASMN->inout));
    }
    std::shared_ptr<Instruction> handleP2I(std::shared_ptr<PtrtointNode> PTIN){
        return std::make_shared<ptrtoint_instruction>(instruct(PTIN->getValue()));
    }

    std::shared_ptr<Instruction> handleChain(std::shared_ptr<ChainNode> ChN){
        std::vector<std::shared_ptr<Instruction>> insts;
        for (auto& ch : ChN->getChain()){
            insts.push_back(instruct(ch));
        }
        return std::make_shared<chain_instruction>(insts);
    }

    std::shared_ptr<Instruction> handleModule(std::shared_ptr<ImportNode> ImN){
        Module mod = std::any_cast<Module>(forwardscplex[ImN->id]);
        std::vector<std::shared_ptr<Instruction>> insts;
        auto from = ImN->getFrom();
        for (auto& method : mod.methods){
            //auto mfn = std::any_cast<MappedFunction>(method.second);
            auto vec = mod.MNodes[method.first];
            std::vector<std::shared_ptr<Instruction>> args;
            
            std::shared_ptr<Instruction> retty = instruct(vec[vec.size()-1]);
            vec.pop_back();

            for (auto& arg: vec){
                args.push_back(instruct(arg));
            }


            std::shared_ptr<Instruction> returnty;
            if (!from.empty()){
                if (in(method.first,from)){
                    insts.push_back(std::make_shared<declare_function_instruction>(mod.hash + method.first,args,retty,false));
                }
            } else {
                insts.push_back(std::make_shared<declare_function_instruction>(mod.hash + method.first,args,retty,false));
            }
        }
        return std::make_shared<chain_instruction>(insts);
    }
    std::shared_ptr<Instruction> instruct(std::shared_ptr<ASTNode> node){
        if (auto in = std::dynamic_pointer_cast<IntLiteralNode>(node)){
            return handleILN(in);
        } else if (auto Object = std::dynamic_pointer_cast<ONode>(node)){
            return handleObject(Object);
        } else if (auto StrLit = std::dynamic_pointer_cast<StringLiteralNode>(node)){
            return handleSLN(StrLit);
        } else if (auto AN = std::dynamic_pointer_cast<AssignNode>(node)){
            return handleAssign(AN);
        } else if (auto IN = std::dynamic_pointer_cast<IdentifierNode>(node)){
            return handleIdentifier(IN);
        } else if (auto BN = std::dynamic_pointer_cast<BlockNode>(node)){
            return handleBlock(BN);
        } else if (auto ON = std::dynamic_pointer_cast<BinOP>(node)){
            return handleOperator(ON);
        } else if (auto EN = std::dynamic_pointer_cast<ExternNode>(node)){
            return handleExtern(EN);
        } else if (auto PN = std::dynamic_pointer_cast<PubNode>(node)){
            return handlePub(PN);
        } else if (auto CN = std::dynamic_pointer_cast<CallNode>(node)){
            return handleCall(CN);
        } else if (auto RN = std::dynamic_pointer_cast<RetNode>(node)){
            return handleRet(RN);
        } else if (auto IFN = std::dynamic_pointer_cast<IFNode>(node)){
            return handleIf(IFN);
        } else if (auto WN = std::dynamic_pointer_cast<WhileNode>(node)){
            return handleWhile(WN);
        } else if (auto PN = std::dynamic_pointer_cast<PointerNode>(node)){
            return handlePointer(PN);
        } else if (auto DN = std::dynamic_pointer_cast<DerefNode>(node)){
            return handleDereference(DN);
        } else if (auto MPN = std::dynamic_pointer_cast<ModifyPtrNode>(node)){
            return handlePointerStore(MPN);
        } else if (auto SIN = std::dynamic_pointer_cast<StructInstanceNode>(node)){
            return handleStructInstance(SIN);
        } else if (auto MAN = std::dynamic_pointer_cast<MemAccNode>(node)){
            return handleMemberAccess(MAN);
        } else if (auto TN = std::dynamic_pointer_cast<TupleNode>(node)){
            return handleTuple(TN);
        } else if (auto MFN = std::dynamic_pointer_cast<MappedFunctionNode>(node)){
            return handleMFN("",MFN);
        } else if (auto CN = std::dynamic_pointer_cast<CastNode>(node)){
            return handleCast(CN);
        } else if (auto TSLN = std::dynamic_pointer_cast<TypeSafeListNode>(node)){
            return handleTSL(TSLN);
        } else if (auto TSLIN = std::dynamic_pointer_cast<TSLInitNode>(node)){
            return handleTSLInit(TSLIN);
        } else if (auto IN = std::dynamic_pointer_cast<IndexNode>(node)){
            return handleIndex(IN);
        } else if (auto DN = std::dynamic_pointer_cast<DecoratorNode>(node)){
            return handleDecorator(DN);
        } else if (auto MN = std::dynamic_pointer_cast<ModNode>(node)){
            return handleExtend(MN);
        } else if (auto SON = std::dynamic_pointer_cast<SizeOfNode>(node)){
            return handleSZ(SON);
        } else if (auto SSN = std::dynamic_pointer_cast<StackSizeNode>(node)){
            return handleStackSize(SSN);
        } else if (auto TIDN = std::dynamic_pointer_cast<TypeIDNode>(node)){
            return handleTY(TIDN);
        } else if (auto ASMN = std::dynamic_pointer_cast<ASMNode>(node)){
            return handleASM(ASMN);
        } else if (auto PTIN = std::dynamic_pointer_cast<PtrtointNode>(node)){
            return handleP2I(PTIN);
        } else if (auto ChN = std::dynamic_pointer_cast<ChainNode>(node)){
            return handleChain(ChN);
        } else if (auto EAN = std::dynamic_pointer_cast<ExprAssignNode>(node)){
            return handleExprAssign(EAN);
        } else if (auto RN = std::dynamic_pointer_cast<RefNode>(node)){
            return handleRef(RN);
        } else if (auto CN = std::dynamic_pointer_cast<ConventionNode>(node)){
            return handleConv(CN);
        } else if (auto DRN = std::dynamic_pointer_cast<DoubleRefNode>(node)){
            return instruct(*DRN->getRef());
        } else {
            if (typeid(node) != typeid(std::make_shared<ASTNode>())){
            std::cout << "Unknown Node" << std::endl;
            }
            return nullptr;
        }
    }

    void finish(){
        std::unordered_map<std::string,bool> options = {};
        std::vector <std::shared_ptr<Instruction>> args;

        auto mainblock = handleBlock(rootblock,true);
        mainblock->insts.push_back(std::make_shared<return_instruction>(std::make_shared<resource_instruction>(std::string("i32"),0)));
        std::string mainfnname = "main";
        if (AP.has("-bundle") || result == "package"){
            mainfnname = exports["t4hash"] + "main";
        }
        mainfn = std::make_shared<function_instruction>(mainfnname,std::make_shared<resource_instruction>(std::string("i32"),0),args,mainblock,true,false,false);

        Instructor instr(AP.values["compile"][0]);
        std::string target = "native";
        if (AP.has("target")){
            target = AP.values["target"][0];
        }
        std::string reloc = "PIC";
        if (AP.has("reloc")){
            target = AP.values["reloc"][0];
        }
        std::string cpu = "generic";
        if (AP.has("target-cpu")){
            cpu = AP.values["target-cpu"][0];
        }
        std::string outas;
        std::string output = replace(AP.values["compile"][0],".ta","");
        if (AP.has("output")){
            outas = AP.values["output"][0];
        } else {
            outas = output;
        }
        if (AP.has("emit")){
            for (auto& out : AP.values["emit"]){
                if (out == "llvm-ir"){
                    options["emit_llvm-ir"] = true;
                }
            }
        }

        std::string llvm_target = instr.inittarget(target,reloc,cpu);
        instr.executeInstruction(mainfn);
        int optlevel = 2;
        if (AP.has("-O0")){
            optlevel = 0;
        } else if (AP.has("-O1")){
            optlevel = 1;
        } else if (AP.has("-O2")){
            optlevel = 2;
        } else if (AP.has("-O3")){
            optlevel = 3;
        } else if (AP.has("-Os")){
            optlevel = -1;
        }


        instr.buildAndOutput(output,options,optlevel);
        std::string cc = "clang";
        if (AP.has("cc")){
            cc = AP.values["cc"][0];
        }
        
        std::string structure_clang = cc + " " + output + ".o -o " + outas + " " + " -target " + llvm_target + " " ;

        if (AP.values.find("link") != AP.values.end()){
            for (const auto& k: AP.values["link"]){
                auto p = k.find_last_of("/");
                auto path = k.substr(0,p);
                path = replace(path,"\"","");
                auto lib = k.substr(p+1);
                lib = replace(lib,".lib","");
                lib = replace(lib,"\"","");
                structure_clang += " -L\"" + path + "\" -l" + lib;

            }
        }
        if (AP.values.find("linkpath") != AP.values.end()){
            for (const auto& k: AP.values["linkpath"]){
                structure_clang += " -L" + k;
            }
        }
        if (AP.values.find("toolchain") != AP.values.end()){
            
            structure_clang += " --sysroot=\"" + AP.values["toolchain"][0] + "\" ";
            
        }
        if (AP.values.find("ccflags") != AP.values.end()){
            for (const auto& k: AP.values["ccflags"]){
                structure_clang += " " + k;
            }
        }
        if (AP.values.find("linklib") != AP.values.end()){
            for (const auto& k: AP.values["linklib"]){
                auto l = replace(k,".lib","");
                l = replace(k,"lib","");
                structure_clang += " -l" + l;
            }
        }
        if (!AP.has("-nocc") && !AP.has("-bundle") && result != "package"){

        //std::vector<std::string> objectpaths;
        auto linkdir = std::filesystem::absolute(outas + ".imports");
        std::filesystem::create_directory(linkdir);
        for (auto& o : objects){
            logat("Objecting " + linkdir.string() + "/" + o.first + ".o" ,"Ins.f");
            std::ofstream ofile(linkdir.string() + "/" + o.first + ".o", std::ios::binary);
            structure_clang += std::string(" " + linkdir.string() + "/" + o.first + ".o");
            ofile.write(o.second.data(),o.second.size());
            ofile.close();
        }
        if (AP.has("-showcc")){
            println(structure_clang);
        }
        

        system(structure_clang.c_str());
        std::filesystem::remove_all(linkdir);
        }
        if (AP.has("-bundle") || result == "package"){
            auto bundler = ObjectBundler();
            bundler.bundle(std::filesystem::absolute(output + ".o"),exports,std::filesystem::absolute(outas + ".t4"));
        }

        if (AP.has("emit")){
            if (in("object",AP.values["emit"])){
                
                
            } else {
                std::filesystem::remove(std::filesystem::absolute(output + ".o"));
            }
        } else {
            std::filesystem::remove(std::filesystem::absolute(output + ".o"));
        }
  
        println(green_block + fcircle + reset + " Finished build" + (warns == 0 ? "." : (" with " + std::to_string(warns) + " warnings.")));
        exit(0);
    }
    

    /*std::string add(std::shared_ptr<ASTNode> node){
        if (auto in = std::dynamic_pointer_cast<IntLiteralNode>(node)){
            return handleILN(in);
        } else if (auto Object = std::dynamic_pointer_cast<ONode>(node)){
            return handleObject(Object);
        } else if (auto StrLit = std::dynamic_pointer_cast<StringLiteralNode>(node)){
            return handleSLN(StrLit);
        } else if (auto AssignN = std::dynamic_pointer_cast<AssignNode>(node)){
            return handleAssign(AssignN);
        } else if (auto BN = std::dynamic_pointer_cast<BlockNode>(node)){
            return handleBlock(BN);
        } else if (auto EN = std::dynamic_pointer_cast<ExternNode>(node)){
            return handleExtern(EN);
        } else if (auto IN = std::dynamic_pointer_cast<IdentifierNode>(node)){
            return handleIdentifier(IN);
        } else if (auto PN = std::dynamic_pointer_cast<PointerNode>(node)){
            return handlePTR(PN);
        } else if (auto DN = std::dynamic_pointer_cast<DerefNode>(node)){
            return handlederef(DN);
        } else if (auto MPN = std::dynamic_pointer_cast<ModifyPtrNode>(node)){
            return handlePTRassign(MPN);
        } else if (auto OpNode = std::dynamic_pointer_cast<BinOP>(node)){
            return handleOps(OpNode);
        } else if (auto RN = std::dynamic_pointer_cast<RetNode>(node)){
            return handleRet(RN);
        } else if (auto PN = std::dynamic_pointer_cast<PubNode>(node)){
            return handlePub(PN);
        } else if (auto MAN = std::dynamic_pointer_cast<MemAccNode>(node)){
            return handleMember(MAN);
        } else if (auto SIN = std::dynamic_pointer_cast<StructInstanceNode>(node)){
            return handleStructI("",SIN);
        } else if (auto CN = std::dynamic_pointer_cast<CallNode>(node)){ // add type checking for CEXTFunction args

            return handleCall(CN);
        } else if (auto CN = std::dynamic_pointer_cast<CastNode>(node)){
            return handleCast(CN);
        } else {
            println("Unknown node");
            return "";
        }
    }*/





    void setscope(std::shared_ptr<Scope> it){
        this->CScope = it;

    }
    void setinte(std::shared_ptr<Interpreter> i){
        this->inte = i;
    }
    private:
    std::shared_ptr<Interpreter> inte;
    std::string  lastid;
    std::vector<std::string> fnargs;
    int bc = 0;
    std::shared_ptr<Scope> CScope = {};
    std::string line = "";
    std::string compiled = "";
};

Instruct It = Instruct(nullptr);

void setscope(std::shared_ptr<Scope> scope){
    It.setscope(scope);
}
#endif