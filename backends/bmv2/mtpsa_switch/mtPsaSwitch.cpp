#include "frontends/common/model.h"
#include "mtPsaSwitch.h"

namespace BMV2 {

void MtPsaProgramStructure::create(ConversionContext* ctxt) {
    createTypes(ctxt);
    createHeaders(ctxt);
    createScalars(ctxt);
    createExterns();
    createParsers(ctxt);
    createActions(ctxt);
    createControls(ctxt);
    createDeparsers(ctxt);
    createGlobals();
}

void MtPsaProgramStructure::createStructLike(ConversionContext* ctxt, const IR::Type_StructLike* st) {
    CHECK_NULL(st);
    cstring name = st->controlPlaneName();
    unsigned max_length = 0;  // for variable-sized headers
    bool varbitFound = false;
    auto fields = new Util::JsonArray();
    LOG5("In createStructLike with struct " << st->toString());
    for (auto f : st->fields) {
        auto field = new Util::JsonArray();
        auto ftype = typeMap->getType(f, true);
        LOG5("Iterating field with field " << f << " and type " << ftype->toString());
        if (ftype->to<IR::Type_StructLike>()) {
            BUG("%1%: nested structure", st);
        } else if (ftype->is<IR::Type_Boolean>()) {
            field->append(f->name.name);
            field->append(1);
            field->append(false);
            max_length += 1;
        } else if (auto type = ftype->to<IR::Type_Bits>()) {
            field->append(f->name.name);
            field->append(type->size);
            field->append(type->isSigned);
            max_length += type->size;
        } else if (auto type = ftype->to<IR::Type_Varbits>()) {
            field->append(f->name.name);
            max_length += type->size;
            field->append("*");
            if (varbitFound)
                ::error(ErrorType::ERR_UNSUPPORTED,
                        "%1%: headers with multiple varbit fields are not supported", st);
            varbitFound = true;
        } else if (ftype->is<IR::Type_Error>()) {
            field->append(f->name.name);
            field->append(error_width);
            field->append(false);
            max_length += error_width;
        } else if (ftype->to<IR::Type_Stack>()) {
            BUG("%1%: nested stack", st);
        } else {
            BUG("%1%: unexpected type for %2%.%3%", ftype, st, f->name);
        }
        fields->append(field);
    }
    // must add padding
    unsigned padding = max_length % 8;
    if (padding != 0) {
        cstring name = refMap->newName("_padding");
        auto field = new Util::JsonArray();
        field->append(name);
        field->append(8 - padding);
        field->append(false);
        fields->append(field);
    }

    unsigned max_length_bytes = (max_length + padding) / 8;
    if (!varbitFound) {
        // ignore
        max_length = 0;
        max_length_bytes = 0;
    }
    ctxt->json->add_header_type(name, fields, max_length_bytes);
}

void MtPsaProgramStructure::createTypes(ConversionContext* ctxt) {
    for (auto kv : header_types)
        createStructLike(ctxt, kv.second);
    for (auto kv : metadata_types)
        createStructLike(ctxt, kv.second);
    for (auto kv : header_union_types) {
        auto st = kv.second;
        auto fields = new Util::JsonArray();
        for (auto f : st->fields) {
            auto field = new Util::JsonArray();
            auto ftype = typeMap->getType(f, true);
            auto ht = ftype->to<IR::Type_Header>();
            CHECK_NULL(ht);
            field->append(f->name.name);
            field->append(ht->name.name);
        }
        ctxt->json->add_union_type(st->name, fields);
    }
    /* TODO */
    // add errors to json
    // add enums to json
}

void MtPsaProgramStructure::createScalars(ConversionContext *ctxt) {
    auto name = scalars.begin()->first;
    ctxt->json->add_header("scalars_t", name);
    ctxt->json->add_header_type("scalars_t");

    unsigned max_length = 0;
    for (auto kv : scalars) {
        LOG5("Adding a scalar field " << kv.second << " to generated json");
        auto field = new Util::JsonArray();
        auto ftype = typeMap->getType(kv.second, true);
        if (auto type = ftype->to<IR::Type_Bits>()) {
            field->append(kv.second->name);
            field->append(type->size);
            field->append(type->isSigned);
        } else if (ftype->to<IR::Type_Boolean>()) {
            field->append(kv.second->name);
            max_length += 1;
            field->append(1);
            field->append(false);
        }
        else
        {
            BUG_CHECK(kv.second, "%1 is not of Type_Bits or Type_Boolean");
        }
        ctxt->json->add_header_field("scalars_t", field);
    }
    // must add padding
    unsigned padding = max_length % 8;
    if (padding != 0) {
        auto field = new Util::JsonArray();
        field->append(refMap->newName("_padding"));
        field->append(8 - padding);
        field->append(false);
        ctxt->json->add_header_field("scalars_t", field);
    }
}

void MtPsaProgramStructure::createHeaders(ConversionContext* ctxt) {
    for (auto kv : headers) {
        auto type = kv.second->type->to<IR::Type_StructLike>();
        ctxt->json->add_header(type->controlPlaneName(), kv.second->name);
    }
    for (auto kv : metadata) {
        auto type = kv.second->type->to<IR::Type_StructLike>();
        ctxt->json->add_metadata(type->controlPlaneName(), kv.second->name);
    }
    /* TODO */
    // for (auto kv : header_stacks) {
    //     json->add_header_stack(stack_type, stack_name, stack_size, ids);
    // }
    for (auto kv : header_unions) {
        auto header_name = kv.first;
        auto header_type = kv.second->to<IR::Type_StructLike>()->controlPlaneName();
        // We have to add separately a header instance for all
        // headers in the union.  Each instance will be named with
        // a prefix including the union name, e.g., "u.h"
        Util::JsonArray* fields = new Util::JsonArray();
        for (auto uf : kv.second->to<IR::Type_HeaderUnion>()->fields) {
            auto uft = typeMap->getType(uf, true);
            auto h_name = header_name + "." + uf->controlPlaneName();
            auto h_type = uft->to<IR::Type_StructLike>()->controlPlaneName();
            unsigned id = ctxt->json->add_header(h_type, h_name);
            fields->append(id);
        }
        ctxt->json->add_union(header_type, fields, header_name);
    }
}

void MtPsaProgramStructure::createParsers(ConversionContext* ctxt) {
    if (userProgram) {
        auto cvt = new ParserConverter(ctxt, "parser");
        auto ingress = parsers.at("ingress");
        ingress->apply(*cvt);
        return;
    }
    {
        auto cvt = new ParserConverter(ctxt, "ingress_parser");
        auto ingress = parsers.at("ingress");
        ingress->apply(*cvt);
    }
    {
        auto cvt = new ParserConverter(ctxt, "egress_parser");
        auto egress = parsers.at("egress");
        egress->apply(*cvt);
    }
}

void MtPsaProgramStructure::createExterns() {
    /* TODO */
    // add parse_vsets to json
    // add meter_arrays to json
    // add counter_arrays to json
    // add register_arrays to json
    // add checksums to json
    // add learn_list to json
    // add calculations to json
    // add extern_instances to json
}

void MtPsaProgramStructure::createActions(ConversionContext* ctxt) {
    auto cvt = new ActionConverter(ctxt, true);
    for (auto it : actions) {
        auto action = it.first;
        action->apply(*cvt);
    }
}

void MtPsaProgramStructure::createControls(ConversionContext* ctxt) {
    auto cvt = new BMV2::ControlConverter<Standard::Arch::PSA>(ctxt, "ingress", true);
    auto ingress = pipelines.at("ingress");
    ingress->apply(*cvt);

    if (!userProgram) {
      cvt = new BMV2::ControlConverter<Standard::Arch::PSA>(ctxt, "egress", true);
      auto egress = pipelines.at("egress");
      egress->apply(*cvt);
    }
}

void MtPsaProgramStructure::createDeparsers(ConversionContext* ctxt) {
    if (userProgram) {
        auto cvt = new DeparserConverter(ctxt, "deparser");
        auto ingress = deparsers.at("ingress");
        ingress->apply(*cvt);
        return;
    }
    {
        auto cvt = new DeparserConverter(ctxt, "ingress_deparser");
        auto ingress = deparsers.at("ingress");
        ingress->apply(*cvt);
    }
    {
        auto cvt = new DeparserConverter(ctxt, "egress_deparser");
        auto egress = deparsers.at("egress");
        egress->apply(*cvt);
    }
}

void MtPsaProgramStructure::createGlobals() {
    /* TODO */
    // for (auto e : globals) {
    //     convertExternInstances(e->node->to<IR::Declaration>(), e->to<IR::ExternBlock>());
    // }
}

bool ParseMtPsaArchitecture::preorder(const IR::ToplevelBlock* block) {
    /// Blocks are not in IR tree, use a custom visitor to traverse
    for (auto it : block->constantValue) {
        if (it.second->is<IR::Block>())
            visit(it.second->getNode());
    }
    return false;
}

bool ParseMtPsaArchitecture::preorder(const IR::ExternBlock* block) {
    if (block->node->is<IR::Declaration>())
        structure->globals.push_back(block);
    return false;
}

bool ParseMtPsaArchitecture::preorder(const IR::PackageBlock* block) {
    if (structure->userProgram) {
        /* User Pipeline */

        auto tmp = block->findParameterValue("pr");
        if (tmp == nullptr) {
            modelError("user program %1% has no parameter named 'pr'", block);
            return false;
        }
        auto parser = tmp->to<IR::ParserBlock>();
        if (parser == nullptr) {
            modelError("%1%: 'pr' argument of user program should be bound to a parser", block);
            return false;
        }

        tmp = block->findParameterValue("pi");
        if (tmp == nullptr) {
            modelError("user program %1% has no parameter named 'pi'", block);
            return false;
        }
        auto pipeline = tmp->to<IR::ParserBlock>();
        if (pipeline == nullptr) {
            modelError("%1%: 'pi' argument of user program should be bound to a pipeline", block);
            return false;
        }

        tmp = block->findParameterValue("dp");
        if (tmp == nullptr) {
            modelError("user program %1% has no parameter named 'dp'", block);
            return false;
        }
        auto deparser = tmp->to<IR::ParserBlock>();
        if (deparser == nullptr) {
            modelError("%1%: 'dp' argument of user program should be bound to a control", block);
            return false;
        }

        structure->block_type.emplace(parser->container, std::make_pair(INGRESS, PARSER));
        structure->block_type.emplace(pipeline->container, std::make_pair(INGRESS, PIPELINE));
        structure->non_pipeline_controls.emplace(deparser->container->name);
        structure->block_type.emplace(deparser->container, std::make_pair(INGRESS, DEPARSER));
    } else {
        /* Superuser Pipeline */

        auto pkg = block->findParameterValue("ingress");
        if (pkg == nullptr) {
            modelError("Package %1% has no parameter named 'ingress'", block);
            return false;
        }

        if (auto ingress = pkg->to<IR::PackageBlock>()) {
            auto tmp = ingress->findParameterValue("ip");
            if (tmp == nullptr) {
                modelError("'ingress' package %1% has no parameter named 'ip'", block);
                return false;
            }
            auto parser = tmp->to<IR::ParserBlock>();
            if (parser == nullptr) {
                modelError("%1%: 'ip' argument of 'ingress' should be bound to a parser", block);
                return false;
            }

            tmp = ingress->findParameterValue("ig");
            if (tmp == nullptr) {
                modelError("'ingress' package %1% has no parameter named 'ig'", block);
                return false;
            }
            auto pipeline = tmp->to<IR::ControlBlock>();
            if (pipeline == nullptr) {
                modelError("%1%: 'ig' argument of 'ingress' should be bound to a control", block);
                return false;
            }

            tmp = ingress->findParameterValue("id");
            if (tmp == nullptr) {
                modelError("'ingress' package %1% has no parameter named 'id'", block);
                return false;
            }
            auto deparser = tmp->to<IR::ControlBlock>();
            if (deparser == nullptr) {
                modelError("'%1%: id' argument of 'ingress' should be bound to a control", block);
                return false;
            }

            structure->block_type.emplace(parser->container, std::make_pair(INGRESS, PARSER));
            structure->block_type.emplace(pipeline->container, std::make_pair(INGRESS, PIPELINE));
            structure->block_type.emplace(deparser->container, std::make_pair(INGRESS, DEPARSER));
            structure->pipeline_controls.emplace(pipeline->container->name);
            structure->non_pipeline_controls.emplace(deparser->container->name);
        } else {
            modelError("'ingress' %1% is not bound to a package", pkg);
            return false;
        }

        auto tmp = block->findParameterValue("ep");
        if (tmp == nullptr) {
            modelError("'egress' package %1% has no parameter named 'ep'", block);
            return false;
        }
        auto egress_parser = tmp->to<IR::ParserBlock>();
        if (egress_parser == nullptr) {
            modelError("%1%: 'ep' argument of 'egress' should be bound to a praser", block);
            return false;
        }

        tmp = block->findParameterValue("eg");
        if (tmp == nullptr) {
            modelError("'egress' package %1% has no parameter named 'eg'", block);
            return false;
        }
        auto pipeline = tmp->to<IR::ControlBlock>();
        if (pipeline == nullptr) {
            modelError("%1%: 'ep' argument of 'egress' should be bound to a control", block);
            return false;
        }

        tmp = block->findParameterValue("ed");
        if (tmp == nullptr) {
            modelError("'egress' package %1% has no parameter named 'ed'", block);
            return false;
        }
        auto egress_deparser = tmp->to<IR::ControlBlock>();
        if (egress_deparser == nullptr) {
            modelError("%1%: 'ed' argument of 'egress' should be bound to a control", block);
            return false;
        }

        structure->block_type.emplace(egress_parser->container, std::make_pair(EGRESS, PARSER));
        structure->block_type.emplace(pipeline->container, std::make_pair(EGRESS, PIPELINE));
        structure->block_type.emplace(egress_deparser->container, std::make_pair(EGRESS, DEPARSER));
        structure->pipeline_controls.emplace(pipeline->container->name);
        structure->non_pipeline_controls.emplace(egress_deparser->container->name);
    }
    return false;
}

void InspectMtPsaProgram::postorder(const IR::Declaration_Instance* di) {
    if (!pinfo->resourceMap.count(di))
        return;
    auto blk = pinfo->resourceMap.at(di);
    if (blk->is<IR::ExternBlock>()) {
        auto eb = blk->to<IR::ExternBlock>();
        LOG3("populate " << eb);
        pinfo->extern_instances.emplace(di->name, di);
    }
}

bool InspectMtPsaProgram::isHeaders(const IR::Type_StructLike* st) {
    bool result = false;
    for (auto f : st->fields) {
        if (f->type->is<IR::Type_Header>() || f->type->is<IR::Type_Stack>()) {
            result = true;
        }
    }
    return result;
}

void InspectMtPsaProgram::addHeaderType(const IR::Type_StructLike *st) {
    LOG5("In addHeaderType with struct " << st->toString());
    if (st->is<IR::Type_HeaderUnion>()) {
      LOG5("Struct is Type_HeaderUnion");
        for (auto f : st->fields) {
            auto ftype = typeMap->getType(f, true);
            auto ht = ftype->to<IR::Type_Header>();
            CHECK_NULL(ht);
            addHeaderType(ht);
        }
        pinfo->header_union_types.emplace(st->getName(), st->to<IR::Type_HeaderUnion>());
        return;
    } else if (st->is<IR::Type_Header>()) {
      LOG5("Struct is Type_Header");
      pinfo->header_types.emplace(st->getName(), st->to<IR::Type_Header>());
    } else if (st->is<IR::Type_Struct>()) {
      LOG5("Struct is Type_Struct");
        pinfo->metadata_types.emplace(st->getName(), st->to<IR::Type_Struct>());
    }
}

void InspectMtPsaProgram::addHeaderInstance(const IR::Type_StructLike *st, cstring name) {
    auto inst = new IR::Declaration_Variable(name, st);
    if (st->is<IR::Type_Header>())
        pinfo->headers.emplace(name, inst);
    else if (st->is<IR::Type_Struct>())
        pinfo->metadata.emplace(name, inst);
    else if (st->is<IR::Type_HeaderUnion>())
        pinfo->header_unions.emplace(name, inst);
}

void InspectMtPsaProgram::addTypesAndInstances(const IR::Type_StructLike* type, bool isHeader) {
    LOG5("Adding type " << type->toString() << " and isHeader " << isHeader);
    for (auto f : type->fields) {
        LOG5("Iterating through field " << f->toString());
        auto ft = typeMap->getType(f, true);
        if (ft->is<IR::Type_StructLike>()) {
            // The headers struct can not contain nested structures.
            if (isHeader && ft->is<IR::Type_Struct>()) {
                ::error(ErrorType::ERR_INVALID,
                        "Type %1% should only contain headers, header stacks, or header unions",
                        type);
                return;
            }
            auto st = ft->to<IR::Type_StructLike>();
            addHeaderType(st);
        }
    }

    for (auto f : type->fields) {
        auto ft = typeMap->getType(f, true);
        if (ft->is<IR::Type_StructLike>()) {
            if (auto hft = ft->to<IR::Type_Header>()) {
                LOG5("Field is Type_Header");
                addHeaderInstance(hft, f->controlPlaneName());
            } else if (ft->is<IR::Type_HeaderUnion>()) {
                LOG5("Field is Type_HeaderUnion");
                for (auto uf : ft->to<IR::Type_HeaderUnion>()->fields) {
                    auto uft = typeMap->getType(uf, true);
                    if (auto h_type = uft->to<IR::Type_Header>()) {
                        addHeaderInstance(h_type, uf->controlPlaneName());
                    } else {
                        ::error(ErrorType::ERR_INVALID,
                                "Type %1% cannot contain type %2%", ft, uft);
                        return;
                    }
                }
                pinfo->header_union_types.emplace(type->getName(),
                                                  type->to<IR::Type_HeaderUnion>());
                addHeaderInstance(type, f->controlPlaneName());
            } else {
                LOG5("Adding struct with type " << type);
                pinfo->metadata_types.emplace(type->getName(), type->to<IR::Type_Struct>());
                addHeaderInstance(type, f->controlPlaneName());
            }
        } else if (ft->is<IR::Type_Stack>()) {
            LOG5("Field is Type_Stack " << ft->toString());
            auto stack = ft->to<IR::Type_Stack>();
            // auto stack_name = f->controlPlaneName();
            auto stack_size = stack->getSize();
            auto type = typeMap->getTypeType(stack->elementType, true);
            BUG_CHECK(type->is<IR::Type_Header>(), "%1% not a header type", stack->elementType);
            auto ht = type->to<IR::Type_Header>();
            addHeaderType(ht);
            auto stack_type = stack->elementType->to<IR::Type_Header>();
            std::vector<unsigned> ids;
            for (unsigned i = 0; i < stack_size; i++) {
                cstring hdrName = f->controlPlaneName() + "[" + Util::toString(i) + "]";
                /* TODO */
                // auto id = json->add_header(stack_type, hdrName);
                addHeaderInstance(stack_type, hdrName);
                // ids.push_back(id);
            }
            // addHeaderStackInstance();
        } else {
            // Treat this field like a scalar local variable
            cstring newName = refMap->newName(type->getName() + "." + f->name);
            LOG5("newname for scalarMetadataFields is " << newName);
            if (ft->is<IR::Type_Bits>()) {
                LOG5("Field is a Type_Bits");
                auto tb = ft->to<IR::Type_Bits>();
                pinfo->scalars_width += tb->size;
                pinfo->scalarMetadataFields.emplace(f, newName);
            } else if (ft->is<IR::Type_Boolean>()) {
                LOG5("Field is a Type_Boolean");
                pinfo->scalars_width += 1;
                pinfo->scalarMetadataFields.emplace(f, newName);
            } else if (ft->is<IR::Type_Error>()) {
                LOG5("Field is a Type_Error");
                pinfo->scalars_width += 32;
                pinfo->scalarMetadataFields.emplace(f, newName);
            } else {
                BUG("%1%: Unhandled type for %2%", ft, f);
            }
        }
    }
}

bool InspectMtPsaProgram::preorder(const IR::Declaration_Variable *dv) {
    auto ft = typeMap->getType(dv->getNode(), true);
    cstring scalarsName = refMap->newName("scalars");

    if (ft->is<IR::Type_Bits>()) {
        LOG5("Adding " << dv << " into scalars map");
        pinfo->scalars.emplace(scalarsName, dv);
    } else if (ft->is<IR::Type_Boolean>()) {
        LOG5("Adding " << dv << " into scalars map");
        pinfo->scalars.emplace(scalarsName, dv);
    }

    return false;
}

// This visitor only visits the parameter in the statement from architecture.
bool InspectMtPsaProgram::preorder(const IR::Parameter* param) {
    auto ft = typeMap->getType(param->getNode(), true);
    LOG3("add param " << ft);
    // only convert parameters that are IR::Type_StructLike
    if (!ft->is<IR::Type_StructLike>()) {
      return false;
    }
    auto st = ft->to<IR::Type_StructLike>();
    // check if it is psa specific standard metadata
    cstring ptName = param->type->toString();
    // parameter must be a type that we have not seen before
    if (pinfo->hasVisited(st)) {
      LOG5("Parameter is visited, returning");
      return false;
    }
    if (MtPsaSwitchExpressionConverter::isStandardMetadata(ptName)) {
      LOG5("Parameter is psa standard metadata");
      addHeaderType(st);
      // remove _t from type name
      cstring headerName = ptName.exceptLast(2);
      addHeaderInstance(st, headerName);
    }
    auto isHeader = isHeaders(st);
    addTypesAndInstances(st, isHeader);
    return false;
}

void InspectMtPsaProgram::postorder(const IR::P4Parser* p) {
    if (pinfo->block_type.count(p)) {
        auto info = pinfo->block_type.at(p);
        if (info.first == INGRESS && info.second == PARSER)
            pinfo->parsers.emplace("ingress", p);
        else if (info.first == EGRESS && info.second == PARSER)
            pinfo->parsers.emplace("egress", p);
    }
}

void InspectMtPsaProgram::postorder(const IR::P4Control *c) {
    if (pinfo->block_type.count(c)) {
        auto info = pinfo->block_type.at(c);
        if (info.first == INGRESS && info.second == PIPELINE)
            pinfo->pipelines.emplace("ingress", c);
        else if (info.first == EGRESS && info.second == PIPELINE)
            pinfo->pipelines.emplace("egress", c);
        else if (info.first == INGRESS && info.second == DEPARSER)
            pinfo->deparsers.emplace("ingress", c);
        else if (info.first == EGRESS && info.second == DEPARSER)
            pinfo->deparsers.emplace("egress", c);
    }
}

void MtPsaSwitchBackend::convert(const IR::ToplevelBlock* tlb) {
    CHECK_NULL(tlb);
    MtPsaProgramStructure structure(refMap, typeMap);
    structure.userProgram = options.userProgram;

    auto parsePsaArch = new ParseMtPsaArchitecture(&structure);
    auto main = tlb->getMain();
    if (!main) return;

    auto switchName = options.userProgram ? "MTPSA_User_Switch" : "MTPSA_Switch";
    if (main->type->name != switchName)
        ::warning(ErrorType::WARN_INVALID, "%1%: the main package should be called %2%"
                  "; are you using the wrong architecture?", main->type->name, switchName);

    main->apply(*parsePsaArch);

    auto evaluator = new P4::EvaluatorPass(refMap, typeMap);
    auto program = tlb->getProgram();
    PassManager simplify = {
        /* TODO */
        // new RenameUserMetadata(refMap, userMetaType, userMetaName),
        new P4::ClearTypeMap(typeMap),  // because the user metadata type has changed
        new P4::SynthesizeActions(refMap, typeMap, new SkipControls(&structure.non_pipeline_controls)),
        new P4::MoveActionsToTables(refMap, typeMap),
        new P4::TypeChecking(refMap, typeMap),
        new P4::SimplifyControlFlow(refMap, typeMap),
        new LowerExpressions(typeMap),
        new P4::ConstantFolding(refMap, typeMap, false),
        new P4::TypeChecking(refMap, typeMap),
        new RemoveComplexExpressions(refMap, typeMap, new ProcessControls(&structure.pipeline_controls)),
        new P4::SimplifyControlFlow(refMap, typeMap),
        new P4::RemoveAllUnusedDeclarations(refMap),
        evaluator,
        [this, evaluator, structure]() { toplevel = evaluator->getToplevelBlock(); },
    };
    auto hook = options.getDebugHook();
    simplify.addDebugHook(hook);
    program->apply(simplify);

    // map IR node to compile-time allocated resource blocks.
    toplevel->apply(*new BMV2::BuildResourceMap(&structure.resourceMap));

    main = toplevel->getMain();
    if (!main)
      return;  // no main
    main->apply(*parsePsaArch);
    program = toplevel->getProgram();

    PassManager toJson = {
        new DiscoverStructure(&structure),
        new InspectMtPsaProgram(refMap, typeMap, &structure),
        new ConvertMtPsaToJson(refMap, typeMap, toplevel, json, &structure)
    };
    for (const auto &pEnum : *enumMap) {
      auto name = pEnum.first->getName();
      for (const auto &pEntry : *pEnum.second) {
        json->add_enum(name, pEntry.first, pEntry.second);
      }
    }
    program->apply(toJson);
    json->add_program_info(options.file);
    json->add_meta_info();
}

ExternConverter_Hash ExternConverter_Hash::singleton;
ExternConverter_Checksum ExternConverter_Checksum::singleton;
ExternConverter_InternetChecksum ExternConverter_InternetChecksum::singleton;
ExternConverter_Counter ExternConverter_Counter::singleton;
ExternConverter_DirectCounter ExternConverter_DirectCounter::singleton;
ExternConverter_Meter ExternConverter_Meter::singleton;
ExternConverter_DirectMeter ExternConverter_DirectMeter::singleton;
ExternConverter_Register ExternConverter_Register::singleton;
ExternConverter_Random ExternConverter_Random::singleton;
ExternConverter_ActionProfile ExternConverter_ActionProfile::singleton;
ExternConverter_ActionSelector ExternConverter_ActionSelector::singleton;
ExternConverter_Digest ExternConverter_Digest::singleton;

Util::IJson* ExternConverter_Hash::convertExternObject(
    UNUSED ConversionContext* ctxt, UNUSED const P4::ExternMethod* em,
    UNUSED const IR::MethodCallExpression* mc, UNUSED const IR::StatOrDecl *s,
    UNUSED const bool& emitExterns) {
    auto primitive = mkPrimitive("Hash");
    return primitive;
}

Util::IJson* ExternConverter_Checksum::convertExternObject(
    UNUSED ConversionContext* ctxt, UNUSED const P4::ExternMethod* em,
    UNUSED const IR::MethodCallExpression* mc, UNUSED const IR::StatOrDecl *s,
    UNUSED const bool& emitExterns) {
    auto primitive = mkPrimitive("Checksum");
    return primitive;
}

Util::IJson* ExternConverter_InternetChecksum::convertExternObject(
    UNUSED ConversionContext* ctxt,
    UNUSED const P4::ExternMethod* em,
    UNUSED const IR::MethodCallExpression* mc,
    UNUSED const IR::StatOrDecl *s,
    UNUSED const bool& emitExterns
) {
    Util::JsonObject* primitive = nullptr;
    if (em->method->name == "add" || em->method->name == "subtract" ||
        em->method->name == "get_state" || em->method->name == "set_state") {
        if (mc->arguments->size() != 1) {
            modelError("Expected 1 argument for %1%", mc);
            return nullptr;
        }
        primitive = mkPrimitive("_" + em->originalExternType->name + "_" + em->method->name);
    } else if (em->method->name == "get") {
        if (mc->arguments->size() == 1) {
            primitive = mkPrimitive("_" + em->originalExternType->name +
                                    "_" + em->method->name);
        } else if (mc->arguments->size() == 2) {
            primitive = mkPrimitive("_" + em->originalExternType->name +
                                    "_" + "get_verify");
        } else {
            modelError("Unexpected number of arguments for %1%", mc);
            return nullptr;
        }
    } else if (em->method->name == "clear") {
        if (mc->arguments->size() != 0) {
            modelError("Expected 0 argument for %1%", mc);
            return nullptr;
        }
        primitive = mkPrimitive("_" + em->originalExternType->name + "_" + em->method->name);
    }

    auto parameters = mkParameters(primitive);
    primitive->emplace_non_null("source_info", s->sourceInfoJsonObj());

    auto cksum = new Util::JsonObject();
    cksum->emplace("type", "extern");
    cksum->emplace("value", em->object->controlPlaneName());
    parameters->append(cksum);

    if (em->method->name == "add" || em->method->name == "subtract") {
        auto fieldList=new Util::JsonObject();
        fieldList->emplace("type","field_list");
        auto fieldsJson = ctxt->conv->convert(mc->arguments->at(0)->expression, true, false);
        fieldList->emplace("value",fieldsJson);
        parameters->append(fieldList);
    } else if (em->method->name != "clear") {
        if (mc->arguments->size() == 2) {
            // get_verify
            auto dst = ctxt->conv->convertLeftValue(mc->arguments->at(0)->expression);
            auto equOp = ctxt->conv->convert(mc->arguments->at(1)->expression);
            parameters->append(dst);
            parameters->append(equOp);
        } else if (mc->arguments->size() == 1) {
            // get or get_state or set_state
            auto dst = ctxt->conv->convert(mc->arguments->at(0)->expression);
            parameters->append(dst);
        }
    }
    return primitive;
}

Util::IJson* ExternConverter_Counter::convertExternObject(
    UNUSED ConversionContext* ctxt, UNUSED const P4::ExternMethod* em,
    UNUSED const IR::MethodCallExpression* mc, UNUSED const IR::StatOrDecl *s,
    UNUSED const bool& emitExterns) {
    if (mc->arguments->size() != 1) {
        modelError("Expected 1 argument for %1%", mc);
        return nullptr;
    }
    auto primitive = mkPrimitive("_" + em->originalExternType->name +
                                 "_" + em->method->name);
    auto parameters = mkParameters(primitive);
    primitive->emplace_non_null("source_info", s->sourceInfoJsonObj());
    auto ctr = new Util::JsonObject();
    ctr->emplace("type", "extern");
    ctr->emplace("value", em->object->controlPlaneName());
    parameters->append(ctr);
    auto index = ctxt->conv->convert(mc->arguments->at(0)->expression);
    parameters->append(index);
    return primitive;
}

Util::IJson* ExternConverter_DirectCounter::convertExternObject(
    UNUSED ConversionContext* ctxt, UNUSED const P4::ExternMethod* em,
    UNUSED const IR::MethodCallExpression* mc, UNUSED const IR::StatOrDecl *s,
    UNUSED const bool& emitExterns) {
    if (mc->arguments->size() != 0) {
        modelError("Expected 0 argument for %1%", mc);
        return nullptr;
    }
    // Do not generate any code for this operation
    return nullptr;
}

Util::IJson* ExternConverter_Meter::convertExternObject(
    UNUSED ConversionContext* ctxt, UNUSED const P4::ExternMethod* em,
    UNUSED const IR::MethodCallExpression* mc, UNUSED const IR::StatOrDecl *s,
    UNUSED const bool& emitExterns) {
    if (mc->arguments->size() != 2) {
        modelError("Expected 2 arguments for %1%", mc);
        return nullptr;
    }
    auto primitive = mkPrimitive("execute_meter");
    auto parameters = mkParameters(primitive);
    primitive->emplace_non_null("source_info", s->sourceInfoJsonObj());
    auto mtr = new Util::JsonObject();
    mtr->emplace("type", "meter_array");
    mtr->emplace("value", em->object->controlPlaneName());
    parameters->append(mtr);
    auto index = ctxt->conv->convert(mc->arguments->at(0)->expression);
    parameters->append(index);
    auto result = ctxt->conv->convert(mc->arguments->at(1)->expression);
    parameters->append(result);
    return primitive;
}

Util::IJson* ExternConverter_DirectMeter::convertExternObject(
    UNUSED ConversionContext* ctxt, UNUSED const P4::ExternMethod* em,
    UNUSED const IR::MethodCallExpression* mc, UNUSED const IR::StatOrDecl *s,
    UNUSED const bool& emitExterns) {
    if (mc->arguments->size() != 1) {
        modelError("Expected 1 argument for %1%", mc);
        return nullptr;
    }
    auto dest = mc->arguments->at(0);
    ctxt->structure->directMeterMap.setDestination(em->object, dest->expression);
    // Do not generate any code for this operation
    return nullptr;
}

Util::IJson* ExternConverter_Register::convertExternObject(
    UNUSED ConversionContext* ctxt, UNUSED const P4::ExternMethod* em,
    UNUSED const IR::MethodCallExpression* mc, UNUSED const IR::StatOrDecl *s,
    UNUSED const bool& emitExterns) {
    if (mc->arguments->size() != 2) {
        modelError("Expected 2 arguments for %1%", mc);
        return nullptr;
    }
    auto reg = new Util::JsonObject();
    reg->emplace("type", "register_array");
    cstring name = em->object->controlPlaneName();
    reg->emplace("value", name);
    if (em->method->name == "read") {
        auto primitive = mkPrimitive("register_read");
        auto parameters = mkParameters(primitive);
        primitive->emplace_non_null("source_info", s->sourceInfoJsonObj());
        /* TODO */
        // auto dest = ctxt->conv->convert(mc->arguments->at(0)->expression);
        // parameters->append(dest);
        parameters->append(reg);
        // auto index = ctxt->conv->convert(mc->arguments->at(1)->expression);
        // parameters->append(index);
        return primitive;
    } else if (em->method->name == "write") {
        auto primitive = mkPrimitive("register_write");
        auto parameters = mkParameters(primitive);
        primitive->emplace_non_null("source_info", s->sourceInfoJsonObj());
        parameters->append(reg);
        auto index = ctxt->conv->convert(mc->arguments->at(0)->expression);
        parameters->append(index);
        auto value = ctxt->conv->convert(mc->arguments->at(1)->expression);
        parameters->append(value);
        return primitive;
    }
    return nullptr;
}

Util::IJson* ExternConverter_Random::convertExternObject(
    UNUSED ConversionContext* ctxt, UNUSED const P4::ExternMethod* em,
    UNUSED const IR::MethodCallExpression* mc, UNUSED const IR::StatOrDecl *s,
    UNUSED const bool& emitExterns) {
    if (mc->arguments->size() != 3) {
        modelError("Expected 3 arguments for %1%", mc);
        return nullptr;
    }
    auto primitive =
        mkPrimitive("modify_field_rng_uniform");
    auto params = mkParameters(primitive);
    primitive->emplace_non_null("source_info", em->method->sourceInfoJsonObj());
    auto dest = ctxt->conv->convert(mc->arguments->at(0)->expression);
    /* TODO */
    // auto lo = ctxt->conv->convert(mc->arguments->at(1)->expression);
    // auto hi = ctxt->conv->convert(mc->arguments->at(2)->expression);
    params->append(dest);
    // params->append(lo);
    // params->append(hi);
    return primitive;
}

Util::IJson* ExternConverter_Digest::convertExternObject(
    UNUSED ConversionContext* ctxt, UNUSED const P4::ExternMethod* em,
    UNUSED const IR::MethodCallExpression* mc, UNUSED const IR::StatOrDecl *s,
    UNUSED const bool& emitExterns) {
    if (mc->arguments->size() != 1) {
        modelError("Expected 1 arguments for %1%", mc);
        return nullptr;
    }
    auto primitive = mkPrimitive("generate_digest");
    auto parameters = mkParameters(primitive);
    primitive->emplace_non_null("source_info", em->method->sourceInfoJsonObj());
    cstring listName = "digest";
    // If we are supplied a type argument that is a named type use
    // that for the list name.
    if (mc->typeArguments->size() == 1) {
        auto typeArg = mc->typeArguments->at(0);
        if (typeArg->is<IR::Type_Name>()) {
            auto origType = ctxt->refMap->getDeclaration(
                typeArg->to<IR::Type_Name>()->path, true);
            if (!origType->is<IR::Type_Struct>()) {
                modelError("%1%: expected a struct type", origType->getNode());
                return nullptr;
            }
            auto st = origType->to<IR::Type_Struct>();
            listName = st->controlPlaneName();
        }
    }
    int id = createFieldList(ctxt, mc->arguments->at(0)->expression, "learn_lists",
                             listName, ctxt->json->learn_lists);
    auto cst = new IR::Constant(id);
    ctxt->typeMap->setType(cst, IR::Type_Bits::get(32));
    auto jcst = ctxt->conv->convert(cst);
    parameters->append(jcst);
    return primitive;
}

void ExternConverter_Hash::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns)
{ /* TODO */ }

