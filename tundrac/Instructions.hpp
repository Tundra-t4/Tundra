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
#include <filesystem>
class Instruct {
    public:
    std::shared_ptr<function_instruction> mainfn;
    std::shared_ptr<BlockNode> rootblock;
    bool isassignflag=false;
    Instruct(std::shared_ptr<BlockNode> rootblock): rootblock(rootblock){

    }

    std::shared_ptr<Type> resolveType(const std::shared_ptr<ASTNode>& node){
        if (!node){
            return TYR.get_void();
        }
        if (node->typeinfo){
            return node->typeinfo;
        }
        if (auto id = std::dynamic_pointer_cast<IdentifierNode>(node)){
            auto ty = TYR.get_type(id->getValue());
            if (ty){
                return ty;
            }
        }
        if (auto pn = std::dynamic_pointer_cast<PointerNode>(node)){
            return std::make_shared<PointerType>(resolveType(pn->getPointee()),pn->isBorrowed());
        }
        if (auto tn = std::dynamic_pointer_cast<TupleNode>(node)){
            std::vector<std::shared_ptr<Type>> elems;
            for (auto& v : tn->getValue()){
                elems.push_back(resolveType(v.second));
            }
            return std::make_shared<TupleType>(elems);
        }
        if (auto tsl = std::dynamic_pointer_cast<TypeSafeListNode>(node)){
            auto elem = resolveType(tsl->getType());
            return std::make_shared<ArrayType>(elem,tsl->getValue().size());
        }
        if (auto tsli = std::dynamic_pointer_cast<TSLInitNode>(node)){
            auto elem = resolveType(tsli->getExpr());
            if (auto iln = std::dynamic_pointer_cast<IntLiteralNode>(tsli->getBody())){
                return std::make_shared<ArrayType>(elem,static_cast<size_t>(iln->getValue()));
            }
            return std::make_shared<ArrayType>(elem);
        }
        if (auto mfn = std::dynamic_pointer_cast<MappedFunctionNode>(node)){
            auto fnnargs = mfn->getParameters();
            auto ftty = SCAST<FunctionType>(mfn->typeinfo);
            std::vector<std::shared_ptr<Type>> params;
            for (auto& order : mfn->getParamOrder()){
                auto it = fnnargs.find(order.second);
                if (it != fnnargs.end()){
                    params.push_back(resolveType(it->second));
                }
            }
            auto retty = ftty->return_type();
            return std::make_shared<FunctionType>(params,retty,false);
        }
        return TYR.get_void();
    }

    std::shared_ptr<type_instruction> typeInstFromType(const std::shared_ptr<Type>& type){
        return std::make_shared<type_instruction>(type);
    }

    std::shared_ptr<type_instruction> typeInst(const std::shared_ptr<ASTNode>& node){
        return typeInstFromType(resolveType(node));
    }

    std::shared_ptr<Type> unwrapFunctionType(const std::shared_ptr<Type>& type){
        if (!type){
            return TYR.get_void();
        }
        if (type->is_function()){
            return type;
        }
        if (type->is_pointer()){
            auto ptr = std::dynamic_pointer_cast<PointerType>(type);
            if (ptr && ptr->pointee()->is_function()){
                return ptr->pointee();
            }
        }
        return type;
    }

    std::string memberName(const std::shared_ptr<ASTNode>& node){
        if (auto id = std::dynamic_pointer_cast<IdentifierNode>(node)){
            return id->getValue();
        }
        if (auto sln = std::dynamic_pointer_cast<StringLiteralNode>(node)){
            return sln->getValue();
        }
        return "";
    }

    bool parseMemberIndex(const std::string& name,int& out){
        if (name.empty()){
            return false;
        }
        for (auto c : name){
            if (!std::isdigit(static_cast<unsigned char>(c))){
                return false;
            }
        }
        try {
            out = std::stoi(name);
            return true;
        } catch (...) {
            return false;
        }
    }

    std::shared_ptr<Instruction> handleILN(std::shared_ptr<IntLiteralNode> in){
        auto ty = in->typeinfo ? in->typeinfo : TYR.get_i32();
        return std::make_shared<resource_instruction>(ty,in->getValue());
    }
    std::shared_ptr<Instruction> handleObject(std::shared_ptr<ONode> obj){
        std::shared_ptr<Type> ty = obj->typeinfo;
        if (!ty){
            ty = TYR.get_type(obj->getValue().getType());
        }
        return std::make_shared<resource_instruction>(ty,obj->getValue().GetStore());
    }