void ExternConverter_Checksum::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns)
{ /* TODO */ }

void ExternConverter_InternetChecksum::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns
) {
    auto inst = c->to<IR::Declaration_Instance>();
    cstring name = inst->controlPlaneName();
    auto trim = inst->controlPlaneName().find(".");
    auto block = inst->controlPlaneName().trim(trim);

    // FIXME: Fix egress structure and User pipeline support;
    auto mtpsaStructure = static_cast<MtPsaProgramStructure *>(ctxt->structure);
    auto ingressParser = mtpsaStructure->parsers.at("ingress")->controlPlaneName();
    auto ingressDeparser = mtpsaStructure->deparsers.at("ingress")->controlPlaneName();
    auto egressParser = mtpsaStructure->parsers.at("egress")->controlPlaneName();
    auto egressDeparser = mtpsaStructure->deparsers.at("egress")->controlPlaneName();

    if (block != ingressParser && block != ingressDeparser && block != egressParser && block != egressDeparser) {
        ::error(ErrorType::ERR_UNSUPPORTED, "%1%: not supported in pipeline on this target", eb);
        return;
    }

    // add checksum instance
    auto jcksum = new Util::JsonObject();
    jcksum->emplace("name", name);
    jcksum->emplace("id", nextId("extern_instances"));
    jcksum->emplace("type", eb->getName());
    jcksum->emplace_non_null("source_info", inst->sourceInfoJsonObj());
    ctxt->json->externs->append(jcksum);
}

void ExternConverter_Counter::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns) {
    auto inst = c->to<IR::Declaration_Instance>();
    cstring name = inst->controlPlaneName();
    auto jctr = new Util::JsonObject();
    jctr->emplace("name", name);
    jctr->emplace("id", nextId("counter_arrays"));
    jctr->emplace_non_null("source_info", eb->sourceInfoJsonObj());
    auto sz = eb->findParameterValue("n_counters");
    CHECK_NULL(sz);
    if (!sz->is<IR::Constant>()) {
        modelError("%1%: expected a constant", sz->getNode());
        return;
    }
    jctr->emplace("size", sz->to<IR::Constant>()->value);
    jctr->emplace("is_direct", false);
    ctxt->json->counters->append(jctr);

    // Code below used to add json into EXTERN_INSTANCES NODE

    auto extern_obj = new Util::JsonObject();
    extern_obj->emplace("name", name);
    extern_obj->emplace("id", nextId("extern_instances"));
    extern_obj->emplace("type", eb->getName());
    extern_obj->emplace("source_info", inst->sourceInfoJsonObj());
    ctxt->json->externs->append(extern_obj);
    Util::JsonArray *arr = ctxt->json->insert_array_field(extern_obj, "attribute_values");

    if (eb->getConstructorParameters()->size() != 2) {
      modelError("%1%: expected two parameters", eb);
      return;
    }
    // first argument to create a counter is just a number, convert and dump to json
    // we get a name from param, type and value from the arguments
    auto attr_obj = new Util::JsonObject();
    auto arg1 = sz->to<IR::Constant>();
    auto param1 = eb->getConstructorParameters()->getParameter(0);
    auto bitwidth = ctxt->typeMap->minWidthBits(arg1->type, sz->getNode());
    cstring repr = BMV2::stringRepr(arg1->value, ROUNDUP(bitwidth, 8));
    attr_obj->emplace("name", param1->toString());
    attr_obj->emplace("type", "hexstr");
    attr_obj->emplace("value", repr);
    arr->append(attr_obj);

    // second argument is the counter type, this is psa metadata, the converter
    // in conversion context will handle that for us
    auto tp = eb->findParameterValue("type");
    if (!tp || !tp->is<IR::Declaration_ID>()) {
      modelError("%1%: expected a declaration_id", tp ? tp->getNode() : eb->getNode());
      return;
    }
    auto arg2 = tp->to<IR::Declaration_ID>();
    auto param2 = eb->getConstructorParameters()->getParameter(1);
    auto mem = arg2->toString();
    LOG5("In convertParam with p2 " << param2->toString() << " and mem " << mem);
    auto jsn = ctxt->conv->convertParam(param2, mem);
    arr->append(jsn);
}