    std::shared_ptr<Instruction> handleSLN(std::shared_ptr<StringLiteralNode> sln){
        auto ty = sln->typeinfo ? sln->typeinfo : TYR.get_string();
        return std::make_shared<resource_instruction>(ty,sln->getValue());
    }

    std::shared_ptr<block_instruction> handleBlock(std::shared_ptr<BlockNode> bn,bool ismain=false){
        std::vector<std::shared_ptr<Instruction>> insts;

        for (auto& stat : bn->getStatements()){

            auto statement = instruct(stat);
            if (statement){
                insts.push_back(statement);
            } else {
                logat("Invalid statement","handleBlock");
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

        linst = std::make_shared<load_instruction>(IN->getValue(),isassignflag);
        
        
        logat("Received " + linst->getName() ,"Ins.hI");
        return linst;
    }
    std::shared_ptr<Instruction> handleOperator(std::shared_ptr<BinOP> ON){
        return std::make_shared<operation_instruction>(instruct(ON->getLeft()),instruct(ON->getRight()),ON->getValue());
    }

    std::shared_ptr<Instruction> handleExtern(std::shared_ptr<ExternNode> EN){
        auto vecnode = EN->getValue();
        auto fnname = EN->getfnnames();
        auto isvdic = EN->getvdic();
        std::vector<std::shared_ptr<Instruction>> fninsts;
        for (int i = 0; i != vecnode.size();i++){
            auto fn = std::dynamic_pointer_cast<MappedFunctionNode>(vecnode[i]);
            std::vector<std::shared_ptr<Instruction>> args;
            auto fnnargs = fn->getParameters();
            std::shared_ptr<Instruction> returnty = typeInst(fn->ReturnType);
            if (fn->getParamOrder().size() != 0){
                for (auto& argn : fn->getParamOrder()){
                    auto it = fnnargs.find(argn.second);
                    if (it != fnnargs.end()){
                        args.push_back(typeInst(it->second));
                    }
                }
            } else {
                for (auto & arg : fnnargs){
                    args.push_back(typeInst(arg.second));
                }
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
        if (!MFN->typeinfo){
            throw std::runtime_error("Invalid type info");
        }
        auto ftty = SCAST<FunctionType>(MFN->typeinfo);
        if (!ftty){
            throw std::runtime_error("Invalid type info");
        }
 
        std::vector<std::shared_ptr<Instruction>> args;
        auto fnnargs = MFN->getParameters();
        auto intes = MFN->getinternals();
        auto rettyType = ftty->return_type();
        bool isnoneret = rettyType ? rettyType->is_void() : false;
        std::shared_ptr<Instruction> returnty = typeInstFromType(rettyType);
        logat("Return ty is : " + std::to_string(static_cast<int>(rettyType->kind())),"inst.hMFN");
        if (ftty && ftty->param_types().size() == MFN->getParamOrder().size()){
            int idx = 0;
            for (auto& argn : MFN->getParamOrder()){
                args.push_back(typeInstFromType(ftty->param_types()[idx]));
                idx++;
            }
        } else if (MFN->getParamOrder().size() != 0){
            for (auto& argn : MFN->getParamOrder()){
                auto it = fnnargs.find(argn.second);
                if (it != fnnargs.end()){
                    args.push_back(typeInst(it->second));
                }
            }
        } else {
            for (auto & arg : fnnargs){
                args.push_back(typeInst(arg.second));
            }
        }
        std::vector<std::string> argnames;
        for (auto& argn : MFN->getParamOrder()){
            if (intes.find(argn.second) != intes.end()){
                argnames.push_back(intes[argn.second]);
            } else {
            
                argnames.push_back(argn.second);
            }
        }
        std::shared_ptr<BlockNode> blk = std::dynamic_pointer_cast<BlockNode>(MFN->getBody());
        
        return std::make_shared<function_instruction>(name,returnty,args,std::dynamic_pointer_cast<block_instruction>(instruct(blk)),false,false,false,argnames,"",isnoneret);
    }
    std::shared_ptr<Instruction> handleConv(std::shared_ptr<ConventionNode> CN){
        auto pm = mangle_rule;
        mangle_rule = CN->getValue();
        std::vector<std::shared_ptr<Instruction>> insts;
        for (auto& inst : CN->getPub()){
            if (inst){
                insts.push_back(instruct(inst));
            }
        }
        mangle_rule = pm;
        return std::make_shared<chain_instruction>(insts);
    }
    std::shared_ptr<Instruction> handlePub(std::shared_ptr<PubNode> PN){
        if (auto aas = std::dynamic_pointer_cast<AssignNode>(PN->getValue())){
            if (auto MFN = std::dynamic_pointer_cast<MappedFunctionNode>(aas->getValue())){
                auto name = aas->getVarName();
                std::vector<std::shared_ptr<Instruction>> args;
                auto fnnargs = MFN->getParameters();
                auto internals = MFN->getinternals();
                auto ftty = SCAST<FunctionType>(MFN->typeinfo);
                auto rettyType = ftty->return_type();
                std::shared_ptr<Instruction> returnty = typeInstFromType(rettyType);
                if (ftty && ftty->param_types().size() == MFN->getParamOrder().size()){
                    int idx = 0;
                    for (auto& argn : MFN->getParamOrder()){
                        args.push_back(typeInstFromType(ftty->param_types()[idx]));
                        idx++;
                    }
                } else if (MFN->getParamOrder().size() != 0){
                    for (auto& argn : MFN->getParamOrder()){
                        auto it = fnnargs.find(argn.second);
                        if (it != fnnargs.end()){
                            args.push_back(typeInst(it->second));
                        }
                    }
                } else {
                    for (auto & arg : fnnargs){
                        args.push_back(typeInst(arg.second));
                    }
                }
                std::vector<std::string> argnames;
                for (auto& argn : MFN->getParamOrder()){
                    if (internals.find(argn.second) == internals.end()){
                        argnames.push_back(argn.second);
                    } else {
                        argnames.push_back(internals[argn.second]);
                    }
                }
                bool isnoneret = rettyType ? rettyType->is_void() : false;
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
        std::vector<std::shared_ptr<Instruction>> args;
        auto CNargs = CN->getArgs();
        for (auto& arg : CNargs){
            args.push_back(instruct(arg.second));
        }
        auto inserts = CN->getinserts();
        for (auto& insert : inserts){
            args.push_back(instruct(insert.second));
        }
        auto fty = typeInstFromType(unwrapFunctionType(resolveType(CN->getCallee())));
        return std::make_shared<call_instruction>(instruct(CN->getCallee()),args,fty);
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
        if (!RN->getValue()){
            return std::make_shared<return_instruction>(typeInstFromType(TYR.get_void()));
        }
        return std::make_shared<return_instruction>(instruct(RN->getValue()));
    }


    std::shared_ptr<Instruction> handleIf(std::shared_ptr<IFNode> IFN,bool isassign = false){
        auto elifs = IFN->getElifBranch();
        std::vector<std::shared_ptr<if_instruction>> insts = {};
        for (auto& elif : elifs){
            insts.push_back(std::make_shared<if_instruction>(instruct(elif.second->get_condition()),std::dynamic_pointer_cast<block_instruction>(instruct(elif.second->getBody()))));
        }
        if (IFN->getElseBranch()){
        return std::make_shared<if_instruction>(instruct(IFN->getMainBranch()->get_condition()),std::dynamic_pointer_cast<block_instruction>(instruct(IFN->getMainBranch()->getBody())),std::dynamic_pointer_cast<block_instruction>(instruct(IFN->getElseBranch()->getBody())),insts,isassign || inT<int>(IFN->id,isused));
        } else {
            return std::make_shared<if_instruction>(instruct(IFN->getMainBranch()->get_condition()),std::dynamic_pointer_cast<block_instruction>(instruct(IFN->getMainBranch()->getBody())),nullptr,insts,isassign || inT<int>(IFN->id,isused));
        }
    }
     std::shared_ptr<Instruction> handleWhile(std::shared_ptr<WhileNode> WN,bool isassign=false){
        return std::make_shared<while_instruction>(instruct(WN->getLoopExpr()->get_condition()),std::dynamic_pointer_cast<block_instruction>(instruct(WN->getLoopExpr()->getBody())),isassign || inT<int>(WN->id,isused));
     }


    std::shared_ptr<Instruction> handlePointer(std::shared_ptr<PointerNode> PN){
        if (!PN->isBorrowed()){
            return std::make_shared<pointer_instruction>(instruct(PN->getPointee()),false);
        } else {
            return std::make_shared<borrowed_pointer_instruction>(instruct(PN->getPointee()));
        }
    }
    std::shared_ptr<Instruction> handleDereference(std::shared_ptr<DerefNode> DN){
        auto derefType = resolveType(DN);
        if (derefType && derefType->is_pointer()){
            auto ptr = std::dynamic_pointer_cast<PointerType>(derefType);
            if (ptr){
                derefType = ptr->pointee();
            }
        }
        return std::make_shared<dereference_instruction>(instruct(DN->getPointer()),typeInstFromType(derefType));
    }

    std::shared_ptr<Instruction> handlePointerStore(std::shared_ptr<ModifyPtrNode> MPN){
        return std::make_shared<pointer_store_instruction>(instruct(MPN->getLHS()),instruct(MPN->getExpr()));
    }
    std::shared_ptr<Instruction> handleStructDecl(std::string name,std::shared_ptr<StructDeclNode> SDN){
        std::vector<std::shared_ptr<Instruction>> fields;
        std::vector<StructType::Field> typefields;
        for (auto& field : SDN->getFields()){
            auto ftype = resolveType(field.second);
            typefields.push_back({field.first,ftype});
            fields.push_back(typeInstFromType(ftype));
        }
        if (!TYR.has_type(name)){
            TYR.register_type(name,std::make_shared<StructType>(name,typefields));
        }
        return std::make_shared<struct_decl_instruction>(name,fields);
    }

    std::shared_ptr<Instruction> handleStructInstance(std::shared_ptr<StructInstanceNode> SIN){
        std::vector<std::shared_ptr<Instruction>> fields;
        std::vector<std::string> sfields;
        for (auto& field : SIN->getFields()){
            fields.push_back(instruct(field.second));
            sfields.push_back(field.first);
        }
        return std::make_shared<struct_instance_instruction>(typeInst(SIN->getBase()),fields,sfields,false,SIN->id);
    }
    std::shared_ptr<Instruction> handleExprAssign(std::shared_ptr<ExprAssignNode> EAN){
        return std::make_shared<alt_assign_instruction>(instruct(EAN->getAssignee()),instruct(EAN->getValue()));
    }
    std::shared_ptr<Instruction> handleMemberAccess(std::shared_ptr<MemAccNode> MAN,bool privassign=false){
        if (MAN->isAssignment()){
            MAN->setAssign(false); // we're done with wrapping it
            return std::make_shared<alt_assign_instruction>(handleMemberAccess(MAN,true),instruct(MAN->getAssignValue()));
        }
        auto baseInst = std::dynamic_pointer_cast<MemAccNode>(MAN->getValue())
            ? handleMemberAccess(std::dynamic_pointer_cast<MemAccNode>(MAN->getValue()))
            : instruct(MAN->getValue());
        auto baseType = resolveType(MAN->getValue());
        auto name = memberName(MAN->getNxt());
        if (baseType && baseType->is_struct()){
            auto structTy = std::dynamic_pointer_cast<StructType>(baseType);
            int fieldIndex = 0;
            std::shared_ptr<Type> fieldType = TYR.get_void();
            if (structTy){
                int idx = 0;
                for (const auto& field : structTy->fields()){
                    if (field.name == name){
                        fieldIndex = idx;
                        fieldType = field.type;
                        break;
                    }
                    idx++;
                }
            }
            std::vector<std::shared_ptr<type_instruction>> vec{typeInstFromType(baseType),typeInstFromType(fieldType)};
            return std::make_shared<access_struct_member_instruction>(baseInst,name,fieldIndex,vec,privassign);
        }
        if (baseType && baseType->kind() == TypeKind::TUPLE){
            auto tupleTy = std::dynamic_pointer_cast<TupleType>(baseType);
            int idx = 0;
            parseMemberIndex(name,idx);
            std::shared_ptr<Type> elemType = TYR.get_void();
            if (tupleTy && idx >= 0 && idx < static_cast<int>(tupleTy->elements().size())){
                elemType = tupleTy->elements()[idx];
            }
            std::vector<std::shared_ptr<type_instruction>> vec{typeInstFromType(baseType),typeInstFromType(elemType)};
            return std::make_shared<access_tuple_member_instruction>(baseInst,idx,vec);
        }
        return std::make_shared<nop_instruction>();
    }

    std::shared_ptr<Instruction> handleTuple(std::shared_ptr<TupleNode> TN){
        std::vector<std::shared_ptr<Instruction>> elements;
        for (auto& v : TN->getValue()){
            elements.push_back(instruct(v.second));
        }
        return std::make_shared<tuple_instruction>(elements);
    }

    std::shared_ptr<Instruction> handleCast(std::shared_ptr<CastNode> CN){
        auto tinst = typeInst(CN->getDest());
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
        int32_t sz = 0;
        auto arrType = std::dynamic_pointer_cast<ArrayType>(resolveType(TSLIN));
        if (arrType && arrType->size().has_value()){
            sz = static_cast<int32_t>(arrType->size().value());
        } else if (auto iln = std::dynamic_pointer_cast<IntLiteralNode>(TSLIN->getBody())){
            sz = iln->getValue();
        }

        return std::make_shared<init_array_instruction>(instruct(TSLIN->getExpr()),sz);
    }
    //std::vector<std::shared_ptr<type_instruction>>
    std::shared_ptr<Instruction> handleIndex(std::shared_ptr<IndexNode> IN){
        auto baseType = resolveType(IN->getValue());
        if (baseType && baseType->is_array()){
            auto arr = std::dynamic_pointer_cast<ArrayType>(baseType);
            auto elem = arr ? arr->element_type() : TYR.get_void();
            std::vector<std::shared_ptr<type_instruction>> vec{typeInstFromType(baseType),typeInstFromType(elem)};
            return std::make_shared<access_array_instruction>(instruct(IN->getValue()),vec,instruct(IN->getIndex()));
        } else if (baseType && baseType->is_pointer()){
            auto ptr = std::dynamic_pointer_cast<PointerType>(baseType);
            auto elem = ptr ? ptr->pointee() : TYR.get_void();
            std::vector<std::shared_ptr<type_instruction>> vec{typeInstFromType(elem),typeInstFromType(elem)};
            return std::make_shared<access_pointer_instruction>(instruct(IN->getValue()),vec,instruct(IN->getIndex()));
        }
        return std::make_shared<nop_instruction>();
    }
    std::shared_ptr<Instruction> handleExtend(std::shared_ptr<ModNode> MN){
        std::vector<std::shared_ptr<Instruction>> exts;
        for (auto& ext: MN->getStates()){
            exts.push_back(instruct(ext));
        }
        return std::make_shared<extend_instruction>(instruct(MN->getValue()),exts);
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
        auto ttype = resolveType(TIDN->getValue());
        std::shared_ptr<type_instruction> ty = ttype ? typeInstFromType(ttype) : nullptr;
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
        return std::make_shared<asm_instruction>(std::make_shared<resource_instruction>(TYR.get_string(),ASMN->ASMStr),ASMN->regs,asmUTIL(ASMN->in),asmUTIL(ASMN->out),asmUTIL(ASMN->inout));
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
        Module mod(ImN->getName());
        std::vector<std::shared_ptr<Instruction>> insts;
        auto from = ImN->getFrom();
        for (auto& method : mod.methods){
            //auto mfn = std::any_cast<MappedFunction>(method.second);
            auto vec = mod.MNodes[method.first];
            std::vector<std::shared_ptr<Instruction>> args;
            
            std::shared_ptr<Instruction> retty = typeInst(vec[vec.size()-1]);
            vec.pop_back();

            for (auto& arg: vec){
                args.push_back(typeInst(arg));
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
        
        auto inst = _instruct(node);
        if (inst){
            inst->typeinfo = node->typeinfo;
        } else {
            logat("Typeinfo: " + std::to_string(static_cast<int>(node->typeinfo->kind())),"instruct:badinst");
            logat("Node: " + std::to_string(node->get_node_type_id()),"instruct:badinst");
            //*(int*)0 = 1;
            /*throw std::runtime_error("Bad Instruction!");*/
            return nullptr;
        }
        return inst;
    }
    std::shared_ptr<Instruction> _instruct(std::shared_ptr<ASTNode> node){
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
        } else if (auto EN = std::dynamic_pointer_cast<ErrorNode>(node)){
            return std::make_shared<nop_instruction>();
        } 
        else {
            if (typeid(node) != typeid(std::make_shared<ASTNode>())){
            std::cout << "Unknown Node" << std::endl;
            }
            logat("Unknown Node","instruct");
            return nullptr;
        }
    }
    TypeRegistry& TYR = TypeRegistry::instance();
    void finish(){
        std::unordered_map<std::string,bool> options = {};
        std::vector <std::shared_ptr<Instruction>> args;

        auto mainblock = handleBlock(rootblock,true);
        mainblock->insts.push_back(std::make_shared<return_instruction>(std::make_shared<resource_instruction>(TYR.get_i32(),0)));
        std::string mainfnname = "main";
        if (AP.has("-bundle")){
            mainfnname = exports["t4hash"] + "main";
        }
        mainfn = std::make_shared<function_instruction>(mainfnname,std::make_shared<type_instruction>(TYR.get_i32()),args,mainblock,true,false,false);

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
        if (!AP.has("-nocc") && !AP.has("-bundle")){

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
        if (AP.has("-bundle")){
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
  
        println(Colors::GREEN + Symbols::FILLED_CIRCLE + Colors::RESET + " Finished build" + (warns == 0 ? "." : (" with " + std::to_string(warns) + " warnings.")));
        exit(0);
    }
    







    void setscope(std::shared_ptr<Scope> it){
        this->CScope = it;

    }
    /*void setinte(std::shared_ptr<Interpreter> i){
        this->inte = i;
    }*/
    private:
    //std::shared_ptr<Interpreter> inte;
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