void ExternConverter_DirectCounter::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns) {
    auto inst = c->to<IR::Declaration_Instance>();
    cstring name = inst->controlPlaneName();
    auto it = ctxt->structure->directCounterMap.find(name);
    if (it == ctxt->structure->directCounterMap.end()) {
        ::warning(ErrorType::WARN_UNUSED, "%1%: Direct counter not used; ignoring", inst);
    } else {
        auto jctr = new Util::JsonObject();
        jctr->emplace("name", name);
        jctr->emplace("id", nextId("counter_arrays"));
        jctr->emplace_non_null("source_info", eb->sourceInfoJsonObj());
        jctr->emplace("is_direct", true);
        jctr->emplace("binding", it->second->controlPlaneName());
        ctxt->json->counters->append(jctr);

        // Adding direct counter to EXTERN_INSTANCES

        auto extern_obj = new Util::JsonObject();
        extern_obj->emplace("name", name);
        extern_obj->emplace("id", nextId("extern_instances"));
        extern_obj->emplace("type", eb->getName());
        extern_obj->emplace("source_info", inst->sourceInfoJsonObj());
        ctxt->json->externs->append(extern_obj);
        Util::JsonArray *arr = ctxt->json->insert_array_field(extern_obj, "attribute_values");

        // Direct Counter only has a single argument, which is psa metadata
        // converter in conversion context will handle this for us
        auto tp = eb->findParameterValue("type");
        if (!tp || !tp->is<IR::Declaration_ID>()) {
          modelError("%1%: expected a declaration_id", tp ? tp->getNode() : eb->getNode());
          return;
        }
        if (eb->getConstructorParameters()->size() < 2) {
          modelError("%1%: expected 2 parameters", eb);
          return;
        }
        auto arg = tp->to<IR::Declaration_ID>();
        auto param = eb->getConstructorParameters()->getParameter(1);
        auto mem = arg->toString();
        LOG5("In convertParam with param " << param->toString()
          << " and mem " << mem);
        auto jsn = ctxt->conv->convertParam(param, mem);
        arr->append(jsn);
    }
}

void ExternConverter_Meter::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns) {
    auto inst = c->to<IR::Declaration_Instance>();
    cstring name = inst->controlPlaneName();
    auto jmtr = new Util::JsonObject();
    jmtr->emplace("name", name);
    jmtr->emplace("id", nextId("meter_arrays"));
    jmtr->emplace_non_null("source_info", eb->sourceInfoJsonObj());
    jmtr->emplace("is_direct", false);
    auto sz = eb->findParameterValue("n_meters");
    CHECK_NULL(sz);
    if (!sz->is<IR::Constant>()) {
        modelError("%1%: expected a constant", sz->getNode());
        return;
    }
    jmtr->emplace("size", sz->to<IR::Constant>()->value);
    jmtr->emplace("rate_count", 2);
    auto mkind = eb->findParameterValue("type");
    CHECK_NULL(mkind);
    if (!mkind->is<IR::Declaration_ID>()) {
        modelError("%1%: expected a member", mkind->getNode());
        return;
    }
    cstring mkind_name = mkind->to<IR::Declaration_ID>()->name;
    cstring type = "?";
    if (mkind_name == "PACKETS")
        type = "packets";
    else if (mkind_name == "BYTES")
        type = "bytes";
    else
        ::error(ErrorType::ERR_UNEXPECTED, "%1%: unexpected meter type", mkind->getNode());
    jmtr->emplace("type", type);
    ctxt->json->meter_arrays->append(jmtr);
}

void ExternConverter_DirectMeter::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns) {
    auto inst = c->to<IR::Declaration_Instance>();
    cstring name = inst->controlPlaneName();
    auto info = ctxt->structure->directMeterMap.getInfo(c);
    CHECK_NULL(info);
    CHECK_NULL(info->table);
    CHECK_NULL(info->destinationField);

    auto jmtr = new Util::JsonObject();
    jmtr->emplace("name", name);
    jmtr->emplace("id", nextId("meter_arrays"));
    jmtr->emplace_non_null("source_info", eb->sourceInfoJsonObj());
    jmtr->emplace("is_direct", true);
    jmtr->emplace("rate_count", 2);
    auto mkind = eb->findParameterValue("type");
    CHECK_NULL(mkind);
    if (!mkind->is<IR::Declaration_ID>()) {
        modelError("%1%: expected a member", mkind->getNode());
        return;
    }
    cstring mkind_name = mkind->to<IR::Declaration_ID>()->name;
    cstring type = "?";
    if (mkind_name == "PACKETS") {
        type = "packets";
    } else if (mkind_name == "BYTES") {
        type = "bytes";
    } else {
        modelError("%1%: unexpected meter type", mkind->getNode());
        return;
    }
    jmtr->emplace("type", type);
    jmtr->emplace("size", info->tableSize);
    cstring tblname = info->table->controlPlaneName();
    jmtr->emplace("binding", tblname);
    auto result = ctxt->conv->convert(info->destinationField);
    jmtr->emplace("result_target", result->to<Util::JsonObject>()->get("value"));
    ctxt->json->meter_arrays->append(jmtr);
}

void ExternConverter_Register::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns) {
    auto inst = c->to<IR::Declaration_Instance>();
    cstring name = inst->controlPlaneName();
    auto jreg = new Util::JsonObject();
    jreg->emplace("name", name);
    jreg->emplace("id", nextId("register_arrays"));
    jreg->emplace_non_null("source_info", eb->sourceInfoJsonObj());
    auto sz = eb->findParameterValue("size");
    CHECK_NULL(sz);
    if (!sz->is<IR::Constant>()) {
        modelError("%1%: expected a constant", sz->getNode());
        return;
    }
    if (sz->to<IR::Constant>()->value == 0)
        error(ErrorType::ERR_UNSUPPORTED, "%1%: direct registers are not supported", inst);
    jreg->emplace("size", sz->to<IR::Constant>()->value);
    if (!eb->instanceType->is<IR::Type_SpecializedCanonical>()) {
        modelError("%1%: Expected a generic specialized type", eb->instanceType);
        return;
    }
    auto st = eb->instanceType->to<IR::Type_SpecializedCanonical>();
    if (st->arguments->size() != 1) {
        modelError("%1%: expected 1 type argument", st);
        return;
    }
    auto regType = st->arguments->at(0);
    if (!regType->is<IR::Type_Bits>()) {
        ::error(ErrorType::ERR_UNSUPPORTED,
                "%1%: registers with types other than bit<> or int<> are not suppoted", eb);
        return;
    }
    unsigned width = regType->width_bits();
    if (width == 0) {
        ::error(ErrorType::ERR_UNKNOWN, "%1%: unknown width", st->arguments->at(0));
        return;
    }
    jreg->emplace("bitwidth", width);
    ctxt->json->register_arrays->append(jreg);
}

void ExternConverter_Random::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns)
{ /* TODO */ }

void ExternConverter_ActionProfile::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns) {
    auto inst = c->to<IR::Declaration_Instance>();
    cstring name = inst->controlPlaneName();
    // Might call this multiple times if the selector/profile is used more than
    // once in a pipeline, so only add it to the action_profiles once
    if (BMV2::JsonObjects::find_object_by_name(ctxt->action_profiles, name))
        return;
    auto action_profile = new Util::JsonObject();
    action_profile->emplace("name", name);
    action_profile->emplace("id", nextId("action_profiles"));
    action_profile->emplace_non_null("source_info", eb->sourceInfoJsonObj());

    auto sz = eb->findParameterValue("size");
    BUG_CHECK(sz, "%1%Invalid declaration of extern ActionProfile ctor: no size param",
            eb->constructor->srcInfo);

    if (!sz->is<IR::Constant>()) {
        ::error(ErrorType::ERR_EXPECTED, "%1%: expected a constant", sz);
    }
    action_profile->emplace("max_size", sz->to<IR::Constant>()->value);

    ctxt->action_profiles->append(action_profile);
}

void ExternConverter_ActionSelector::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns) {
    auto inst = c->to<IR::Declaration_Instance>();
    cstring name = inst->controlPlaneName();
    // Might call this multiple times if the selector/profile is used more than
    // once in a pipeline, so only add it to the action_profiles once
    if (BMV2::JsonObjects::find_object_by_name(ctxt->action_profiles, name))
        return;
    auto action_profile = new Util::JsonObject();
    action_profile->emplace("name", name);
    action_profile->emplace("id", nextId("action_profiles"));
    action_profile->emplace_non_null("source_info", eb->sourceInfoJsonObj());

    auto sz = eb->findParameterValue("size");
    BUG_CHECK(sz, "%1%Invalid declaration of extern ActionProfile ctor: no size param",
        eb->constructor->srcInfo);

    if (!sz->is<IR::Constant>()) {
        ::error(ErrorType::ERR_EXPECTED, "%1%: expected a constant", sz);
        return;
    }
    action_profile->emplace("max_size", sz->to<IR::Constant>()->value);

    auto selector = new Util::JsonObject();
    auto hash = eb->findParameterValue("algo");

    if (!hash->is<IR::Declaration_ID>()) {
        modelError("%1%: expected a member", hash->getNode());
        return;
    }
    auto algo = ExternConverter::convertHashAlgorithm(hash->to<IR::Declaration_ID>()->name);
    selector->emplace("algo", algo);
    auto input = ctxt->get_selector_input(
        c->to<IR::Declaration_Instance>());
    if (input == nullptr) {
        // the selector is never used by any table, we cannot figure out its
        // input and therefore cannot include it in the JSON
        ::warning(ErrorType::WARN_UNUSED, "Action selector '%1%' is never referenced by a table "
                  "and cannot be included in bmv2 JSON", c);
        return;
    }
    auto j_input = mkArrayField(selector, "input");
    for (auto expr : *input) {
        auto jk = ctxt->conv->convert(expr);
        j_input->append(jk);
    }
    action_profile->emplace("selector", selector);

    ctxt->action_profiles->append(action_profile);
}

void ExternConverter_Digest::convertExternInstance(
    UNUSED ConversionContext* ctxt, UNUSED const IR::Declaration* c,
    UNUSED const IR::ExternBlock* eb, UNUSED const bool& emitExterns)
{ /* TODO */ }

}  // namespace BMV2
