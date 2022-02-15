#pragma once

#include "Allocator.h"
#include "CompileStructMember.h"
#include "IO.h"
#include "Instruction.h"
#include "InstructionList.h"
#include "State.h"

#include <errno.h>

typedef struct VariableTarget
{
    const TemLangString* name;
    enum
    {
        VariableTarget_None,
        VariableTarget_Variable,
        VariableTarget_ReturnValue,
    } type;
} VariableTarget, *pVariableTarget;

static size_t scopeNumber = 0;
static size_t variableId = 0;
static size_tList returnValueScopes = { 0 };
static size_t noCleanupState = 0;
#define MAX_RETURN_VALUE_SCOPES 512

static inline void
prepareCompiler()
{
    static Allocator noAllocator = { 0 };
    noAllocator = makeNoAllocator();
    static size_t buffer[MAX_RETURN_VALUE_SCOPES] = { 0 };
    returnValueScopes.allocator = &noAllocator;
    returnValueScopes.buffer = buffer;
    returnValueScopes.size = MAX_RETURN_VALUE_SCOPES;
    returnValueScopes.used = 0;
    noCleanupState = 0;
}

#define GET_RETURN_VALUE_SCOPE(f)                                              \
    if (size_tListIsEmpty(&returnValueScopes)) {                               \
        TemLangError("Compiler error! No return value scope in stack.");       \
        f                                                                      \
    }                                                                          \
    const size_t returnScope =                                                 \
      returnValueScopes.buffer[returnValueScopes.used - 1UL];

static inline bool
CompileInstruction(State*,
                   const Instruction*,
                   const Allocator*,
                   const VariableTarget,
                   pTemLangString);

static inline TemLangString
CompilerAssignValue(const State* state,
                    const TemLangString* name,
                    const Allocator* allocator,
                    const Value* value,
                    const Expression* expression,
                    const bool create);

static inline TemLangString
CompilerGetExpression(const State* state,
                      const Allocator* allocator,
                      const VariableTarget target,
                      const Value* value,
                      const Expression* e);

static inline bool
GetSimpleExpressionName(const State*,
                        const Allocator*,
                        const Expression* e,
                        pTemLangString s);

static inline bool
ExpressionIsSimple(const Expression* e);

static inline bool
ValueNeedsCleanup(const ValueType type)
{
    switch (type) {
        case ValueType_String:
        case ValueType_Variant:
        case ValueType_Struct:
        case ValueType_List:
            return true;
        default:
            return false;
    }
}

static inline TemLangString
ValueFreeName(const Value* value, const Allocator* allocator)
{
    TemLangString s = TemLangStringCreate("", allocator);
    switch (value->type) {
        case ValueType_String:
            TemLangStringAppendChars(&s, "TemLangStringFree");
            break;
        case ValueType_Boolean: {
            TemLangStringAppendChars(&s, "booleanFree");
        } break;
        case ValueType_Variant: {
            TemLangStringAppendFormat(
              s, "%sFree", value->variantValue.name.buffer);
        } break;
        case ValueType_Enum: {
            TemLangStringAppendFormat(
              s, "%sFree", value->enumValue.name.buffer);
        } break;
        case ValueType_Flag: {
            TemLangStringAppendFormat(
              s, "%sFree", value->flagValue.name.buffer);
        } break;
        case ValueType_Number: {
            CType type = { 0 };
            if (value->rangedNumber.hasRange) {
                type = RangeToCType(&value->rangedNumber.range);
            } else {
                type = NumberToCType(&value->rangedNumber.number);
            }
            TemLangStringAppendFormat(s, "%sFree", CTypeToTypeString(type));
        } break;
        case ValueType_Null: {
            TemLangStringAppendChars(&s, "NullValueFree");
        } break;
        case ValueType_List: {
            TemLangString s1 =
              ValueFreeName(value->list.exampleValue, allocator);
            TemLangStringAppendFormat(s, "%sListFree", s1.buffer);
            TemLangStringFree(&s1);
        } break;
        default:
            TemLangStringAppendChars(&s, "NoFree");
            break;
    }
    return s;
}

static inline TemLangString
ValueCopyName(const Value* value, const Allocator* allocator)
{
    TemLangString s = TemLangStringCreate("", allocator);
    switch (value->type) {
        case ValueType_String:
            TemLangStringAppendChars(&s, "TemLangStringCopy");
            break;
        case ValueType_Boolean: {
            TemLangStringAppendChars(&s, "booleanCopy");
        } break;
        case ValueType_Enum: {
            TemLangStringAppendFormat(
              s, "%sCopy", value->enumValue.name.buffer);
        } break;
        case ValueType_Flag: {
            TemLangStringAppendFormat(
              s, "%sCopy", value->flagValue.name.buffer);
        } break;
        case ValueType_Variant: {
            TemLangStringAppendFormat(
              s, "%sCopy", value->variantValue.name.buffer);
        } break;
        case ValueType_Number: {
            CType type = { 0 };
            if (value->rangedNumber.hasRange) {
                type = RangeToCType(&value->rangedNumber.range);
            } else {
                type = NumberToCType(&value->rangedNumber.number);
            }
            TemLangStringAppendFormat(s, "%sCopy", CTypeToTypeString(type));
        } break;
        case ValueType_Null: {
            TemLangStringAppendChars(&s, "NullValueCopy");
        } break;
        case ValueType_List: {
            TemLangString s1 =
              ValueFreeName(value->list.exampleValue, allocator);
            TemLangStringAppendFormat(s, "%sListCopy", s1.buffer);
            TemLangStringFree(&s1);
        } break;
        default:
            TemLangStringAppendChars(&s, "DefaultCopy");
            break;
    }
    return s;
}

static inline TemLangString
CompileValueCleanup(const TemLangString* name,
                    const Value* value,
                    const Allocator* allocator)
{
    TemLangString s = TemLangStringCreate("", allocator);
    switch (value->type) {
        case ValueType_List: {
            if (ValueNeedsCleanup(value->list.exampleValue->type)) {
                TemLangString looper = TemLangStringCreate("", allocator);
                TemLangString c = TemLangStringCreate("", allocator);
                if (value->list.isArray) {
                    {
                        TemLangStringAppendFormat(
                          looper,
                          "{for(size_t listCleanupIndex = 0; "
                          "listCleanupIndex < %u; ++listCleanupIndex){",
                          value->list.values.used);
                    }
                    {
                        TemLangStringAppendFormat(
                          c, "%s[listCleanupIndex]", name->buffer);
                    }
                } else {
                    {
                        TemLangStringAppendFormat(
                          looper,
                          "{for(size_t listCleanupIndex = 0; "
                          "listCleanupIndex < "
                          "%s.used; ++listCleanupIndex){",
                          name->buffer);
                    }
                    {
                        TemLangStringAppendFormat(
                          c, "%s.buffer[listCleanupIndex]", name->buffer);
                    }
                }
                TemLangString k =
                  CompileValueCleanup(&c, value->list.exampleValue, allocator);
                if (!TemLangStringIsEmpty(&k)) {
                    TemLangStringAppend(&s, &looper);
                    TemLangStringAppend(&s, &k);
                    TemLangStringAppendChars(&s, "}}");
                }
                TemLangStringFree(&looper);
                TemLangStringFree(&k);
                TemLangStringFree(&c);
            }
            if (!value->list.isArray) {
                TemLangStringAppendFormat(
                  s,
                  "if(%s.buffer != NULL){ "
                  "%s.allocator->free(%s.buffer); %s.buffer "
                  "= NULL; }",
                  name->buffer,
                  name->buffer,
                  name->buffer,
                  name->buffer);
            }
        } break;
        case ValueType_Struct: {
            char buffer[256] = { 0 };
            TemLangString n = { .buffer = buffer, .allocator = NULL };
            for (size_t i = 0; i < value->structValues->used; ++i) {
                const NamedValue* nv = &value->structValues->buffer[i];
                n.used = n.size = snprintf(buffer,
                                           sizeof(buffer),
                                           "%s.%s",
                                           name->buffer,
                                           nv->name.buffer);
                TemLangString s1 =
                  CompileValueCleanup(&n, &nv->value, allocator);
                TemLangStringAppend(&s, &s1);
                TemLangStringFree(&s1);
            }
        } break;
        case ValueType_Variant:
        case ValueType_String: {
            TemLangString s1 = ValueFreeName(value, allocator);
            TemLangStringAppendFormat(s, "%s(&%s);", s1.buffer, name->buffer);
            TemLangStringFree(&s1);
        } break;
        default:
            break;
    }
    return s;
}

static inline TemLangString
CompileStateCleanup(const State* state, const Allocator* allocator)
{
    TemLangString a = TemLangStringCreate("", allocator);
    for (size_t i = 0; i < state->atoms.used; ++i) {
        const Atom* atom = &state->atoms.buffer[i];
        if (atom->type != AtomType_Variable || atom->notCompiled) {
            continue;
        }
        TemLangString s1 =
          CompileValueCleanup(&atom->name, &atom->variable.value, allocator);
        TemLangStringAppend(&a, &s1);
        TemLangStringFree(&s1);
    }
    TemLangString s = { .allocator = allocator };
    if (a.used > 0) {
        if (noCleanupState == 0) {
            TemLangStringAppendFormat(s, "\n//Cleanup\n%s", a.buffer);
        } else {
            TemLangStringList list = { .allocator = allocator };
            for (size_t i = 0; i < state->atoms.used; ++i) {
                const Atom* atom = &state->atoms.buffer[i];
                if (atom->type != AtomType_Variable || atom->notCompiled) {
                    continue;
                }
                TemLangStringListAppend(&list, &atom->name);
            }
            if (!TemLangStringListIsEmpty(&list)) {
                TemLangString s = TemLangStringListToString(&list, allocator);
                TemLangStringAppendFormat(
                  a, "\n//Cleanup disabled! Leaked variables: %s\n", s.buffer);
                TemLangStringFree(&s);
            }
            TemLangStringListFree(&list);
        }
    }
    TemLangStringFree(&a);
    return s;
}

#define COMPILE_STATE_CLEANUP(state, s)                                        \
    {                                                                          \
        TemLangString k = CompileStateCleanup(&state, allocator);              \
        TemLangStringAppend(&s, &k);                                           \
        TemLangStringFree(&k);                                                 \
        StateFree(&state);                                                     \
    }

#define COMPILE_COPIED_STATE_CLEANUP(orig, copy, s)                            \
    for (size_t i = 0; i < orig.atoms.used; ++i) {                             \
        AtomListRemove(&copy.atoms, 0, allocator);                             \
    }                                                                          \
    COMPILE_STATE_CLEANUP(copy, s);

static inline bool
CompileInstructions(const InstructionList* list,
                    const Allocator* allocator,
                    const VariableTarget target,
                    pState state,
                    pTemLangString s)
{
    bool result = true;
    for (size_t i = 0; result && i < list->used; ++i) {
        result =
          CompileInstruction(state, &list->buffer[i], allocator, target, s);
    }
    return result;
}

static inline TemLangString
CompilerGetVariableName(const Expression* ref,
                        const State* state,
                        const Allocator* allocator)
{
    TemLangString s = TemLangStringCreate("", allocator);
    switch (ref->type) {
        case ExpressionType_UnaryVariable:
            TemLangStringAppend(&s, &ref->identifier);
            break;
        case ExpressionType_Binary: {
            TemLangString s1 =
              CompilerGetVariableName(ref->left, state, allocator);
            TemLangStringAppend(&s, &s1);
            TemLangStringFree(&s1);
        } break;
        default:
            break;
    }
    return s;
}

static inline TemLangString
CompilerValueIndexerName(const Value* v, const Allocator* allocator);

static inline TemLangString
CompilerExpressionIndexerName(const Expression* e,
                              const State* state,
                              const Allocator* allocator)
{
    Value value = { 0 };
    TemLangString s = { .allocator = allocator };
    if (EvaluateExpression(e, state, &value, allocator)) {
        switch (e->type) {
            case ExpressionType_UnaryVariable: {
                switch (value.type) {
                    case ValueType_Number:
                    case ValueType_Enum:
                        TemLangStringAppendFormat(
                          s, "[%s]", e->identifier.buffer);
                        break;
                    default:
                        TemLangStringAppendFormat(
                          s, ".%s", e->identifier.buffer);
                        break;
                }
            } break;
            default:
                s = CompilerValueIndexerName(&value, allocator);
                break;
        }
    } else {
        s = TemLangStringCreate("", allocator);
    }
    ValueFree(&value);
    return s;
}

static inline TemLangString
CompilerValueIndexerName(const Value* v, const Allocator* allocator)
{
    TemLangString s = { .allocator = allocator };
    switch (v->type) {
        case ValueType_Number: {
            const int64_t i = NumberToInt(&v->rangedNumber.number);
            TemLangStringAppendFormat(s, "[%" PRId64 "]", i);
        } break;
        case ValueType_String:
            TemLangStringAppendFormat(s, ".%s", v->string.buffer);
            break;
        case ValueType_Enum: {
            TemLangStringAppendFormat(s,
                                      "[%s_%s]",
                                      v->enumValue.name.buffer,
                                      v->enumValue.value.buffer);
        } break;
        default: {
            TemLangError("Value type '%s' cannot be a indexer",
                         ValueTypeToString(v->type));
        } break;
    }
    return s;
}

static inline TemLangString
CompilerGetFullVariableName(const Expression* ref,
                            const State* state,
                            const Allocator* allocator);

static inline bool
TryCompilerGetFullVariableName(const Expression* ref,
                               const State* state,
                               const Allocator* allocator,
                               pTemLangString s)
{
    switch (ref->type) {
        case ExpressionType_UnaryVariable:
            TemLangStringAppend(s, &ref->identifier);
            return true;
        case ExpressionType_Binary: {
            {
                TemLangString s1 =
                  CompilerGetFullVariableName(ref->left, state, allocator);
                TemLangStringAppend(s, &s1);
                TemLangStringFree(&s1);
            }
            TemLangString indexer =
              CompilerExpressionIndexerName(ref->right, state, allocator);
            TemLangStringAppend(s, &indexer);
            TemLangStringFree(&indexer);
            return s->used > 0;
        } break;
        default:
            break;
    }
    return false;
}

static inline TemLangString
CompilerGetFullVariableName(const Expression* ref,
                            const State* state,
                            const Allocator* allocator)
{
    TemLangString s = { .allocator = allocator };
    if (!TryCompilerGetFullVariableName(ref, state, allocator, &s)) {
        TemLangString k = ExpressionToString(ref, allocator);
        TemLangError("Failed to find name of variable from expression '%s'",
                     k.buffer);
        TemLangStringFree(&k);
    }
    return s;
}

static inline TemLangString
CompilerDeclareValueType(const TemLangString* name,
                         const State* state,
                         const Allocator* allocator,
                         const Value* value,
                         const bool setDefault)
{
    TemLangString s = TemLangStringCreate("", allocator);
    switch (value->type) {
        case ValueType_Null:
            if (name == NULL) {
                s = TemLangStringCreate("NullValue", allocator);
            } else {
                TemLangStringCreateFormat(
                  a, allocator, "NullValue %s;", name->buffer);
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
            }
            break;
        case ValueType_Boolean:
            if (name == NULL) {
                s = TemLangStringCreate("boolean", allocator);
            } else {
                TemLangStringCreateFormat(a,
                                          allocator,
                                          "boolean %s%s;",
                                          name->buffer,
                                          setDefault ? "= false" : "");
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
            }
            break;
        case ValueType_String:
            if (name == NULL) {
                s = TemLangStringCreate("TemLangString", allocator);
            } else {
                TemLangStringCreateFormat(
                  a,
                  allocator,
                  "TemLangString %s%s;",
                  name->buffer,
                  setDefault ? "={.used =0, .size = 0, .buffer = NULL, "
                               ".allocator = currentAllocator}"
                             : "");
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
            }
            break;
        case ValueType_Number: {
            const CType type = value->rangedNumber.hasRange
                                 ? RangeToCType(&value->rangedNumber.range)
                                 : NumberToCType(&value->rangedNumber.number);
            if (name == NULL) {
                TemLangStringCreateFormat(
                  a, allocator, "%s", CTypeToTypeString(type));
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
            } else {
                TemLangStringCreateFormat(a,
                                          allocator,
                                          "%s %s%s;",
                                          CTypeToTypeString(type),
                                          name->buffer,
                                          setDefault ? "=0" : "");
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
            }
        } break;
        case ValueType_Enum:
            if (name == NULL) {
                TemLangStringCreateFormat(
                  a, allocator, "%s", value->enumValue.name.buffer);
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
            } else {
                TemLangStringCreateFormat(a,
                                          allocator,
                                          "%s %s%s;",
                                          value->enumValue.name.buffer,
                                          name->buffer,
                                          setDefault ? "=-1L" : "");
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
            }
            break;
        case ValueType_Flag:
            if (name == NULL) {
                TemLangStringCreateFormat(
                  a, allocator, "%s", value->flagValue.name.buffer);
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
            } else {
                TemLangStringCreateFormat(a,
                                          allocator,
                                          "%s %s%s;",
                                          value->enumValue.name.buffer,
                                          name->buffer,
                                          setDefault ? "=0" : "");
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
            }
            break;
        case ValueType_List: {
            if (name == NULL) {
                if (value->list.isArray) {
                    TemLangError("Cannot declare a array  without a name.");
                } else {
                    TemLangString b = CompilerDeclareValueType(
                      NULL, state, allocator, value->list.exampleValue, false);
                    TemLangStringCreateFormat(a, allocator, "%sList", b.buffer);
                    TemLangStringCopy(&s, &a, allocator);
                    TemLangStringFree(&a);
                    TemLangStringFree(&b);
                }
            } else {
                if (value->list.isArray) {
                    if (useCGLM &&
                        value->list.exampleValue->type == ValueType_Number &&
                        RangeToCType(
                          &value->list.exampleValue->rangedNumber.range) ==
                          CType_f32) {
                        switch (value->list.values.used) {
                            case 2: {
                                TemLangStringCreateFormat(a,
                                                          allocator,
                                                          "vec2 %s%s;",
                                                          name->buffer,
                                                          setDefault ? "={0}"
                                                                     : "");
                                TemLangStringAppend(&s, &a);
                                TemLangStringFree(&a);
                                goto doneWithArrayCheck;
                            } break;
                            case 3: {
                                TemLangStringCreateFormat(a,
                                                          allocator,
                                                          "vec3 %s%s;",
                                                          name->buffer,
                                                          setDefault ? "={0}"
                                                                     : "");
                                TemLangStringAppend(&s, &a);
                                TemLangStringFree(&a);
                                goto doneWithArrayCheck;
                            } break;
                            case 4: {
                                TemLangStringCreateFormat(a,
                                                          allocator,
                                                          "vec4 %s%s;",
                                                          name->buffer,
                                                          setDefault ? "={0}"
                                                                     : "");
                                TemLangStringAppend(&s, &a);
                                TemLangStringFree(&a);
                                goto doneWithArrayCheck;
                            } break;
                            case 9: {
                                TemLangStringCreateFormat(a,
                                                          allocator,
                                                          "mat3 %s%s;",
                                                          name->buffer,
                                                          setDefault ? "={0}"
                                                                     : "");
                                TemLangStringAppend(&s, &a);
                                TemLangStringFree(&a);
                                goto doneWithArrayCheck;
                            } break;
                            case 16: {
                                TemLangStringCreateFormat(a,
                                                          allocator,
                                                          "mat4 %s%s;",
                                                          name->buffer,
                                                          setDefault ? "={0}"
                                                                     : "");
                                TemLangStringAppend(&s, &a);
                                TemLangStringFree(&a);
                                goto doneWithArrayCheck;
                            } break;
                            default:
                                break;
                        }
                    }
                    TemLangStringCreateFormat(b,
                                              allocator,
                                              "%s[%u]",
                                              name->buffer,
                                              value->list.values.used);
                    TemLangString a = CompilerDeclareValueType(
                      &b, state, allocator, value->list.exampleValue, false);
                    TemLangStringCopy(&s, &a, allocator);
                    TemLangStringFree(&a);
                    TemLangStringFree(&b);
                    if (setDefault) {
                        TemLangStringAppendFormat(s,
                                                  "memset(%s, 0, sizeof(%s));",
                                                  name->buffer,
                                                  name->buffer);
                    }
                } else {
                    TemLangString b = CompilerDeclareValueType(
                      NULL, state, allocator, value->list.exampleValue, false);
                    TemLangStringCreateFormat(a,
                                              allocator,
                                              "%sList %s%s;",
                                              b.buffer,
                                              name->buffer,
                                              setDefault
                                                ? "={.used=0,.size=0,."
                                                  "allocator=currentAllocator}"
                                                : "");
                    TemLangStringCopy(&s, &a, allocator);
                    TemLangStringFree(&a);
                    TemLangStringFree(&b);
                }
            doneWithArrayCheck:
                break;
            }
        } break;
        case ValueType_Struct: {
            if (name == NULL) {
                // Make a fake name
                TemLangString fakeName =
                  TemLangStringCreate("unnamed_struct", allocator);
                NamedValue nv = { .name = fakeName, .value = *value };
                StructMember m = { 0 };
                if (NamedValueToStructMember(&nv, state, &m, allocator, true)) {
                    TemLangStringAppend(&s, &m.typeName);
                }
                StructMemberFree(&m);
                TemLangStringFree(&fakeName);
                break;
            }
            {
                {
                    NamedValue nv = { .name = *name, .value = *value };
                    StructMember m = { 0 };
                    if (NamedValueToStructMember(
                          &nv, state, &m, allocator, false)) {
                        TemLangStringAppendFormat(s,
                                                  "%s %s%s;",
                                                  m.typeName.buffer,
                                                  name->buffer,
                                                  setDefault ? "={0}" : "");
                        StructMemberFree(&m);
                        break;
                    }
                    StructMemberFree(&m);
                }

                TemLangString b = TemLangStringCreate("", allocator);
                for (size_t i = 0; i < value->structValues->used; ++i) {
                    const NamedValue* nv = &value->structValues->buffer[i];
                    TemLangString c = CompilerDeclareValueType(
                      &nv->name, state, allocator, &nv->value, false);
                    TemLangStringAppend(&b, &c);
                    TemLangStringFree(&c);
                }
                TemLangStringCreateFormat(
                  a, allocator, "struct { %s } %s;", b.buffer, name->buffer);
                TemLangStringFree(&b);
                if (setDefault) {
                    TemLangStringAppendFormat(a,
                                              "memset(&%s, 0, sizeof(%s));",
                                              name->buffer,
                                              name->buffer);
                }
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
            }
        } break;
        case ValueType_Variant: {
            if (name == NULL) {
                TemLangStringCreateFormat(
                  a, allocator, "%s", value->variantValue.name.buffer);
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
            } else {
                TemLangStringCreateFormat(a,
                                          allocator,
                                          "%s %s;",
                                          value->variantValue.name.buffer,
                                          name->buffer);
                TemLangStringCopy(&s, &a, allocator);
                TemLangStringFree(&a);
                if (setDefault) {
                    TemLangStringAppendFormat(s,
                                              "memset(&%s,0,sizeof(%s));",
                                              name->buffer,
                                              value->variantValue.name.buffer);
                }
            }
        } break;
        default:
            TemLangError("Value '%s' could not be declared",
                         ValueTypeToString(value->type));
            break;
    }
    return s;
}

static inline TemLangString
CompilerGetValue(const Value* value, const Allocator* allocator)
{
    TemLangString s = { 0 };
    switch (value->type) {
        case ValueType_Null:
            s = TemLangStringCreate("NULL", allocator);
            break;
        case ValueType_Number:
            s = NumberToString(&value->rangedNumber.number, allocator);
            break;
        case ValueType_Boolean: {
            TemLangStringCreateFormat(
              a, allocator, "%s", value->b ? "true" : "false");
            s = a;
        } break;
        case ValueType_String: {
            TemLangStringCreateFormat(
              a, allocator, "\"%s\"", value->string.buffer);
            s = a;
        } break;
        case ValueType_Enum: {
            TemLangStringCreateFormat(a,
                                      allocator,
                                      "%s_%s",
                                      value->enumValue.name.buffer,
                                      value->enumValue.value.buffer);
            s = a;
        } break;
        case ValueType_Flag: {
            if (value->flagValue.members.used == 0) {
                s = TemLangStringCreate("0", allocator);
            } else {
                s = TemLangStringCreate("", allocator);
                for (size_t i = 0; i < value->flagValue.members.used; ++i) {
                    TemLangStringAppendFormat(
                      s,
                      "%s_%s %c",
                      value->flagValue.name.buffer,
                      value->flagValue.members.buffer[i].buffer,
                      i == value->flagValue.members.used - 1 ? ' ' : '|');
                }
            }
        } break;
        case ValueType_Variant:
            TemLangStringCopy(&s, &value->variantValue.memberName, allocator);
            break;
        default:
            TemLangError("Cannot compile value '%s'",
                         ValueTypeToString(value->type));
            break;
    }
    return s;
}

#define MAKE_LEFT_AND_RIGHT(leftP, rightP)                                     \
    TemLangString ls = { 0 };                                                  \
    TemLangString rs = { 0 };                                                  \
    {                                                                          \
        ls = TemLangStringCreate("left", allocator);                           \
        rs = TemLangStringCreate("right", allocator);                          \
        if (target.type == VariableTarget_Variable) {                          \
            while (TemLangStringCompare(&ls, target.name) ==                   \
                   ComparisonOperator_EqualTo) {                               \
                TemLangStringAppendChar(&ls, '_');                             \
            }                                                                  \
            while (TemLangStringCompare(&rs, target.name) ==                   \
                   ComparisonOperator_EqualTo) {                               \
                TemLangStringAppendChar(&rs, '_');                             \
            }                                                                  \
        }                                                                      \
        TemLangString a =                                                      \
          CompilerAssignValue(state, &ls, allocator, &leftP, e->left, true);   \
        TemLangString b =                                                      \
          CompilerAssignValue(state, &rs, allocator, &rightP, e->right, true); \
        TemLangStringAppend(&s, &a);                                           \
        TemLangStringAppend(&s, &b);                                           \
        TemLangStringFree(&a);                                                 \
        TemLangStringFree(&b);                                                 \
    }

static inline TemLangString
CopmilerGetBooleanExpression(const State* state,
                             const Allocator* allocator,
                             const VariableTarget target,
                             const Value* left,
                             const Value* right,
                             const Expression* e)
{
    TemLangString s = TemLangStringCreate("", allocator);
    if (left->type == ValueType_Boolean && right->type == ValueType_Boolean) {
        TemLangString s1 = { .allocator = allocator };
        TemLangString s2 = { .allocator = allocator };

        if (!GetSimpleExpressionName(state, allocator, e->left, &s1) ||
            !GetSimpleExpressionName(state, allocator, e->right, &s2)) {
            MAKE_LEFT_AND_RIGHT((*left), (*right));
            TemLangStringCopy(&s1, &ls, allocator);
            TemLangStringCopy(&s2, &rs, allocator);
            TemLangStringFree(&ls);
            TemLangStringFree(&rs);
        }

        switch (target.type) {
            case VariableTarget_ReturnValue: {
                TemLangStringAppendFormat(
                  s,
                  "return %s %s %s;",
                  s1.buffer,
                  BooleanOperatorToChars(e->op.booleanOperator),
                  s2.buffer);
            } break;
            case VariableTarget_Variable: {
                TemLangStringAppendFormat(
                  s,
                  "%s = %s %s %s;",
                  target.name->buffer,
                  s1.buffer,
                  BooleanOperatorToChars(e->op.booleanOperator),
                  s2.buffer);
            } break;
            case VariableTarget_None:
            default: {
                TemLangStringAppendFormat(
                  s,
                  "%s %s %s",
                  s1.buffer,
                  BooleanOperatorToChars(e->op.booleanOperator),
                  s2.buffer);
            } break;
                break;
        }
        TemLangStringFree(&s1);
        TemLangStringFree(&s2);
    } else if (e->op.booleanOperator == BooleanOperator_Not) {
        const Value* realValue = getUnaryValue(left, right);
        TemLangString s1 = { .allocator = allocator };
        if (GetSimpleExpressionName(
              state, allocator, realValue == left ? e->left : e->right, &s1)) {
            switch (target.type) {
                case VariableTarget_ReturnValue: {
                    TemLangStringAppendFormat(s, "return !%s;", s1.buffer);
                } break;
                case VariableTarget_Variable: {
                    TemLangStringAppendFormat(
                      s, "%s = !%s;", target.name->buffer, s1.buffer);
                } break;
                case VariableTarget_None:
                    TemLangStringAppendFormat(s, "!%s", s1.buffer);
                default:
                    break;
            }
        } else if (realValue != NULL) {
            MAKE_LEFT_AND_RIGHT((*left), (*right));
            const char* c = realValue == left ? ls.buffer : rs.buffer;
            switch (target.type) {
                case VariableTarget_ReturnValue: {
                    TemLangStringAppendFormat(s, "return !%s;", c);
                } break;
                case VariableTarget_Variable: {
                    TemLangStringAppendFormat(
                      s, "%s = !%s;", target.name->buffer, c);
                } break;
                case VariableTarget_None:
                    TemLangStringAppendFormat(s, "!%s", c);
                default:
                    break;
            }
            TemLangStringFree(&ls);
            TemLangStringFree(&rs);
        }
        TemLangStringFree(&s1);
    }
    return s;
}

static inline TemLangString
CopmilerGetComparisonExpression(const State* state,
                                const Allocator* allocator,
                                const VariableTarget target,
                                const Value* left,
                                const Value* right,
                                const Expression* e)
{
    TemLangString s = TemLangStringCreate("", allocator);
    if (left->type == right->type) {
        switch (left->type) {
            case ValueType_Null:
            case ValueType_Number:
            case ValueType_Boolean:
            case ValueType_Enum: {
                TemLangString s1 = { .allocator = allocator };
                TemLangString s2 = { .allocator = allocator };

                if (!GetSimpleExpressionName(state, allocator, e->left, &s1) ||
                    !GetSimpleExpressionName(state, allocator, e->right, &s2)) {
                    MAKE_LEFT_AND_RIGHT((*left), (*right));
                    TemLangStringCopy(&s1, &ls, allocator);
                    TemLangStringCopy(&s2, &rs, allocator);
                    TemLangStringFree(&ls);
                    TemLangStringFree(&rs);
                }

                switch (target.type) {
                    case VariableTarget_ReturnValue: {
                        TemLangStringAppendFormat(
                          s,
                          "return %s %s %s;",
                          s1.buffer,
                          ComparisonOperatorToChars(e->op.comparisonOperator),
                          s2.buffer);
                    } break;
                    case VariableTarget_Variable: {
                        TemLangStringAppendFormat(
                          s,
                          "%s = %s %s %s;",
                          target.name->buffer,
                          s1.buffer,
                          ComparisonOperatorToChars(e->op.comparisonOperator),
                          s2.buffer);
                    } break;
                    case VariableTarget_None:
                    default: {
                        TemLangStringAppendFormat(
                          s,
                          "%s %s %s",
                          s1.buffer,
                          ComparisonOperatorToChars(e->op.comparisonOperator),
                          s2.buffer);
                    } break;
                }
                TemLangStringFree(&s1);
                TemLangStringFree(&s2);
                return s;
            } break;
            case ValueType_String: {
                MAKE_LEFT_AND_RIGHT((*left), (*right));

                switch (target.type) {
                    case VariableTarget_ReturnValue: {
                        TemLangStringAppendFormat(
                          s,
                          "return TemLangStringCompare(&%s,&%s) == %s;",
                          ls.buffer,
                          rs.buffer,
                          ComparisonOperatorToString(e->op.comparisonOperator));
                    } break;
                    case VariableTarget_Variable: {
                        TemLangStringAppendFormat(
                          s,
                          "%s = TemLangStringCompare(&%s,&%s) == %s;",
                          target.name->buffer,
                          ls.buffer,
                          rs.buffer,
                          ComparisonOperatorToString(e->op.comparisonOperator));
                    } break;
                    case VariableTarget_None:
                    default: {
                        TemLangStringAppendFormat(
                          s,
                          "TemLangStringCompare(&%s,&%s) == %s",
                          ls.buffer,
                          rs.buffer,
                          ComparisonOperatorToString(e->op.comparisonOperator));
                    } break;
                }

                TemLangStringFree(&ls);
                TemLangStringFree(&rs);
                return s;
            } break;
            default:
                break;
        }
    }
    switch (target.type) {
        case VariableTarget_ReturnValue:
            TemLangStringAppendChars(&s, "return false;");
            break;
        case VariableTarget_Variable: {
            TemLangStringAppendFormat(s, "%s = false;", target.name->buffer);
        } break;
        case VariableTarget_None:
        default:
            TemLangStringAppendChars(&s, "false");
            break;
    }
    return s;
}

static inline TemLangString
CompilerGetBranch(const State* state,
                  const Allocator* allocator,
                  const VariableTarget resultTarget,
                  const Branch* branch)
{
    TemLangString s = TemLangStringCreate("", allocator);
    switch (branch->type) {
        case MatchBranchType_Expression: {
            Value value = { 0 };
            EvaluateExpression(&branch->expression, state, &value, allocator);
            TemLangString s2 = CompilerGetExpression(
              state, allocator, resultTarget, &value, &branch->expression);
            TemLangStringAppend(&s, &s2);
            TemLangStringFree(&s2);
            ValueFree(&value);
        } break;
        case MatchBranchType_Instructions: {
            const VariableTarget target = { .type = VariableTarget_None };
            State temp = { 0 };
            StateCopy(&temp, state, allocator);
            CompileInstructions(
              &branch->instructions, allocator, target, &temp, &s);
            COMPILE_COPIED_STATE_CLEANUP((*state), temp, s);
        } break;
        case MatchBranchType_None:
            break;
        default:
            TemLangError("Falied to compile branch of type '%s'",
                         MatchBranchTypeToString(branch->type));
            break;
    }
    return s;
}

static inline TemLangString
CompilerGetMatchBranch(State* state,
                       const Allocator* allocator,
                       const VariableTarget resultTarget,
                       const TemLangString* targetName,
                       const Value* value,
                       const MatchBranch* branch)
{
    GET_RETURN_VALUE_SCOPE({ return TemLangStringCreate("", allocator); });
    TemLangString s =
      TemLangStringCreate("{\n// Start of match branch\n", allocator);
    Value matcher = { 0 };
    EvaluateExpression(&branch->matcher, state, &matcher, allocator);
    if (value->type == ValueType_Enum && matcher.type == ValueType_String) {
        TemLangString s2 =
          CompilerGetBranch(state, allocator, resultTarget, &branch->branch);
        TemLangStringAppendFormat(s,
                                  "if(%s == %s_%s) { %s goto scope%zu;}",
                                  targetName->buffer,
                                  value->enumValue.name.buffer,
                                  matcher.string.buffer,
                                  s2.buffer,
                                  returnScope);
        TemLangStringFree(&s2);
        goto end;
    }

    {
        Expression left = { 0 };
        left.type = ExpressionType_UnaryVariable;
        TemLangStringCopy(&left.identifier, targetName, allocator);

        Expression right = { 0 };
        right.type = ExpressionType_UnaryVariable;
        {
            TemLangStringCreateFormat(rname, allocator, "value%zu", variableId);
            ++variableId;
            right.identifier = rname;
        }
        State temp = { 0 };
        temp.atoms.allocator = allocator;
        temp.parent = state;
        if (!StateAddValue(&temp, &right.identifier, &matcher)) {
            TemLangError("Compiler error! Check (%s:%zu)", __FILE__, __LINE__);
        }
        {
            TemLangString s2 = CompilerAssignValue(&temp,
                                                   &right.identifier,
                                                   allocator,
                                                   &matcher,
                                                   &branch->matcher,
                                                   true);
            TemLangStringAppend(&s, &s2);
            TemLangStringFree(&s2);
        }

        Value value = { 0 };
        Expression e = { .type = ExpressionType_Binary };
        e.left = &left;
        e.right = &right;

        e.op.type = OperatorType_Comparison;
        e.op.comparisonOperator = ComparisonOperator_EqualTo;
        EvaluateExpression(&e, &temp, &value, allocator);

        {
            TemLangStringCreateFormat(name, allocator, "result%zu", variableId);
            ++variableId;
            TemLangString s2 =
              CompilerAssignValue(&temp, &name, allocator, &value, &e, true);
            TemLangStringAppendFormat(s, "%sif(%s){\n", s2.buffer, name.buffer);
            TemLangStringFree(&s2);
            TemLangStringFree(&name);
        }
        ValueFree(&value);
        ExpressionFree(&left);
        ExpressionFree(&right);
        StateFree(&temp);
    }
    {
        TemLangString s2 =
          CompilerGetBranch(state, allocator, resultTarget, &branch->branch);
        TemLangStringAppendFormat(
          s, "%s goto scope%zu; \n}", s2.buffer, returnScope);
        TemLangStringFree(&s2);
    }
end:
    TemLangStringAppendChars(&s, "\n//End of match branch\n}");
    ValueFree(&matcher);
    return s;
}

#define CompilerGetMethod(left, right)                                         \
    Expression tempLeft = { .type = ExpressionType_UnaryValue,                 \
                            .value = *left.variantValue.value };               \
    Expression tempRight = { .type = ExpressionType_UnaryValue,                \
                             .value = right };                                 \
    Expression tempE = { 0 };                                                  \
    tempE.type = ExpressionType_Binary;                                        \
    tempE.left = &tempLeft;                                                    \
    tempE.right = &tempRight;                                                  \
    tempE.op.type = OperatorType_Function;                                     \
    tempE.op.functionCall = *functionName;                                     \
    TemLangString c =                                                          \
      CompilerGetExpression(state, allocator, target, value, &tempE);

static inline TemLangString
CompilerGetNumberExpression(const State* state,
                            const Allocator* allocator,
                            const VariableTarget target,
                            const Value* left,
                            const Value* right,
                            const Value* realValue,
                            const Expression* e);

static inline TemLangString
CompilerGetOperatorExpression(const State* state,
                              const Allocator* allocator,
                              const VariableTarget target,
                              const Value* left,
                              const Value* right,
                              const Value* value,
                              const Expression* e)
{
    TemLangString s = { .allocator = allocator };
    switch (e->op.getOperator) {
        case GetOperator_MakeList:
            // Type coercsion operator. No code to generate
            break;
        case GetOperator_Type:
            // Type checking operator. No code to generate.
            break;
        case GetOperator_Length: {
            const Value* targetValue = getUnaryValue(left, right);
            if (targetValue == NULL) {
                break;
            }
            const Expression* targetE =
              targetValue == left ? e->left : e->right;
            TemLangString lengthS = { .allocator = allocator };
            if (targetE->type == ExpressionType_UnaryVariable) {
                switch (targetValue->type) {
                    case ValueType_List:
                        TemLangStringAppendFormat(
                          lengthS, "%s.used", targetE->identifier.buffer);
                        break;
                    case ValueType_Enum:
                        TemLangStringAppendFormat(
                          lengthS,
                          "%s_Length",
                          targetValue->enumValue.name.buffer);
                        break;
                    default:
                        break;
                }
            } else {
                switch (targetValue->type) {
                    case ValueType_List:
                        lengthS = CompilerGetFullVariableName(
                          targetE, state, allocator);
                        TemLangStringAppendChars(&lengthS, ".used");
                        break;
                    case ValueType_Enum:
                        TemLangStringAppendFormat(
                          lengthS,
                          "%s_Length",
                          targetValue->enumValue.name.buffer);
                        break;
                    default:
                        break;
                }
            }
            switch (target.type) {
                case VariableTarget_ReturnValue:
                    TemLangStringAppendFormat(s, "return %s;", lengthS.buffer);
                    break;
                case VariableTarget_Variable:
                    TemLangStringAppendFormat(
                      s, "%s = %s;", target.name->buffer, lengthS.buffer);
                    break;
                default:
                    TemLangStringAppendFormat(s, "%s;", lengthS.buffer);
                    break;
            }
            TemLangStringFree(&lengthS);
        } break;
        case GetOperator_Member: {
            TemLangString containerName =
              CompilerGetFullVariableName(e->left, state, allocator);
            switch (right->type) {
                case ValueType_Enum: {
                    if (e->right->type == ExpressionType_UnaryVariable) {
                        switch (target.type) {
                            case VariableTarget_ReturnValue: {
                                TemLangStringAppendFormat(
                                  s,
                                  "return %s[%s];",
                                  containerName.buffer,
                                  e->right->identifier.buffer);
                            } break;
                            case VariableTarget_Variable: {
                                TemLangStringCreateFormat(
                                  varName,
                                  allocator,
                                  "%s[%s]",
                                  containerName.buffer,
                                  e->right->identifier.buffer);
                                State temp = { 0 };
                                temp.parent = state;
                                temp.atoms.allocator = allocator;
                                if (!StateAddValue(&temp, &varName, value)) {
                                    TemLangError("Compiler error! "
                                                 "Check (%s:%zu)",
                                                 __FILE__,
                                                 __LINE__);
                                }
                                Expression tempE = {
                                    .type = ExpressionType_UnaryVariable,
                                    .identifier = varName
                                };
                                TemLangString s1 =
                                  CompilerAssignValue(&temp,
                                                      target.name,
                                                      allocator,
                                                      value,
                                                      &tempE,
                                                      false);
                                TemLangStringFree(&varName);
                                TemLangStringAppend(&s, &s1);
                                TemLangStringFree(&s1);
                                StateFree(&temp);
                            } break;
                            default:
                                TemLangStringAppendFormat(
                                  s,
                                  "%s[%s];",
                                  containerName.buffer,
                                  e->right->identifier.buffer);
                                break;
                        }
                    } else {
                        switch (target.type) {
                            case VariableTarget_ReturnValue: {
                                TemLangStringAppendFormat(
                                  s,
                                  "return %s[%s_%s];",
                                  containerName.buffer,
                                  right->enumValue.name.buffer,
                                  right->enumValue.value.buffer);
                            } break;
                            case VariableTarget_Variable: {
                                TemLangStringCreateFormat(
                                  varName,
                                  allocator,
                                  "%s[%s_%s]",
                                  containerName.buffer,
                                  right->enumValue.name.buffer,
                                  right->enumValue.value.buffer);
                                State temp = { 0 };
                                temp.parent = state;
                                temp.atoms.allocator = allocator;
                                if (!StateAddValue(&temp, &varName, value)) {
                                    TemLangError("Compiler error! "
                                                 "Check (%s:%zu)",
                                                 __FILE__,
                                                 __LINE__);
                                }
                                Expression tempE = {
                                    .type = ExpressionType_UnaryVariable,
                                    .identifier = varName
                                };
                                Value tempValue = { .type = ValueType_Number };
                                TemLangString s1 =
                                  CompilerAssignValue(&temp,
                                                      target.name,
                                                      allocator,
                                                      &tempValue,
                                                      &tempE,
                                                      false);
                                TemLangStringFree(&varName);
                                TemLangStringAppend(&s, &s1);
                                TemLangStringFree(&s1);
                                StateFree(&temp);
                            } break;
                            default:
                                TemLangStringAppendFormat(
                                  s,
                                  "%s[%s_%s];",
                                  containerName.buffer,
                                  right->enumValue.name.buffer,
                                  right->enumValue.value.buffer);
                                break;
                        }
                    }
                } break;
                case ValueType_Number: {
                    if (e->right->type == ExpressionType_UnaryVariable) {
                        switch (target.type) {
                            case VariableTarget_ReturnValue: {
                                TemLangStringAppendFormat(
                                  s,
                                  "return %s[%s];",
                                  containerName.buffer,
                                  e->right->identifier.buffer);
                            } break;
                            case VariableTarget_Variable: {
                                TemLangStringCreateFormat(
                                  varName,
                                  allocator,
                                  "%s[%s]",
                                  containerName.buffer,
                                  e->right->identifier.buffer);
                                State temp = { 0 };
                                temp.parent = state;
                                temp.atoms.allocator = allocator;
                                if (!StateAddValue(&temp, &varName, value)) {
                                    TemLangError("Compiler error! "
                                                 "Check (%s:%zu)",
                                                 __FILE__,
                                                 __LINE__);
                                }
                                Expression tempE = {
                                    .type = ExpressionType_UnaryVariable,
                                    .identifier = varName
                                };
                                TemLangString s1 =
                                  CompilerAssignValue(&temp,
                                                      target.name,
                                                      allocator,
                                                      value,
                                                      &tempE,
                                                      false);
                                TemLangStringFree(&varName);
                                TemLangStringAppend(&s, &s1);
                                TemLangStringFree(&s1);
                                StateFree(&temp);
                            } break;
                            default:
                                TemLangStringAppendFormat(
                                  s,
                                  "%s[%s];",
                                  containerName.buffer,
                                  e->right->identifier.buffer);
                                break;
                        }
                    } else {
                        uint64_t index;
                        ValueToIndex(right, &index);
                        switch (target.type) {
                            case VariableTarget_ReturnValue: {
                                TemLangStringAppendFormat(s,
                                                          "return %s[%" PRIu64
                                                          "];",
                                                          containerName.buffer,
                                                          index);
                            } break;
                            case VariableTarget_Variable: {
                                TemLangStringCreateFormat(varName,
                                                          allocator,
                                                          "%s[%" PRIu64 "]",
                                                          containerName.buffer,
                                                          index);
                                State temp = { 0 };
                                temp.parent = state;
                                temp.atoms.allocator = allocator;
                                if (!StateAddValue(&temp, &varName, value)) {
                                    TemLangError("Compiler error! "
                                                 "Check (%s:%zu)",
                                                 __FILE__,
                                                 __LINE__);
                                }
                                Expression tempE = {
                                    .type = ExpressionType_UnaryVariable,
                                    .identifier = varName
                                };
                                TemLangString s1 =
                                  CompilerAssignValue(&temp,
                                                      target.name,
                                                      allocator,
                                                      value,
                                                      &tempE,
                                                      false);
                                TemLangStringFree(&varName);
                                TemLangStringAppend(&s, &s1);
                                TemLangStringFree(&s1);
                                StateFree(&temp);
                            } break;
                            default:
                                TemLangStringAppendFormat(s,
                                                          "%s[%" PRIu64 "];",
                                                          containerName.buffer,
                                                          index);
                                break;
                        }
                    }
                } break;
                default: {
                    switch (target.type) {
                        case VariableTarget_ReturnValue: {
                            TemLangStringAppendFormat(s,
                                                      "return %s.%s;",
                                                      containerName.buffer,
                                                      right->string.buffer);
                        } break;
                        case VariableTarget_Variable: {
                            TemLangStringCreateFormat(varName,
                                                      allocator,
                                                      "%s.%s",
                                                      containerName.buffer,
                                                      right->string.buffer);
                            State temp = { 0 };
                            temp.parent = state;
                            temp.atoms.allocator = allocator;
                            if (!StateAddValue(&temp, &varName, value)) {
                                TemLangError("Compiler error! "
                                             "Check (%s:%zu)",
                                             __FILE__,
                                             __LINE__);
                            }
                            Expression tempE = { .type =
                                                   ExpressionType_UnaryVariable,
                                                 .identifier = varName };
                            TemLangString s1 = CompilerAssignValue(&temp,
                                                                   target.name,
                                                                   allocator,
                                                                   value,
                                                                   &tempE,
                                                                   false);
                            TemLangStringFree(&varName);
                            TemLangStringAppend(&s, &s1);
                            TemLangStringFree(&s1);
                            StateFree(&temp);
                        } break;
                        default:
                            TemLangStringAppendFormat(s,
                                                      "%s.%s;",
                                                      containerName.buffer,
                                                      right->string.buffer);
                            break;
                    }
                } break;
            }
            TemLangStringFree(&containerName);
        } break;
        default:
            TemLangError("Failed to compile get operator '%s'",
                         GetOperatorToString(e->op.getOperator));
            break;
    }
    return s;
}

static inline TemLangString
CompilerGetExpression(const State* state,
                      const Allocator* allocator,
                      const VariableTarget target,
                      const Value* value,
                      const Expression* e)
{
    TemLangString s = TemLangStringCreate("", allocator);
    switch (e->type) {
        case ExpressionType_Nullary: {
            switch (target.type) {
                case VariableTarget_Variable: {
                    TemLangStringAppendFormat(
                      s, "%s = NULL;", target.name->buffer);
                } break;
                case VariableTarget_ReturnValue:
                    TemLangStringAppendChars(&s, "return NULL;");
                    break;
                case VariableTarget_None:
                default:
                    TemLangStringAppendChars(&s, "NULL");
                    break;
            }
        } break;
        case ExpressionType_UnaryValue: {
            switch (value->type) {
                case ValueType_String: {
                    switch (target.type) {
                        case VariableTarget_Variable: {
                            TemLangStringAppendFormat(
                              s,
                              "TemLangStringFree(&%s); %s = "
                              "TemLangStringCreate(\"%s\", "
                              "currentAllocator);",
                              target.name->buffer,
                              target.name->buffer,
                              e->value.string.buffer);
                        } break;
                        case VariableTarget_ReturnValue: {
                            TemLangStringAppendFormat(
                              s,
                              "return TemLangStringCreate(\"%s\", "
                              "currentAllocator);",
                              e->value.string.buffer);
                        } break;
                        case VariableTarget_None:
                        default: {
                            TemLangStringAppendFormat(
                              s,
                              "TemLangStringCreate(\"%s\", "
                              "currentAllocator)",
                              e->value.string.buffer);
                        } break;
                    }
                } break;
                case ValueType_List: {
                    char buffer[256] = { 0 };
                    TemLangString tempName = { .buffer = buffer,
                                               .size = sizeof(buffer),
                                               .used = 0,
                                               .allocator = NULL };
                    const VariableTarget t = { .type = VariableTarget_Variable,
                                               .name = &tempName };
                    const float isCGLM =
                      useCGLM && value->list.isArray &&
                      value->list.exampleValue->type == ValueType_Number &&
                      RangeToCType(
                        &value->list.exampleValue->rangedNumber.range) ==
                        CType_f32;
                    if (isCGLM && e->value.list.values.used == 9) {
                        for (size_t i = 0; i < 3; ++i) {
                            for (size_t j = 0; j < 3; ++j) {
                                const Value* value =
                                  &e->value.list.values.buffer[i * 3 + j];
                                tempName.used = snprintf(
                                  buffer,
                                  sizeof(buffer),
                                  "%s%s[%zu][%zu]",
                                  target.name->buffer,
                                  e->value.list.isArray ? "" : ".buffer",
                                  i,
                                  j);
                                Expression temp = { 0 };
                                temp.type = ExpressionType_UnaryValue;
                                temp.value = *value;
                                TemLangString a = CompilerGetExpression(
                                  state, allocator, t, value, &temp);
                                TemLangStringAppend(&s, &a);
                                TemLangStringFree(&a);
                            }
                        }
                    } else if (isCGLM && e->value.list.values.used == 16) {
                        for (size_t i = 0; i < 4; ++i) {
                            for (size_t j = 0; j < 4; ++j) {
                                const Value* value =
                                  &e->value.list.values.buffer[i * 4 + j];
                                tempName.used = snprintf(
                                  buffer,
                                  sizeof(buffer),
                                  "%s%s[%zu][%zu]",
                                  target.name->buffer,
                                  e->value.list.isArray ? "" : ".buffer",
                                  i,
                                  j);
                                Expression temp = { 0 };
                                temp.type = ExpressionType_UnaryValue;
                                temp.value = *value;
                                TemLangString a = CompilerGetExpression(
                                  state, allocator, t, value, &temp);
                                TemLangStringAppend(&s, &a);
                                TemLangStringFree(&a);
                            }
                        }
                    } else {
                        for (size_t i = 0; i < e->value.list.values.used; ++i) {
                            const Value* value =
                              &e->value.list.values.buffer[i];
                            tempName.used =
                              snprintf(buffer,
                                       sizeof(buffer),
                                       "%s%s[%zu]",
                                       target.name->buffer,
                                       e->value.list.isArray ? "" : ".buffer",
                                       i);
                            Expression temp = { 0 };
                            temp.type = ExpressionType_UnaryValue;
                            temp.value = *value;
                            TemLangString a = CompilerGetExpression(
                              state, allocator, t, value, &temp);
                            TemLangStringAppend(&s, &a);
                            TemLangStringFree(&a);
                        }
                    }
                } break;
                case ValueType_Struct: {
                    for (size_t i = 0; i < value->structValues->used; ++i) {
                        const NamedValue* nv = &value->structValues->buffer[i];
                        switch (target.type) {
                            case VariableTarget_Variable: {
                                Expression t1 = { 0 };
                                t1.type = ExpressionType_UnaryVariable;
                                TemLangStringCreateFormat(a,
                                                          allocator,
                                                          "%s.%s",
                                                          e->identifier.buffer,
                                                          nv->name.buffer);
                                t1.identifier = a;
                                TemLangStringCreateFormat(b,
                                                          allocator,
                                                          "%s.%s",
                                                          target.name->buffer,
                                                          nv->name.buffer);
                                const VariableTarget tempTarget = {
                                    .type = VariableTarget_Variable, .name = &b
                                };
                                TemLangString c =
                                  CompilerGetExpression(state,
                                                        allocator,
                                                        tempTarget,
                                                        &nv->value,
                                                        &t1);
                                TemLangStringAppend(&s, &c);
                                TemLangStringFree(&a);
                                TemLangStringFree(&b);
                                TemLangStringFree(&c);
                            } break;
                            case VariableTarget_ReturnValue:
                            case VariableTarget_None:
                            default:
                                break;
                        }
                    }
                } break;
                case ValueType_Variant: {
                    TemLangString a = CompilerGetValue(&e->value, allocator);
                    switch (target.type) {
                        case VariableTarget_Variable: {
                            TemLangStringAppendFormat(
                              s,
                              "%sCopy(&%s, %s, currentAllocator);",
                              value->variantValue.name.buffer,
                              target.name->buffer,
                              a.buffer);
                        } break;
                        case VariableTarget_ReturnValue: {
                            TemLangStringAppendFormat(
                              s, "return %s;", a.buffer);
                        } break;
                        case VariableTarget_None:
                        default: {
                            TemLangStringAppendFormat(s, "%s", a.buffer);
                        } break;
                    }
                    TemLangStringFree(&a);
                } break;
                default: {
                    TemLangString a = CompilerGetValue(&e->value, allocator);
                    switch (target.type) {
                        case VariableTarget_Variable: {
                            TemLangStringAppendFormat(
                              s, "%s = %s;", target.name->buffer, a.buffer);
                        } break;
                        case VariableTarget_ReturnValue: {
                            TemLangStringAppendFormat(
                              s, "return %s;", a.buffer);
                        } break;
                        case VariableTarget_None:
                        default: {
                            TemLangStringAppendFormat(s, "%s", a.buffer);
                        } break;
                    }
                    TemLangStringFree(&a);
                } break;
            }
        } break;
        case ExpressionType_UnaryVariable: {
            {
                const StateFindArgs args = { .log = false,
                                             .searchParent = true };
                const Atom* atom = StateFindAtomConst(
                  state, &e->identifier, AtomType_Variable, args);
                if (atom != NULL &&
                    atom->variable.type == VariableType_Constant) {
                    Expression tempE = { .type = ExpressionType_UnaryValue,
                                         .value = *value };
                    return CompilerGetExpression(
                      state, allocator, target, value, &tempE);
                }
            }
            switch (value->type) {
                case ValueType_List: {
                    if (target.type != VariableTarget_Variable) {
                        TemLangError("Cannot compile an expression array/list "
                                     "if it isn't named from a variable");
                        break;
                    }
                    State temp = { 0 };
                    temp.atoms.allocator = allocator;
                    temp.parent = state;
                    const bool isCGLM =
                      useCGLM &&
                      value->list.exampleValue->type == ValueType_Number &&
                      RangeToCType(
                        &value->list.exampleValue->rangedNumber.range) ==
                        CType_f32;
                    if (value->list.isArray) {
                        if (isCGLM) {
                            switch (value->list.values.used) {
                                case 2:
                                    TemLangStringAppendFormat(
                                      s,
                                      "glm_vec2_copy(%s, %s);",
                                      e->identifier.buffer,
                                      target.name->buffer);
                                    goto exit;
                                case 3:
                                    TemLangStringAppendFormat(
                                      s,
                                      "glm_vec3_copy(%s, %s);",
                                      e->identifier.buffer,
                                      target.name->buffer);
                                    goto exit;
                                case 4:
                                    TemLangStringAppendFormat(
                                      s,
                                      "glm_vec4_copy(%s, %s);",
                                      e->identifier.buffer,
                                      target.name->buffer);
                                    goto exit;
                                case 9:
                                    TemLangStringAppendFormat(
                                      s,
                                      "glm_mat3_copy(%s, %s);",
                                      e->identifier.buffer,
                                      target.name->buffer);
                                    goto exit;
                                case 16:
                                    TemLangStringAppendFormat(
                                      s,
                                      "glm_mat4_copy(%s, %s);",
                                      e->identifier.buffer,
                                      target.name->buffer);
                                    goto exit;
                                default:
                                    break;
                            }
                        }
                        TemLangString name = TemLangStringCreate("", allocator);
                        TemLangString targetName =
                          TemLangStringCreate("", allocator);
                        TemLangStringCreateFormat(
                          indexName, allocator, "i%zu", variableId);
                        ++variableId;
                        TemLangStringAppendFormat(
                          s,
                          "for(size_t %s = 0; %s < %u; ++%s){ ",
                          indexName.buffer,
                          indexName.buffer,
                          value->list.values.used,
                          indexName.buffer);
                        TemLangStringAppendFormat(name,
                                                  "%s[%s]",
                                                  target.name->buffer,
                                                  indexName.buffer);
                        TemLangStringAppendFormat(targetName,
                                                  "%s[%s]",
                                                  e->identifier.buffer,
                                                  indexName.buffer);
                        if (!StateAddValue(
                              &temp, &name, value->list.exampleValue)) {
                            TemLangError("Compiler error! Check (%s:%zu)",
                                         __FILE__,
                                         __LINE__);
                        }
                        Expression tempE = { .type =
                                               ExpressionType_UnaryVariable,
                                             .identifier = targetName };
                        TemLangString b =
                          CompilerAssignValue(&temp,
                                              &name,
                                              allocator,
                                              value->list.exampleValue,
                                              &tempE,
                                              false);
                        TemLangStringAppend(&s, &b);
                        TemLangStringFree(&b);
                        COMPILE_COPIED_STATE_CLEANUP((*state), temp, s);
                        TemLangStringAppendChar(&s, '}');
                        TemLangStringFree(&targetName);
                        TemLangStringFree(&name);
                        TemLangStringFree(&indexName);
                    } else {
                        TemLangString k =
                          CompilerDeclareValueType(NULL,
                                                   state,
                                                   allocator,
                                                   value->list.exampleValue,
                                                   false);
                        TemLangStringAppendFormat(
                          s,
                          "%sListCopy(&%s, &%s, currentAllocator);",
                          k.buffer,
                          target.name->buffer,
                          e->identifier.buffer);
                        TemLangStringFree(&k);
                        COMPILE_COPIED_STATE_CLEANUP((*state), temp, s);
                        // TemLangStringAppendChar(&s, '}');
                    }
                } break;
                case ValueType_Struct:
                    for (size_t i = 0; i < value->structValues->used; ++i) {
                        const NamedValue* nv = &value->structValues->buffer[i];
                        switch (target.type) {
                            case VariableTarget_Variable: {
                                Expression t1 = { 0 };
                                t1.type = ExpressionType_UnaryVariable;
                                TemLangStringCreateFormat(a,
                                                          allocator,
                                                          "%s.%s",
                                                          e->identifier.buffer,
                                                          nv->name.buffer);
                                t1.identifier = a;
                                TemLangStringCreateFormat(b,
                                                          allocator,
                                                          "%s.%s",
                                                          target.name->buffer,
                                                          nv->name.buffer);
                                const VariableTarget tempTarget = {
                                    .type = VariableTarget_Variable, .name = &b
                                };
                                TemLangString c =
                                  CompilerGetExpression(state,
                                                        allocator,
                                                        tempTarget,
                                                        &nv->value,
                                                        &t1);
                                TemLangStringAppend(&s, &c);
                                TemLangStringFree(&a);
                                TemLangStringFree(&b);
                                TemLangStringFree(&c);
                            } break;
                            case VariableTarget_ReturnValue:
                            case VariableTarget_None:
                            default:
                                break;
                        }
                    }
                    break;
                default: {
                    switch (target.type) {
                        case VariableTarget_Variable: {
                            switch (value->type) {
                                case ValueType_String:
                                case ValueType_Variant:
                                case ValueType_Struct:
                                case ValueType_List: {
                                    TemLangString copyName =
                                      ValueCopyName(value, allocator);
                                    TemLangStringAppendFormat(
                                      s,
                                      "%s(&%s, &%s, currentAllocator);",
                                      copyName.buffer,
                                      target.name->buffer,
                                      e->identifier.buffer);
                                    TemLangStringFree(&copyName);
                                } break;
                                default: {
                                    TemLangStringAppendFormat(
                                      s,
                                      "%s = %s;",
                                      target.name->buffer,
                                      e->identifier.buffer);
                                } break;
                            }
                        } break;
                        case VariableTarget_ReturnValue: {
                            TemLangStringAppendFormat(
                              s, "return %s;", e->identifier.buffer);
                        } break;
                        case VariableTarget_None: {
                            TemLangStringAppendFormat(
                              s, "%s", e->identifier.buffer);
                        } break;
                        default:
                            break;
                    }
                } break;
            }
        } break;
        case ExpressionType_UnaryStruct: {
            if (target.type != VariableTarget_Variable) {
                break;
            }
            TemLangStringAppendChars(&s, "{\n//Start of unary struct\n");
            State temp = { 0 };
            temp.parent = state;
            temp.atoms.allocator = allocator;
            if (CompileInstructions(
                  &e->instructions, allocator, target, &temp, &s)) {
                for (size_t i = 0; i < value->structValues->used; ++i) {
                    const NamedValue* nv = &value->structValues->buffer[i];
                    Expression t1 = { 0 };
                    t1.type = ExpressionType_UnaryVariable;
                    TemLangStringCreateFormat(
                      a, allocator, "%s", nv->name.buffer);
                    t1.identifier = a;
                    TemLangStringCreateFormat(b,
                                              allocator,
                                              "%s.%s",
                                              target.name->buffer,
                                              nv->name.buffer);
                    const VariableTarget tempTarget = {
                        .type = VariableTarget_Variable, .name = &b
                    };
                    TemLangString c = CompilerGetExpression(
                      state, allocator, tempTarget, &nv->value, &t1);
                    TemLangStringAppend(&s, &c);
                    TemLangStringFree(&a);
                    TemLangStringFree(&b);
                    TemLangStringFree(&c);
                }
            }
            COMPILE_STATE_CLEANUP(temp, s);
            TemLangStringAppendChars(&s, "\n}");
        } break;
        case ExpressionType_UnaryList: {
            if (target.type != VariableTarget_Variable) {
                TemLangError("Cannot compile an expression array/list "
                             "if it isn't named from a variable");
                break;
            }
            const bool isCGLM =
              useCGLM && value->list.isArray &&
              value->list.exampleValue->type == ValueType_Number &&
              RangeToCType(&value->list.exampleValue->rangedNumber.range) ==
                CType_f32;
            if (isCGLM) {
                switch (value->list.values.used) {
                    case 9:
                        for (size_t i = 0; i < 3; ++i) {
                            for (size_t j = 0; j < 3; ++j) {
                                TemLangStringCreateFormat(name,
                                                          allocator,
                                                          "%s[%zu][%zu]",
                                                          target.name->buffer,
                                                          i,
                                                          j);
                                VariableTarget newTarget = {
                                    .type = VariableTarget_Variable,
                                    .name = &name
                                };
                                TemLangString s1 = CompilerGetExpression(
                                  state,
                                  allocator,
                                  newTarget,
                                  &value->list.values.buffer[i * 3 + j],
                                  &e->expressions.buffer[i * 3 + j]);
                                TemLangStringAppend(&s, &s1);
                                TemLangStringFree(&name);
                                TemLangStringFree(&s1);
                            }
                        }
                        goto unaryListDone;
                    case 16:
                        for (size_t i = 0; i < 4; ++i) {
                            for (size_t j = 0; j < 4; ++j) {
                                TemLangStringCreateFormat(name,
                                                          allocator,
                                                          "%s[%zu][%zu]",
                                                          target.name->buffer,
                                                          i,
                                                          j);
                                VariableTarget newTarget = {
                                    .type = VariableTarget_Variable,
                                    .name = &name
                                };
                                TemLangString s1 = CompilerGetExpression(
                                  state,
                                  allocator,
                                  newTarget,
                                  &value->list.values.buffer[i * 4 + j],
                                  &e->expressions.buffer[i * 4 + j]);
                                TemLangStringAppend(&s, &s1);
                                TemLangStringFree(&name);
                                TemLangStringFree(&s1);
                            }
                        }
                        goto unaryListDone;
                    default:
                        break;
                }
            }
            for (size_t i = 0; i < value->list.values.used; ++i) {
                if (value->list.isArray) {
                    TemLangStringCreateFormat(
                      name, allocator, "%s[%zu]", target.name->buffer, i);
                    VariableTarget newTarget = { .type =
                                                   VariableTarget_Variable,
                                                 .name = &name };
                    TemLangString s1 =
                      CompilerGetExpression(state,
                                            allocator,
                                            newTarget,
                                            &value->list.values.buffer[i],
                                            &e->expressions.buffer[i]);
                    TemLangStringAppend(&s, &s1);
                    TemLangStringFree(&name);
                    TemLangStringFree(&s1);
                } else {
                    TemLangStringCreateFormat(
                      s1, allocator, "temp%zu", variableId);
                    ++variableId;
                    TemLangString s2 =
                      CompilerAssignValue(state,
                                          &s1,
                                          allocator,
                                          &value->list.values.buffer[i],
                                          &e->expressions.buffer[i],
                                          true);
                    TemLangString s3 = CompilerDeclareValueType(
                      NULL, state, allocator, value->list.exampleValue, false);
                    TemLangStringAppendFormat(
                      s,
                      "{\n %s %sListAppend(&%s, &%s); \n}",
                      s2.buffer,
                      s3.buffer,
                      target.name->buffer,
                      s1.buffer);
                    TemLangStringFree(&s1);
                    TemLangStringFree(&s3);
                    TemLangStringFree(&s2);
                }
            }
        unaryListDone:
            break;
        } break;
        case ExpressionType_UnaryScope: {
            const size_t targetScopeNumber = scopeNumber;
            ++scopeNumber;
            size_tListAppend(&returnValueScopes, &targetScopeNumber);
            TemLangStringAppendFormat(
              s, "{\n//Start of scope%zu\n", targetScopeNumber);
            State temp = { 0 };
            temp.parent = state;
            temp.atoms.allocator = allocator;
            CompileInstructions(&e->instructions, allocator, target, &temp, &s);
            COMPILE_STATE_CLEANUP(temp, s);
            TemLangStringAppendFormat(
              s,
              "goto scope%zu;scope%zu:\n(void)NULL;\n//End of scope%zu\n}",
              targetScopeNumber,
              targetScopeNumber,
              targetScopeNumber);
            size_tListPop(&returnValueScopes);
        } break;
        case ExpressionType_UnaryMatch: {
            const size_t targetScopeNumber = scopeNumber;
            ++scopeNumber;
            size_tListAppend(&returnValueScopes, &targetScopeNumber);
            TemLangStringAppendFormat(
              s, "{\n//Start of scope%zu\n", targetScopeNumber);

            State temp = { 0 };
            StateCopy(&temp, state, allocator);

            Value matcher = { 0 };
            EvaluateExpression(
              &e->matchExpression->matcher, state, &matcher, allocator);
            switch (matcher.type) {
                case ValueType_Variant: {
                    const StateFindArgs args = { .log = true,
                                                 .searchParent = true };
                    const Atom* atom = StateFindAtomConst(
                      &temp, &matcher.variantValue.name, AtomType_Struct, args);
                    if (atom == NULL) {
                        TemLangError("Variant '%s' doesn't exist",
                                     matcher.variantValue.name.buffer);
                        break;
                    }

                    TemLangString targetName = CompilerGetFullVariableName(
                      &e->matchExpression->matcher, &temp, allocator);
                    TemLangStringAppendFormat(
                      s, "switch(%s.tag){", targetName.buffer);

                    NamedValue nv = { 0 };
                    for (size_t i = 0; i < e->matchExpression->branches.used;
                         ++i) {
                        const MatchBranch* branch =
                          &e->matchExpression->branches.buffer[i];
                        const StructMember* m = NULL;
                        StructMemberListFindIf(
                          &atom->structDefinition.members,
                          (StructMemberListFindFunc)StructMemberNameEquals,
                          &branch->matcher.identifier,
                          &m,
                          NULL);
                        if (m == NULL) {
                            TemLangError("Failed to find variant member '%s' "
                                         "in variant '%s'",
                                         branch->matcher.identifier.buffer,
                                         matcher.variantValue.name.buffer);
                            break;
                        }
                        StructMemberToFakeValue(m, &temp, allocator, &nv);
                        State temp2 = { 0 };
                        StateCopy(&temp2, &temp, allocator);
                        TemLangString typeName =
                          StructMemberToTypeName(m, allocator);
                        if (!StateAddValue(&temp2,
                                           &nv.name,
                                           nv.value.type == ValueType_Type
                                             ? nv.value.fakeValue
                                             : &nv.value)) {
                            TemLangError("Cannot add variable '%s' because it "
                                         "already exists. "
                                         "Must execute this match expression "
                                         "in another scope",
                                         nv.name.buffer);
                            StateFree(&temp2);
                            TemLangStringFree(&typeName);
                            goto cleanupVariantMatch;
                        }
                        TemLangString s4 = CompilerGetBranch(
                          &temp2, allocator, target, &branch->branch);
                        if (nv.value.type == ValueType_Null) {
                            TemLangStringAppendFormat(
                              s,
                              "case %sTag_%s:{ %s}break;",
                              matcher.variantValue.name.buffer,
                              branch->matcher.identifier.buffer,
                              s4.buffer);
                        } else {
                            TemLangStringAppendFormat(
                              s,
                              "case %sTag_%s:{%s %s = %s.%s; %s}break;",
                              matcher.variantValue.name.buffer,
                              branch->matcher.identifier.buffer,
                              typeName.buffer,
                              nv.name.buffer,
                              targetName.buffer,
                              nv.name.buffer,
                              s4.buffer);
                        }
                        TemLangStringFree(&s4);
                        StateFree(&temp2);
                        TemLangStringFree(&typeName);
                    }
                cleanupVariantMatch:
                    NamedValueFree(&nv);
                    TemLangString s4 =
                      CompilerGetBranch(&temp,
                                        allocator,
                                        target,
                                        &e->matchExpression->defaultBranch);
                    TemLangStringAppendFormat(
                      s, "default:{%s}break;}", s4.buffer);
                    TemLangStringFree(&s4);
                    TemLangStringFree(&targetName);
                } break;
                default: {
                    TemLangStringCreateFormat(
                      targetName, allocator, "targetName%zu", variableId);
                    ++variableId;
                    {
                        TemLangString s1 =
                          CompilerAssignValue(&temp,
                                              &targetName,
                                              allocator,
                                              &matcher,
                                              &e->matchExpression->matcher,
                                              true);
                        if (!StateAddValue(&temp, &targetName, &matcher)) {
                            TemLangError("Compiler error! Check (%s:%zu)",
                                         __FILE__,
                                         __LINE__);
                        }
                        TemLangStringAppend(&s, &s1);
                        TemLangStringFree(&s1);
                    }
                    for (size_t i = 0; i < e->matchExpression->branches.used;
                         ++i) {
                        State temp2 = { 0 };
                        StateCopy(&temp2, &temp, allocator);
                        TemLangString s1 = CompilerGetMatchBranch(
                          &temp2,
                          allocator,
                          target,
                          &targetName,
                          &matcher,
                          &e->matchExpression->branches.buffer[i]);
                        TemLangStringAppendFormat(s, "%s", s1.buffer);
                        TemLangStringFree(&s1);
                        COMPILE_COPIED_STATE_CLEANUP(temp, temp2, s);
                    }
                    {
                        TemLangString s1 =
                          CompilerGetBranch(&temp,
                                            allocator,
                                            target,
                                            &e->matchExpression->defaultBranch);
                        TemLangStringAppend(&s, &s1);
                        TemLangStringFree(&s1);
                    }
                    TemLangStringFree(&targetName);
                } break;
            }
            ValueFree(&matcher);
            COMPILE_COPIED_STATE_CLEANUP((*state), temp, s);
            TemLangStringAppendFormat(s,
                                      "goto scope%zu;\nscope%zu:(void)NULL;\n}",
                                      targetScopeNumber,
                                      targetScopeNumber);
            size_tListPop(&returnValueScopes);
        } break;
        case ExpressionType_Binary: {
            if (!ExpressionIsSimple(e)) {
                TemLangStringAppendChars(
                  &s,
                  "{\n//Start of binary expression. (void)NULL "
                  "is to ensure clang-format can properly format the "
                  "code (fix later?)\n(void)NULL;\n");
            }
            Value left = { 0 };
            Value right = { 0 };
            EvaluateExpression(e->left, state, &left, allocator);
            EvaluateExpression(e->right, state, &right, allocator);
            switch (e->op.type) {
                case OperatorType_Number: {
                    TemLangString a = CompilerGetNumberExpression(
                      state, allocator, target, &left, &right, value, e);
                    TemLangStringAppend(&s, &a);
                    TemLangStringFree(&a);
                } break;
                case OperatorType_Boolean: {
                    TemLangString a = CopmilerGetBooleanExpression(
                      state, allocator, target, &left, &right, e);
                    TemLangStringAppend(&s, &a);
                    TemLangStringFree(&a);
                } break;
                case OperatorType_Comparison: {
                    TemLangString a = CopmilerGetComparisonExpression(
                      state, allocator, target, &left, &right, e);
                    TemLangStringAppend(&s, &a);
                    TemLangStringFree(&a);
                } break;
                case OperatorType_Function: {
                    const size_t targetScope = scopeNumber;
                    ++scopeNumber;
                    size_tListAppend(&returnValueScopes, &targetScope);
                    TemLangStringAppendFormat(
                      s, "// function %s\n", e->op.functionCall.buffer);
                    // TemLangStringAppendChars(&s, "{\n");
                    State temp = { 0 };
                    temp.parent = state;
                    temp.atoms.allocator = allocator;
                    const StateFindArgs args = { .log = true,
                                                 .searchParent = true };
                    const Atom* atom =
                      StateFindAnyAtomConst(state, &e->op.functionCall, args);
                    if (atom == NULL) {
                        break;
                    }
                    switch (atom->functionDefinition.type) {
                        case FunctionType_Nullary: {
                            // No parameters to add
                        } break;
                        case FunctionType_Unary: {
                            const Value* realValue =
                              getUnaryValue(&left, &right);
                            const Expression* realExp =
                              realValue == &left ? e->left : e->right;
                            TemLangString ls = CompilerAssignValue(
                              state,
                              &atom->functionDefinition.leftParameter,
                              allocator,
                              realValue,
                              realExp,
                              true);
                            TemLangStringAppend(&s, &ls);
                            TemLangStringFree(&ls);
                            if (!StateAddValue(
                                  &temp,
                                  &atom->functionDefinition.leftParameter,
                                  realValue)) {
                                TemLangError("Compiler error! Check (%s:%zu)",
                                             __FILE__,
                                             __LINE__);
                            }
                        } break;
                        case FunctionType_Binary: {
                            TemLangString ls = CompilerAssignValue(
                              state,
                              &atom->functionDefinition.leftParameter,
                              allocator,
                              &left,
                              e->left,
                              true);
                            TemLangString rs = CompilerAssignValue(
                              state,
                              &atom->functionDefinition.rightParameter,
                              allocator,
                              &right,
                              e->right,
                              true);
                            TemLangStringAppend(&s, &ls);
                            TemLangStringAppend(&s, &rs);
                            TemLangStringFree(&ls);
                            TemLangStringFree(&rs);
                            if (!StateAddValue(
                                  &temp,
                                  &atom->functionDefinition.leftParameter,
                                  &left) ||
                                !StateAddValue(
                                  &temp,
                                  &atom->functionDefinition.rightParameter,
                                  &right)) {
                                TemLangError("Compiler error! Check (%s:%zu)",
                                             __FILE__,
                                             __LINE__);
                            }
                        } break;
                        default:
                            TemLangError("Failed to compile function type '%s'",
                                         FunctionTypeToString(
                                           atom->functionDefinition.type));
                            break;
                    }

                    CompileInstructions(&atom->functionDefinition.instructions,
                                        allocator,
                                        target,
                                        &temp,
                                        &s);
                    COMPILE_STATE_CLEANUP(temp, s);
                    {
                        TemLangStringAppendFormat(
                          s,
                          "goto scope%zu;scope%zu:(void)NULL;",
                          targetScope,
                          targetScope);
                    }
                    size_tListPop(&returnValueScopes);
                } break;
                case OperatorType_Get: {
                    TemLangString s1 = CompilerGetOperatorExpression(
                      state, allocator, target, &left, &right, value, e);
                    TemLangStringAppend(&s, &s1);
                    TemLangStringFree(&s1);
                } break;
                default:
                    TemLangError("Failed to compile operator type '%s'",
                                 OperatorTypeToString(e->op.type));
                    break;
            }
            ValueFree(&left);
            ValueFree(&right);
            if (!ExpressionIsSimple(e)) {
                TemLangStringAppendChars(&s, "\n}");
            }
        } break;
        default:
            TemLangError("Cannot compile expression of type '%s'",
                         ExpressionTypeToString(e->type));
            break;
    }
exit:
    return s;
}

static inline TemLangString
CompilerAssignValue(const State* state,
                    const TemLangString* name,
                    const Allocator* allocator,
                    const Value* value,
                    const Expression* expression,
                    const bool create)
{
    TemLangString s = TemLangStringCreate("", allocator);
    if (create) {
        TemLangString a =
          CompilerDeclareValueType(name, state, allocator, value, true);
        TemLangStringAppend(&s, &a);
        TemLangStringFree(&a);
        if (value->type == ValueType_Variant) {
            TemLangStringAppendFormat(s,
                                      "%s.tag = %sTag_%s;",
                                      name->buffer,
                                      value->variantValue.name.buffer,
                                      value->variantValue.memberName.buffer);
        }
    } else if (value->type == ValueType_Variant) {
        TemLangStringAppendFormat(s,
                                  "%sFree(&%s);%s.tag = %sTag_%s;",
                                  value->variantValue.name.buffer,
                                  name->buffer,
                                  name->buffer,
                                  value->variantValue.name.buffer,
                                  value->variantValue.memberName.buffer);
    }
    {
        const VariableTarget target = { .name = name,
                                        .type = VariableTarget_Variable };
        TemLangString a =
          CompilerGetExpression(state, allocator, target, value, expression);
        TemLangStringAppend(&s, &a);
        TemLangStringFree(&a);
    }
    return s;
}

static inline TemLangString
CompileWhileLoop(const State* state,
                 const Allocator* allocator,
                 const Expression* condition,
                 const InstructionList* instructions,
                 const bool isWhile)
{
    TemLangString name = TemLangStringCreate("condition", allocator);
    VariableTarget target = { .type = VariableTarget_Variable, .name = &name };
    TemLangString e =
      CompilerGetExpression(state, allocator, target, NULL, condition);
    TemLangString i = TemLangStringCreate("", allocator);
    VariableTarget instructionTarget = { .type = VariableTarget_None,
                                         .name = NULL };
    State temp = { 0 };
    StateCopy(&temp, state, allocator);
    CompileInstructions(instructions, allocator, instructionTarget, &temp, &i);
    TemLangStringCreateFormat(
      s,
      allocator,
      "{\nbool condition;%swhile(%ccondition){\n %s %s ",
      e.buffer,
      isWhile ? ' ' : '!',
      i.buffer,
      e.buffer);
    COMPILE_COPIED_STATE_CLEANUP((*state), temp, s);
    TemLangStringAppendChars(&s, "\n}\n}");
    TemLangStringFree(&name);
    TemLangStringFree(&e);
    TemLangStringFree(&i);
    return s;
}

static inline TemLangString
CompileListModify(const State* state,
                  const Allocator* allocator,
                  const ListModifyInstruction* i)
{
    TemLangString s = TemLangStringCreate("", allocator);
    State temp = { 0 };
    StateCopy(&temp, state, allocator);
    TemLangString name =
      CompilerGetFullVariableName(&i->list, &temp, allocator);
    const Value* listRef =
      EvaluateExpressionToReference(&i->list, &temp, allocator);
    Value newValue = { 0 };
    switch (i->type) {
        case ListModifyType_Append: {
            EvaluateExpression(&i->newValue[0], &temp, &newValue, allocator);
            TemLangStringCreateFormat(
              tempVar, allocator, "temp%zu", variableId);
            ++variableId;
            TemLangString o = CompilerAssignValue(
              &temp, &tempVar, allocator, &newValue, &i->newValue[0], true);
            if (listRef->type == ValueType_String) {
                switch (newValue.type) {
                    case ValueType_Number: {
                        TemLangStringAppendFormat(
                          s,
                          "{ %s TemLangStringAppendChar(&%s, (char)%s); }",
                          o.buffer,
                          name.buffer,
                          tempVar.buffer);
                    } break;
                    case ValueType_List: {
                        if (newValue.list.isArray) {
                            TemLangStringAppendFormat(
                              s,
                              "{ %s for(size_t i = 0; i < %u; ++i){ "
                              "TemLangStringAppendChar(&%s, (char)%s[i]); } }",
                              o.buffer,
                              newValue.list.values.used,
                              name.buffer,
                              tempVar.buffer);
                        } else {
                            TemLangStringAppendFormat(
                              s,
                              "{ %s for(size_t i = 0; i < %s.used; ++i){ "
                              "TemLangStringAppendChar(&%s,(char)%s.buffer[i]);"
                              " } }",
                              o.buffer,
                              tempVar.buffer,
                              name.buffer,
                              tempVar.buffer);
                        }
                    } break;
                    default: {
                        TemLangString temp = { .allocator = allocator };
                        if (noCleanupState == 0) {
                            TemLangStringAppendFormat(
                              temp, "TemLangStringFree(&%s);", tempVar.buffer);
                        } else {
                            TemLangStringAppendFormat(
                              temp,
                              "\n//Cleanup disabled! %s will "
                              "not be cleaned up\n",
                              tempVar.buffer);
                        }
                        TemLangStringAppendFormat(
                          s,
                          "{ %s TemLangStringAppend(&%s, &%s); %s }",
                          o.buffer,
                          name.buffer,
                          tempVar.buffer,
                          temp.buffer);
                        TemLangStringFree(&temp);
                    } break;
                }
            } else {
                TemLangString c = CompilerDeclareValueType(
                  NULL, state, allocator, &newValue, false);
                TemLangStringAppendFormat(s,
                                          "{ %s %sListAppend(&%s, &%s); }",
                                          o.buffer,
                                          c.buffer,
                                          name.buffer,
                                          tempVar.buffer);
                TemLangStringFree(&c);
            }
            TemLangStringFree(&tempVar);
            TemLangStringFree(&o);
        } break;
        case ListModifyType_Insert: {
            uint64_t index = 0;
            EvaluateExpression(&i->newValue[0], state, &newValue, allocator);
            ValueToIndex(&newValue, &index);
            EvaluateExpression(&i->newValue[1], state, &newValue, allocator);
            TemLangStringCreateFormat(
              tempVar, allocator, "temp%zu", variableId);
            ++variableId;
            TemLangString o = CompilerAssignValue(
              &temp, &tempVar, allocator, &newValue, &i->newValue[1], true);
            if (listRef->type == ValueType_String) {
                TemLangStringAppendFormat(
                  s,
                  "{ %s TemLangStringInsertChar(&%s, %" PRIu64 ", (char)%s); }",
                  o.buffer,
                  name.buffer,
                  index,
                  tempVar.buffer);
            } else {
                TemLangString c = CompilerDeclareValueType(
                  NULL, state, allocator, &newValue, NULL);
                TemLangStringAppendFormat(s,
                                          "{ %s %sListInsert(&%s, %" PRIu64
                                          ", &%s); }",
                                          o.buffer,
                                          c.buffer,
                                          name.buffer,
                                          index,
                                          tempVar.buffer);
                TemLangStringFree(&c);
            }
            TemLangStringFree(&tempVar);
            TemLangStringFree(&o);
        } break;
        case ListModifyType_Remove: {
            uint64_t index = 0;
            EvaluateExpression(&i->newValue[0], state, &newValue, allocator);
            ValueToIndex(&newValue, &index);
            if (listRef->type == ValueType_String) {
                TemLangStringAppendFormat(s,
                                          "TemLangStringRemove(&%s, %" PRIu64
                                          ");",
                                          name.buffer,
                                          index);
            } else {
                TemLangString c = CompilerDeclareValueType(
                  NULL, state, allocator, listRef->list.exampleValue, false);
                TemLangStringAppendFormat(s,
                                          "%sListRemove(&%s, %" PRIu64
                                          ", currentAllocator);",
                                          c.buffer,
                                          name.buffer,
                                          index);
                TemLangStringFree(&c);
            }
        } break;
        case ListModifyType_SwapRemove: {
            uint64_t index = 0;
            EvaluateExpression(&i->newValue[0], state, &newValue, allocator);
            ValueToIndex(&newValue, &index);
            TemLangString c = CompilerDeclareValueType(
              NULL, state, allocator, listRef->list.exampleValue, false);
            TemLangStringAppendFormat(s,
                                      "%sListSwapRemove(&%s, %" PRIu64 ");",
                                      c.buffer,
                                      name.buffer,
                                      index);
            TemLangStringFree(&c);
        } break;
        case ListModifyType_Pop: {
            if (listRef->type == ValueType_String) {
                TemLangStringAppendFormat(
                  s, "TemLangStringPop(&%s);", name.buffer);
            } else {
                TemLangString c = CompilerDeclareValueType(
                  NULL, state, allocator, listRef->list.exampleValue, false);
                TemLangStringAppendFormat(
                  s, "%sListPop(&%s);", c.buffer, name.buffer);
                TemLangStringFree(&c);
            }
        } break;
        case ListModifyType_Empty: {
            {
                TemLangStringAppendFormat(
                  s, "{const Allocator* a = %s.allocator;", name.buffer)
            }
            if (listRef->type == ValueType_String) {
                if (noCleanupState == 0) {
                    TemLangStringAppendFormat(
                      s, "TemLangStringFree(&%s);", name.buffer);
                }
            } else {
                TemLangString freeName = CompilerDeclareValueType(
                  NULL, state, allocator, listRef->list.exampleValue, false);
                TemLangStringAppendFormat(
                  s, "%sListFree(&%s);", freeName.buffer, name.buffer);
                TemLangStringFree(&freeName);
            }
            TemLangStringAppendFormat(s, "%s.allocator = a;}", name.buffer);
        } break;
        default:
            TemLangError("ListModify '%s' not implemented.",
                         ListModifyTypeToString(i->type));
            break;
    }
    ValueFree(&newValue);
    TemLangStringFree(&name);
    COMPILE_COPIED_STATE_CLEANUP((*state), temp, s);
    return s;
}

static inline bool
CompileCFunctionReturnValue(const State* state,
                            const Allocator* allocator,
                            const Expression* functionNameExp,
                            const Expression* functionArgsExp,
                            const TemLangString* returnValueName,
                            const InstructionList* instructions,
                            pStructDefinition d,
                            pTemLangString structOutput,
                            pTemLangString implOutput,
                            const bool appendStructOutput)
{
    bool result;
    Value value = { 0 };
    Value functionName = { 0 };
    Value functionArgs = { 0 };
    TemLangString returnInstruction = {
        .allocator = allocator, .buffer = NULL, .used = 0, .size = 0
    };
    const VariableTarget target = { .type = VariableTarget_None };
    {
        State temp = { 0 };
        temp.parent = state;
        temp.atoms.allocator = allocator;
        result =
          EvaluateExpression(
            functionNameExp, &temp, &functionName, allocator) &&
          EvaluateExpression(functionArgsExp, &temp, &functionArgs, allocator);
        if (!result) {
            goto cleanupInlineCFunctionReturnStruct;
        }
        if (functionName.type != ValueType_String ||
            functionArgs.type != ValueType_String) {
            TemLangError("Inline C function definition and return struct must "
                         "be a string for both the name and argument. Got '%s'",
                         ValueTypeToString(value.type));
            result = false;
            goto stateCleanup;
        }
        TemLangString unused = {
            .allocator = allocator, .buffer = NULL, .used = 0, .size = 0
        };
        for (size_t i = 0; result && i < instructions->used; ++i) {
            ValueFree(&value);
            const Instruction* instruction = &instructions->buffer[i];
            switch (instruction->type) {
                case InstructionType_InlineVariable: {
                    result = CompileInstruction(
                      &temp, instruction, allocator, target, &unused);
                } break;
                default:
                    result = StateProcessInstruction(
                      &temp, instruction, allocator, &value);
                    break;
            }
            if (!result) {
                InstructionError(instruction);
            }
        }
        TemLangStringFree(&unused);
        if (!result) {
            goto stateCleanup;
        }
        NamedValueList nvList = StateToVariableList(&temp, allocator);
        const VariableTarget target = { .type = VariableTarget_None };
        TemLangString structName = TemLangStringCreate("", allocator);
        if (returnValueName == NULL) {
            TemLangStringAppendFormat(
              structName, "%sReturnValue", functionName.string.buffer);
        } else {
            TemLangStringAppend(&structName, returnValueName);
        }
        {
            Instruction newI = { .type = InstructionType_DefineStruct,
                                 .defineStruct = {
                                   .name = structName,
                                   .definition = { .isVariant = false } } };

            // Struct name cleaned up by instruction
            newI.defineStruct.definition.members.allocator = allocator;
            result =
              NamedValuesToStructMemberList(
                &nvList, &temp, &newI.defineStruct.definition.members, true) &&
              CompileInstruction(&temp, &newI, allocator, target, structOutput);
            if (d != NULL) {
                StructDefinitionCopy(
                  d, &newI.defineStruct.definition, allocator);
            }
            InstructionFree(&newI);
        }
        for (size_t i = 0; i < nvList.used; ++i) {
            const NamedValue* nv = &nvList.buffer[i];
            if (nv->value.type == ValueType_List && nv->value.list.isArray) {
                TemLangStringCreateFormat(
                  ts,
                  allocator,
                  "memcpy(returnValue.%s, %s, sizeof(%s));",
                  nv->name.buffer,
                  nv->name.buffer,
                  nv->name.buffer);
                TemLangStringAppend(&returnInstruction, &ts);
                TemLangStringFree(&ts);
            } else {
                TemLangStringCreateFormat(ts,
                                          allocator,
                                          "returnValue.%s = %s;",
                                          nv->name.buffer,
                                          nv->name.buffer);
                TemLangStringAppend(&returnInstruction, &ts);
                TemLangStringFree(&ts);
            }
        }
        NamedValueListFree(&nvList);
    stateCleanup:
        StateFree(&temp);
    }
    if (!result) {
        goto cleanupInlineCFunctionReturnStruct;
    }
    if (returnValueName == NULL) {
        TemLangStringAppendFormat(
          (*implOutput),
          "%s %sReturnValue %s %s{%sReturnValue returnValue = {0};",
          appendStructOutput ? structOutput->buffer : "",
          functionName.string.buffer,
          functionName.string.buffer,
          functionArgs.string.buffer,
          functionName.string.buffer);
    } else {
        TemLangStringAppendFormat((*implOutput),
                                  "%s %s %s %s{%s returnValue = {0};",
                                  appendStructOutput ? structOutput->buffer
                                                     : "",
                                  returnValueName->buffer,
                                  functionName.string.buffer,
                                  functionArgs.string.buffer,
                                  returnValueName->buffer);
    }
    {
        State temp = { 0 };
        temp.parent = state;
        temp.atoms.allocator = allocator;
        result = CompileInstructions(
          instructions, allocator, target, &temp, implOutput);
        StateFree(&temp);
    }
cleanupInlineCFunctionReturnStruct:
    TemLangStringAppendFormat((*implOutput),
                              "%sgoto end;end:\nreturn returnValue;}",
                              returnInstruction.buffer);
    ValueFree(&functionName);
    ValueFree(&functionArgs);
    TemLangStringFree(&returnInstruction);
    return result;
}

static inline bool
CompileInstruction(State* state,
                   const Instruction* instruction,
                   const Allocator* allocator,
                   const VariableTarget target,
                   pTemLangString output)
{
    static size_t inVariant = 0;
    Value value = { 0 };
    bool result = true;
    switch (instruction->type) {
        case InstructionType_Inline:
            break;
        default:
            result =
              StateProcessInstruction(state, instruction, allocator, &value);
            break;
    }
    if (!result) {
        TemLangError("Error occurred before generating code");
        goto cleanup;
    }

    TemLangStringAppendFormat((*output),
                              "\n//%s:%zu\n",
                              instruction->source.source.buffer,
                              instruction->source.lineNumber);

    switch (instruction->type) {
        case InstructionType_CreateVariable: {
            const StateFindArgs args = { .log = true, .searchParent = false };
            const Atom* atom =
              StateFindAtomConst(state,
                                 &instruction->createVariable.name,
                                 AtomType_Variable,
                                 args);
            if (atom == NULL) {
                result = false;
                break;
            }
            TemLangString s = { 0 };
            switch (atom->variable.type) {
                case VariableType_Constant: {
                    result =
                      EvaluateExpression(&instruction->createVariable.value,
                                         state,
                                         &value,
                                         allocator);
                    if (!result) {
                        break;
                    }
                    Expression tempE = { .type = ExpressionType_UnaryValue,
                                         .value = value };
                    s = CompilerAssignValue(state,
                                            &instruction->createVariable.name,
                                            allocator,
                                            &value,
                                            &tempE,
                                            true);
                } break;
                default:
                    s = CompilerAssignValue(state,
                                            &instruction->createVariable.name,
                                            allocator,
                                            &atom->variable.value,
                                            &instruction->createVariable.value,
                                            true);
                    break;
            }
            TemLangStringAppend(output, &s);
            TemLangStringFree(&s);
        } break;
        case InstructionType_UpdateVariable: {
            Value* temp = EvaluateExpressionToReference(
              &instruction->updateVariable.target, state, allocator);
            TemLangString s1 = CompilerGetFullVariableName(
              &instruction->updateVariable.target, state, allocator);
            TemLangString s2 =
              CompilerAssignValue(state,
                                  &s1,
                                  allocator,
                                  temp,
                                  &instruction->updateVariable.value,
                                  false);
            TemLangStringAppend(output, &s2);
            TemLangStringFree(&s1);
            TemLangStringFree(&s2);
        } break;
        case InstructionType_DefineEnum: {
            const StateFindArgs args = { .log = true, .searchParent = false };
            const Atom* atom = StateFindAtomConst(
              state, &instruction->createVariable.name, AtomType_Enum, args);
            if (atom == NULL) {
                result = false;
                break;
            }

            const int64_t count = (int64_t)atom->enumDefinition.members.used;
            if (inVariant == 0 && count == 2) {
                TemLangStringAppendFormat(
                  (*output),
                  "typedef bool %s; typedef bool* p%s; const bool "
                  "%s_Invalid=false; const bool %s_%s=false; const bool "
                  "%s_%s=true;\n#define %s_Length 2\n "
                  "MAKE_COPY_AND_FREE(%s);\nstatic inline size_t "
                  "%sSerialize(const %s* value, pBytes bytes, const bool e){ "
                  "return booleanSerialize((const boolean*)value, bytes, e);}"
                  "static inline size_t %sDeserialize(%s* value, const Bytes* "
                  "bytes, const size_t offset, const bool e){ return "
                  "booleanDeserialize((boolean*)value,bytes,offset, e);}",
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->enumDefinition.members.buffer[0].buffer,
                  atom->name.buffer,
                  atom->enumDefinition.members.buffer[1].buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer);
                TemLangStringAppendFormat(
                  (*output),
                  "static inline const char* %sToCharString(const %s value){"
                  "return value ? \"%s\" : \"%s\";} static inline "
                  "TemLangString "
                  "%sToString(const %s value){ return "
                  "TemLangStringCreate(%sToCharString(value), "
                  "currentAllocator); }",
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->enumDefinition.members.buffer[1].buffer,
                  atom->enumDefinition.members.buffer[0].buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer);
                break;
            }

            const char* c = NULL;
            {
                const Range range = {
                    .min = { .type = NumberType_Signed, .i = -1L },
                    .max = { .type = NumberType_Signed, .i = count }
                };
                c = CTypeToTypeString(RangeToCType(&range));
            }
            {
                TemLangStringAppendFormat(
                  (*output),
                  "typedef %s %s; typedef %s* p%s; const %s %s_Invalid=-1;",
                  c,
                  atom->name.buffer,
                  c,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer);
            }
            TemLangString toString = TemLangStringCreate("", allocator);
            TemLangString fromString = TemLangStringCreate("", allocator);
            for (size_t i = 0; i < atom->enumDefinition.members.used; ++i) {
                const TemLangString* s =
                  &atom->enumDefinition.members.buffer[i];
                {
                    TemLangStringAppendFormat(
                      (*output),
                      "const %s %s_%s = %zu;",
                      atom->name.buffer,
                      atom->name.buffer,
                      s->buffer,
                      atom->enumDefinition.isFlag ? (1 << i) : i);
                }
                {
                    TemLangStringAppendFormat(toString,
                                              "case %s_%s: return \"%s\";",
                                              atom->name.buffer,
                                              s->buffer,
                                              s->buffer);
                }
                TemLangStringAppendFormat(
                  fromString,
                  "if(TemLangStringEquals(value, \"%s\")){ return %s_%s; }",
                  s->buffer,
                  atom->name.buffer,
                  s->buffer);
            }
            const Range r = {
                .min = { .type = NumberType_Signed, .i = -1L },
                .max = { .type = NumberType_Unsigned,
                         .u = atom->enumDefinition.isFlag
                                ? (1UL << atom->enumDefinition.members.used)
                                : atom->enumDefinition.members.used }
            };
            const CType t = RangeToCType(&r);
            const char* tc = CTypeToTypeString(t);
            TemLangStringAppendFormat(
              (*output),
              "\n#define %sFree(a)\nstatic inline bool %sCopy(%s* "
              "a, const %s* b, const void* p) { *a = *b; (void)p; return "
              "true;}static inline size_t %sSerialize(const %s* value, pBytes "
              "bytes, const bool e){ const %s serValue = *value; return "
              "%sSerialize(&serValue, bytes,e);}static inline size_t "
              "%sDeserialize(%s* value, const Bytes* bytes, const size_t "
              "offset, const bool e){ %s serValue = {0}; const size_t result = "
              "%sDeserialize(&serValue, bytes, offset, "
              "e); *value = (%s)serValue; return result;}static inline const "
              "char* %sToCharString(const %s value){switch(value){ %s default: "
              "return \"Invalid %s\";}}static inline %s %sFromString(const "
              "TemLangString *value){%s return %s_Invalid;}",
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer,
              tc,
              tc,
              atom->name.buffer,
              atom->name.buffer,
              tc,
              tc,
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer,
              toString.buffer,
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer,
              fromString.buffer,
              atom->name.buffer);
            TemLangStringAppendFormat(
              (*output),
              "static inline TemLangString %sToString(const %s value){return "
              "TemLangStringCreate(%sToCharString(value), currentAllocator);}",
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer);
            TemLangStringAppendFormat((*output),
                                      "\n#define %s_Length %u\n",
                                      atom->name.buffer,
                                      atom->enumDefinition.members.used);
            TemLangStringFree(&toString);
            TemLangStringFree(&fromString);
        } break;
        case InstructionType_DefineRange: {
            StateFindArgs args = { .log = true, .searchParent = false };
            const Atom* atom = StateFindAtomConst(
              state, &instruction->defineRange.name, AtomType_Range, args);
            if (atom == NULL) {
                result = false;
                break;
            }
            const char* c = CTypeToTypeString(RangeToCType(&atom->range));
            TemLangStringAppendFormat(
              (*output),
              "typedef %s %s;\n#define %sFree(a)\n"
              "static inline bool %sCopy(%s* a, const %s* b, const "
              "void* p) { *a = *b; (void)p; return true; } static inline "
              "size_t %sSerialize(const %s* value, pBytes bytes, const bool "
              "e){ return %sSerialize(value, bytes, e);}size_t "
              "%sDeserialize(%s* value, const Bytes* bytes, const size_t "
              "offset,const bool e){return %sDeserialize(value, bytes, offset, "
              "e);}",
              c,
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer,
              atom->name.buffer,
              c,
              atom->name.buffer,
              c,
              c);
        } break;
        case InstructionType_DefineStruct: {
            StateFindArgs args = { .log = true, .searchParent = false };
            const Atom* atom = StateFindAtomConst(
              state, &instruction->defineStruct.name, AtomType_Struct, args);
            if (atom == NULL) {
                result = false;
                break;
            }
            TemLangString cleanups = TemLangStringCreate("", allocator);
            TemLangString copies = TemLangStringCreate("", allocator);
            TemLangString serialization = TemLangStringCreate("", allocator);
            TemLangString deserialization = TemLangStringCreate("", allocator);
            if (atom->structDefinition.isVariant) {
                {

                    Instruction newI = { .type = InstructionType_DefineEnum,
                                         .defineEnum = {
                                           .name =
                                             TemLangStringCreate("", allocator),
                                           .definition = {
                                             .isFlag = false,
                                             .members = { .buffer = NULL,
                                                          .allocator =
                                                            allocator,
                                                          .size = 0,
                                                          .used = 0 } } } };
                    InstructionSourceCopy(
                      &newI.source, &instruction->source, allocator);
                    {
                        TemLangStringAppendFormat(
                          newI.defineEnum.name, "%sTag", atom->name.buffer);
                    }
                    for (size_t i = 0;
                         result && i < atom->structDefinition.members.used;
                         ++i) {
                        const StructMember* m =
                          &atom->structDefinition.members.buffer[i];
                        result = TemLangStringListAppend(
                          &newI.defineEnum.definition.members, &m->name);
                    }
                    State temp = { 0 };
                    temp.parent = state;
                    temp.atoms.allocator = allocator;
                    ++inVariant;
                    result = CompileInstruction(
                      &temp, &newI, allocator, target, output);
                    --inVariant;
                    TemLangStringAppendFormat(
                      (*output),
                      "typedef struct %s{ %sTag tag; union{",
                      atom->name.buffer,
                      atom->name.buffer);
                    StateFree(&temp);
                    InstructionFree(&newI);
                }
                for (size_t i = 0;
                     result && i < atom->structDefinition.members.used;
                     ++i) {
                    const StructMember* m =
                      &atom->structDefinition.members.buffer[i];
                    CompileStructMember(true, end);
                end:
                    continue;
                }
                TemLangStringAppendFormat(
                  (*output),
                  "};} %s, *p%s; static inline void %sFree(%s* value){\n "
                  "switch(value->tag){\n %s default:break;  \n} "
                  "memset(value,0,sizeof(*value));\n}static "
                  "inline bool %sCopy(%s* a, const %s* b, const Allocator* p){ "
                  "%sFree(a); a->tag = b->tag;switch(b->tag){ %s "
                  "default:return false; } return true;} static inline size_t "
                  "%sSerialize(const %s* value, pBytes bytes, const bool e){ "
                  "size_t total = %sTagSerialize(&value->tag, bytes, e); "
                  "switch(value->tag){%s default: break;} return total;} "
                  "static inline size_t %sDeserialize(%s* value, const Bytes* "
                  "bytes, const size_t offset, const bool e){ size_t total = "
                  "%sTagDeserialize(&value->tag, bytes, offset, e);"
                  "switch(value->tag){ %s default:break;} return total;}",
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  cleanups.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  TemLangStringIsEmpty(&copies) ? "(void)a;(void)b;(void)p;"
                                                : copies.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  serialization.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  deserialization.buffer);
            } else {
                {
                    TemLangStringAppendFormat(
                      (*output), "typedef struct %s{", atom->name.buffer);
                }
                for (size_t i = 0; i < atom->structDefinition.members.used;
                     ++i) {
                    const StructMember* m =
                      &atom->structDefinition.members.buffer[i];
                    CompileStructMember(false, vend);
                vend:
                    continue;
                }

                TemLangString destructor = TemLangStringCreate("", allocator);
                if (!InstructionListIsEmpty(
                      &atom->structDefinition.deleteInstructions)) {
                    TemLangString cm = { .allocator = allocator };
                    State temp = { 0 };
                    temp.parent = state;
                    temp.atoms.allocator = allocator;
                    result = CompileInstructions(
                      &atom->structDefinition.deleteInstructions,
                      allocator,
                      target,
                      &temp,
                      &cm);
                    if (result) {
                        TemLangStringAppendFormat(
                          destructor,
                          "{%s %s = *value; %s}",
                          atom->name.buffer,
                          instruction->defineStruct.definition
                            .destructorTargetName.buffer,
                          TemLangStringIsEmpty(&cm) ? "" : cm.buffer);
                    }
                    TemLangStringFree(&cm);
                }
                TemLangStringAppendFormat(
                  (*output),
                  "} %s, *p%s; static inline void %sFree(%s* "
                  "value){ %s %s memset(value,0,sizeof(*value)); }\nstatic "
                  "inline bool %sCopy(%s* a, const %s* b, const Allocator* p) "
                  "{ %sFree(a); %s return true; } static inline size_t "
                  "%sSerialize(const %s* value, pBytes bytes, const bool e){ "
                  "size_t total = 0UL; %s return total;} static inline size_t "
                  "%sDeserialize(%s* value, const Bytes* bytes, const size_t "
                  "offset, const bool e){ size_t total = 0UL; %s return "
                  "total;}",
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  destructor.buffer,
                  cleanups.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  TemLangStringIsEmpty(&copies) ? "(void)a;(void)b;(void)p;"
                                                : copies.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  serialization.buffer,
                  atom->name.buffer,
                  atom->name.buffer,
                  deserialization.buffer);
                TemLangStringFree(&destructor);
            }
            TemLangStringFree(&cleanups);
            TemLangStringFree(&copies);
            TemLangStringFree(&serialization);
            TemLangStringFree(&deserialization);
        } break;
        case InstructionType_ChangeFlag: {
            const ChangeFlagInstruction* cf = &instruction->changeFlag;
            TemLangString s1 = CompilerGetFullVariableName(
              &instruction->changeFlag.target, state, allocator);
            TemLangString s2 = { 0 };
            if (cf->flag == ChangeFlagType_Toggle) {
                Value v = { 0 };
                EvaluateExpression(
                  &instruction->changeFlag.target, state, &v, allocator);
                if (v.type == ValueType_Boolean) {
                    TemLangStringAppendFormat(
                      (*output), "%s = !%s;", s1.buffer, s1.buffer);
                    goto changeFlagCleanup;
                }
                ValueFree(&v);
            }
            s2 = CompilerGetVariableName(
              &instruction->changeFlag.target, state, allocator);
            const StateFindArgs args = { .log = true, .searchParent = false };
            const Atom* atom =
              StateFindAtomConst(state, &s2, AtomType_Variable, args);
            if (atom == NULL) {
                result = false;
                goto changeFlagCleanup;
            }

            switch (cf->flag) {
                case ChangeFlagType_Add: {
                    TemLangStringAppendFormat(
                      (*output),
                      "%s |= %s_%s;",
                      s1.buffer,
                      atom->variable.value.flagValue.name.buffer,
                      cf->member.buffer);
                } break;
                case ChangeFlagType_Toggle: {
                    TemLangStringAppendFormat(
                      (*output),
                      "%s ^= %s_%s;",
                      s1.buffer,
                      atom->variable.value.flagValue.name.buffer,
                      cf->member.buffer);
                } break;
                case ChangeFlagType_Remove: {
                    TemLangStringAppendFormat(
                      (*output),
                      "%s &= ~(%s_%s);",
                      s1.buffer,
                      atom->variable.value.flagValue.name.buffer,
                      cf->member.buffer);
                } break;
                default:
                    break;
            }
        changeFlagCleanup:
            TemLangStringFree(&s1);
            TemLangStringFree(&s2);
        } break;
        case InstructionType_SetAllFlag: {
            TemLangString s2 = CompilerGetVariableName(
              &instruction->setAllFlag.target, state, allocator);
            const StateFindArgs args = { .log = true, .searchParent = false };
            const Atom* atom =
              StateFindAtomConst(state, &s2, AtomType_Variable, args);

            TemLangString s1 = CompilerGetFullVariableName(
              &instruction->changeFlag.target, state, allocator);
            if (instruction->setAllFlag.clear) {
                TemLangStringAppendFormat((*output), "%s = 0;", s1.buffer);
            } else {
                TemLangStringAppendFormat((*output), "%s = ", s1.buffer);
                for (size_t i = 0;
                     i < atom->variable.value.flagValue.members.used;
                     ++i) {
                    TemLangStringAppendFormat(
                      (*output),
                      "%s_%s",
                      atom->variable.value.flagValue.name.buffer,
                      atom->variable.value.flagValue.members.buffer[i].buffer);
                    if (i != atom->variable.value.flagValue.members.used - 1) {
                        TemLangStringAppendChar(output, '|');
                    }
                }
                TemLangStringAppendChar(output, ';');
            }
            TemLangStringFree(&s1);
            TemLangStringFree(&s2);
        } break;
        case InstructionType_While: {
            TemLangString s1 =
              CompileWhileLoop(state,
                               allocator,
                               &instruction->captureInstruction.target,
                               &instruction->captureInstruction.instructions,
                               true);
            TemLangStringAppend(output, &s1);
            TemLangStringFree(&s1);
        } break;
        case InstructionType_Until: {
            TemLangString s1 =
              CompileWhileLoop(state,
                               allocator,
                               &instruction->captureInstruction.target,
                               &instruction->captureInstruction.instructions,
                               false);
            TemLangStringAppend(output, &s1);
            TemLangStringFree(&s1);
        } break;
        case InstructionType_Inline: {
            result = EvaluateExpression(
              &instruction->expression, state, &value, allocator);
            if (!result) {
                break;
            }
            char buffer[1024] = { 0 };
            snprintf(buffer,
                     sizeof(buffer),
                     "<Inlined instructions at (%s:%zu)>",
                     instruction->source.source.buffer,
                     instruction->source.lineNumber);
            TokenList list = performLex(
              allocator, value.string.buffer, value.string.used, 1, buffer);
            InstructionList instructions =
              TokensToInstructions(&list, allocator);
            TemLangString s1 = TemLangStringCreate("", allocator);

            result =
              CompileInstructions(&instructions, allocator, target, state, &s1);
            TemLangStringAppend(output, &s1);
            TemLangStringFree(&s1);
            InstructionListFree(&instructions);
            TokenListFree(&list);
        } break;
        case InstructionType_InlineFile: {
            Value value = { 0 };
            int fd = -1;
            char* ptr = NULL;
            size_t size = 0UL;
            result = EvaluateExpression(
              &instruction->expression, state, &value, allocator);
            if (!result) {
                goto inlineFileCleanup;
            }
            if (value.type != ValueType_String) {
                TemLangError("Inlined file must be a string. Got '%s'",
                             ValueTypeToString(value.type));
                result = false;
                goto inlineFileCleanup;
            }
            TemLangStringAppendFormat(
              (*output), "// Inlining file '%s'", value.string.buffer);
            if (mapFile(
                  value.string.buffer, &fd, &ptr, &size, MapFileType_Read)) {
                TokenList tokens =
                  performLex(allocator, ptr, size, 0UL, value.string.buffer);
                InstructionList instructions =
                  TokensToInstructions(&tokens, allocator);
                const VariableTarget target = { .type = VariableTarget_None };
                State temp = { 0 };
                temp.atoms.allocator = allocator;
                // temp.parent = state;
                result = CompileInstructions(
                  &instructions, allocator, target, &temp, output);
                InstructionListFree(&instructions);
                TokenListFree(&tokens);
                StateFree(&temp);
            } else {
                TemLangError("Failed to open file '%s': %s",
                             value.string.buffer,
                             strerror(errno));
                result = false;
            }
        inlineFileCleanup:
            ValueFree(&value);
            unmapFile(fd, ptr, size);
        } break;
        case InstructionType_InlineText:
        case InstructionType_InlineData: {
            Value value = { 0 };
            int fd = -1;
            char* ptr = NULL;
            size_t size = 0UL;
            result = EvaluateExpression(
              &instruction->dataFile, state, &value, allocator);
            if (!result) {
                goto inlineDataCleanup;
            }
            if (value.type != ValueType_String) {
                TemLangError("Inlined file must be a string. Got '%s'",
                             ValueTypeToString(value.type));
                result = false;
                goto inlineDataCleanup;
            }
            if (mapFile(
                  value.string.buffer, &fd, &ptr, &size, MapFileType_Read)) {
                if (instruction->dataIsBinary) {
                    TemLangStringAppendFormat((*output),
                                              "const uint8_t %s[] = {",
                                              instruction->dataName.buffer);
                    for (size_t i = 0; i < size; ++i) {
                        TemLangStringAppendFormat(
                          (*output), "0x%xU,", *(uint8_t*)&ptr[i]);
                    }
                    TemLangStringAppendChars(output, "};");
                } else {
                    TemLangStringAppendFormat((*output),
                                              "const char %s[] = \"",
                                              instruction->dataName.buffer);
                    TemLangString s =
                      TemLangStringCreateWithSize(size, allocator);
                    result = TemLangStringAppendCount(&s, ptr, size);
                    TemLangStringRemoveNewLines(&s);
                    if (result) {
                        result = TemLangStringAppend(output, &s);
                    }
                    TemLangStringFree(&s);
                    TemLangStringAppendChars(output, "\";\0");
                }
            } else {
                TemLangError("Failed to open file '%s': %s",
                             value.string.buffer,
                             strerror(errno));
                result = false;
            }
        inlineDataCleanup:
            ValueFree(&value);
            unmapFile(fd, ptr, size);
        } break;
        case InstructionType_InlineCFile: {
            Value value = { 0 };
            int fd = -1;
            char* ptr = NULL;
            size_t size = 0UL;
            result = EvaluateExpression(
              &instruction->expression, state, &value, allocator);
            if (!result) {
                goto inlineCFileCleanup;
            }
            if (value.type != ValueType_String) {
                TemLangError("Inlined file must be a string. Got '%s'",
                             ValueTypeToString(value.type));
                result = false;
                goto inlineCFileCleanup;
            }
            if (mapFile(
                  value.string.buffer, &fd, &ptr, &size, MapFileType_Read)) {
                result = TemLangStringAppendCount(output, ptr, size) &&
                         TemLangStringAppendChar(output, '\n');
            } else {
                TemLangError("Failed to open file '%s': %s",
                             value.string.buffer,
                             strerror(errno));
                result = false;
            }
        inlineCFileCleanup:
            ValueFree(&value);
            unmapFile(fd, ptr, size);
        } break;
        case InstructionType_InlineC: {
            result = EvaluateExpression(
              &instruction->expression, state, &value, allocator);
            if (!result) {
                break;
            }
            if (value.type != ValueType_String) {
                TemLangError("Inlined C must be a string. Got '%s'",
                             ValueTypeToString(value.type));
                result = false;
                break;
            }

            TemLangStringAppend(output, &value.string);
            TemLangStringAppendChar(output, '\n');
        } break;
        case InstructionType_InlineCFunction: {
            State temp = { 0 };
            temp.parent = state;
            temp.atoms.allocator = allocator;

            result = EvaluateExpression(
              &instruction->inlinedFunctionName, &temp, &value, allocator);
            if (!result) {
                result = false;
                break;
            }
            if (value.type != ValueType_String) {
                TemLangError(
                  "Inline C function definition must be a string. Got '%s'",
                  ValueTypeToString(value.type));
                result = false;
                break;
            }
            TemLangStringAppend(output, &value.string);

            const size_t targetScopeNumber = scopeNumber;
            ++scopeNumber;
            size_tListAppend(&returnValueScopes, &targetScopeNumber);
            TemLangStringAppendChars(output, "{\n");

            {
                Expression e = { .type = ExpressionType_UnaryScope,
                                 .instructions =
                                   instruction->inlinedFunctionInstructions };
                result = EvaluateExpression(&e, &temp, &value, allocator);
                if (!result) {
                    break;
                }
            }
            if (value.type == ValueType_Null) {
                const VariableTarget target = { .type =
                                                  VariableTarget_ReturnValue };
                result =
                  CompileInstructions(&instruction->inlinedFunctionInstructions,
                                      allocator,
                                      target,
                                      &temp,
                                      output);
                COMPILE_STATE_CLEANUP(temp, (*output));
                TemLangStringAppendFormat((*output), "\n}");
            } else {
                TemLangStringCreateFormat(returnValueName,
                                          allocator,
                                          "returnValue%zu",
                                          instruction->source.lineNumber);
                const VariableTarget target = { .type = VariableTarget_Variable,
                                                .name = &returnValueName };
                TemLangString s = CompilerDeclareValueType(
                  &returnValueName, &temp, allocator, &value, true);
                result =
                  TemLangStringAppend(output, &s) &&
                  CompileInstructions(&instruction->inlinedFunctionInstructions,
                                      allocator,
                                      target,
                                      &temp,
                                      output);
                COMPILE_STATE_CLEANUP(temp, (*output));
                TemLangStringAppendFormat((*output),
                                          "scope%zu:\nreturn %s;\n}",
                                          targetScopeNumber,
                                          returnValueName.buffer);
                TemLangStringFree(&returnValueName);
                TemLangStringFree(&s);
            }
            size_tListPop(&returnValueScopes);
        } break;
        case InstructionType_InlineCFunctionReturnStruct: {
            TemLangString temp = { .allocator = allocator };
            result = CompileCFunctionReturnValue(
              state,
              allocator,
              &instruction->inlinedFunctionName,
              &instruction->inlinedFunctionArgs,
              NULL,
              &instruction->inlinedFunctionInstructions,
              NULL,
              &temp,
              output,
              true);
            TemLangStringFree(&temp);
        } break;
        case InstructionType_InlineVariable: {
            TemLangStringAppendFormat((*output),
                                      "// Inlined variable '%s'\n",
                                      instruction->definitionName.buffer);
        } break;
        case InstructionType_InlineCHeaders: {
            TemLangStringAppendFormat(
              (*output),
              "%s#include <Includes.h>\n#include <Serialize.h>\n",
              useCGLM ? "#define TEMLANG_USE_CGLM 1\n" : "");
        } break;
        case InstructionType_ConvertContainer: {
            result = EvaluateExpression(
              &instruction->fromContainer, state, &value, allocator);
            if (!result) {
                break;
            }
            if (instruction->toArray == value.list.isArray) {
                TemLangString s =
                  CompilerAssignValue(state,
                                      &instruction->toContainer,
                                      allocator,
                                      &value,
                                      &instruction->fromContainer,
                                      true);
                TemLangStringAppend(output, &s);
                TemLangStringFree(&s);
            } else {
                TemLangString name = CompilerGetFullVariableName(
                  &instruction->fromContainer, state, allocator);
                value.list.isArray = !value.list.isArray;
                TemLangString s = CompilerDeclareValueType(
                  &instruction->toContainer, state, allocator, &value, true);
                TemLangStringAppend(output, &s);
                TemLangStringFree(&s);

                if (value.list.isArray) {
                    TemLangStringAppendFormat(
                      (*output),
                      "for(size_t i%zu = 0; i%zu < %s.used; ++i%zu){",
                      variableId,
                      variableId,
                      name.buffer,
                      variableId);
                    TemLangStringCreateFormat(left,
                                              allocator,
                                              "%s[i%zu]",
                                              instruction->toContainer.buffer,
                                              variableId);
                    TemLangStringCreateFormat(right,
                                              allocator,
                                              "%s.buffer[i%zu]",
                                              name.buffer,
                                              variableId);
                    Expression e = { .type = ExpressionType_UnaryVariable,
                                     .identifier = right };
                    s = CompilerAssignValue(state,
                                            &left,
                                            allocator,
                                            value.list.exampleValue,
                                            &e,
                                            false);

                    TemLangStringFree(&left);
                    TemLangStringFree(&right);

                    TemLangStringAppendFormat((*output), "%s }", s.buffer);
                } else {
                    s = CompilerDeclareValueType(
                      NULL, state, allocator, value.list.exampleValue, false);
                    TemLangStringAppendFormat(
                      (*output),
                      "for(size_t i%zu = 0; i%zu < %u; ++i%zu){"
                      "%sListAppend(&%s, &%s[i%zu]);}",
                      variableId,
                      variableId,
                      value.list.values.used,
                      variableId,
                      s.buffer,
                      instruction->toContainer.buffer,
                      name.buffer,
                      variableId);
                }
                ++variableId;
                TemLangStringFree(&s);
                TemLangStringFree(&name);
            }
        } break;
        case InstructionType_Run: {
            const StateFindArgs args = { .log = true, .searchParent = true };
            const Atom* atom = StateFindAtomConst(
              state, &instruction->procedureName, AtomType_Function, args);
            if (atom == NULL) {
                result = false;
                break;
            }
            const VariableTarget target = { .type = VariableTarget_None };
            TemLangStringAppendChars(output, "{\n");
            State temp = { 0 };
            result = StateCopy(&temp, state, allocator) &&
                     CompileInstructions(&atom->functionDefinition.instructions,
                                         allocator,
                                         target,
                                         &temp,
                                         output);
            COMPILE_COPIED_STATE_CLEANUP((*state), temp, (*output));
            TemLangStringAppendFormat((*output),
                                      "goto scope%zu;scope%zu:(void)NULL;\n}",
                                      scopeNumber,
                                      scopeNumber);
            ++scopeNumber;
        } break;
        case InstructionType_Match: {
            Expression temp = { .type = ExpressionType_UnaryMatch };
            temp.matchExpression =
              (pMatchExpression)&instruction->matchInstruction.expression;
            const VariableTarget target = { .type = VariableTarget_None };
            TemLangString s =
              CompilerGetExpression(state, allocator, target, &value, &temp);
            TemLangStringAppend(output, &s);
            TemLangStringFree(&s);
        } break;
        case InstructionType_Iterate: {
            TemLangStringAppendChars(output, "//Iterate instruction\n{\n");
            const CaptureInstruction* cIn = &instruction->captureInstruction;
            Value newValue = { 0 };
            EvaluateExpression(&cIn->target, state, &newValue, allocator);
            TemLangString listName =
              CompilerGetFullVariableName(&cIn->target, state, allocator);
            State temp = { 0 };
            StateCopy(&temp, state, allocator);
            switch (newValue.type) {
                case ValueType_Number: {
                    {
                        TemLangString name =
                          TemLangStringCreate("item", allocator);
                        // Don't care if 'item' already exists. Outer 'item'
                        // can't be used anyway.
                        /*result = */ StateAddValue(&temp, &name, &newValue);
                        TemLangStringFree(&name);
                    }
                    TemLangString s = TemLangStringCreate("", allocator);
                    TemLangString name =
                      TemLangStringCreate("continueLoop", allocator);
                    const VariableTarget newTarget = {
                        .type = VariableTarget_Variable, .name = &name
                    };
                    result = CompileInstructions(
                      &cIn->instructions, allocator, newTarget, &temp, &s);
                    TemLangString cleanupString =
                      TemLangStringCreate("", allocator);
                    COMPILE_COPIED_STATE_CLEANUP((*state), temp, cleanupString);
                    if (result) {
                        TemLangStringAppendFormat(
                          (*output),
                          "bool continueLoop = true;\nfor(int64_t item = "
                          "%" PRId64 "; continueLoop && item < %" PRId64
                          "; ++item){\n %s %s \n}",
                          NumberToInt(&newValue.rangedNumber.range.min),
                          NumberToInt(&newValue.rangedNumber.range.max),
                          s.buffer,
                          cleanupString.buffer);
                    } else {
                        TemLangStringAppendChars(
                          output, "// Failed to compile iterate loop\n");
                    }
                    TemLangStringFree(&cleanupString);
                    TemLangStringFree(&name);
                    TemLangStringFree(&s);
                } break;
                case ValueType_List: {
                    TemLangString declareString = { 0 };
                    {
                        Value item = { .type = ValueType_Number,
                                       .rangedNumber = {
                                         .hasRange = false,
                                         .number = NumberFromUInt(0UL) } };

                        TemLangString name =
                          TemLangStringCreate("index", allocator);
                        result &= StateAddValue(&temp, &name, &item);
                        TemLangStringFree(&name);
                    }
                    TemLangString expString = { 0 };
                    {
                        TemLangString name =
                          TemLangStringCreate("item", allocator);
                        result &= StateAddValue(
                          &temp, &name, newValue.list.exampleValue);
                        declareString =
                          CompilerDeclareValueType(&name,
                                                   state,
                                                   allocator,
                                                   newValue.list.exampleValue,
                                                   false);
                        TemLangStringCreateFormat(getValue,
                                                  allocator,
                                                  newValue.list.isArray
                                                    ? "%s[index]"
                                                    : "%s.buffer[index]",
                                                  listName.buffer);
                        Expression e = { .type = ExpressionType_UnaryVariable,
                                         .identifier = getValue };
                        VariableTarget target = { .type =
                                                    VariableTarget_Variable,
                                                  .name = &name };
                        expString =
                          CompilerGetExpression(state,
                                                allocator,
                                                target,
                                                newValue.list.exampleValue,
                                                &e);
                        TemLangStringFree(&name);
                        TemLangStringFree(&getValue);
                    }
                    TemLangString name =
                      TemLangStringCreate("continueLoop", allocator);
                    const VariableTarget newTarget = {
                        .type = VariableTarget_Variable, .name = &name
                    };
                    TemLangString s = { .allocator = allocator };
                    result &= CompileInstructions(
                      &cIn->instructions, allocator, newTarget, &temp, &s);
                    TemLangString cleanupString =
                      TemLangStringCreate("", allocator);
                    COMPILE_COPIED_STATE_CLEANUP((*state), temp, cleanupString);

                    if (result) {
                        if (newValue.list.isArray) {
                            TemLangStringAppendFormat(
                              (*output),
                              "%s bool continueLoop = true;\nfor(size_t "
                              "index = 0UL; continueLoop && index < %u; "
                              "++index){\n %s %s %s \n}",
                              declareString.buffer,
                              newValue.list.values.used,
                              expString.buffer,
                              s.buffer,
                              cleanupString.buffer);
                        } else {
                            TemLangStringAppendFormat(
                              (*output),
                              "%s bool continueLoop = true;\nfor(size_t "
                              "index = 0UL; continueLoop && index < %s.used; "
                              "++index){\n %s %s %s "
                              "\n}",
                              declareString.buffer,
                              listName.buffer,
                              expString.buffer,
                              s.buffer,
                              cleanupString.buffer);
                        }
                    }
                    TemLangStringFree(&cleanupString);
                    TemLangStringFree(&declareString);
                    TemLangStringFree(&expString);
                    TemLangStringFree(&name);
                    TemLangStringFree(&s);
                } break;
                case ValueType_Enum: {
                    {
                        TemLangString name =
                          TemLangStringCreate("item", allocator);
                        if (!StateAddValue(&temp, &name, &newValue)) {
                            TemLangError("Compiler error! Check (%s:%zu)",
                                         __FILE__,
                                         __LINE__);
                        }
                        TemLangStringFree(&name);
                    }
                    TemLangString s = TemLangStringCreate("", allocator);
                    TemLangString name =
                      TemLangStringCreate("continueLoop", allocator);
                    const VariableTarget newTarget = {
                        .type = VariableTarget_Variable, .name = &name
                    };
                    result &= CompileInstructions(
                      &cIn->instructions, allocator, newTarget, &temp, &s);
                    TemLangString cleanupString =
                      TemLangStringCreate("", allocator);
                    COMPILE_COPIED_STATE_CLEANUP((*state), temp, cleanupString);
                    const StateFindArgs args = { .log = true,
                                                 .searchParent = true };
                    const Atom* atom = StateFindAtomConst(
                      state, &newValue.enumValue.name, AtomType_Enum, args);
                    result = atom != NULL;

                    const Range range = {
                        .min = { .type = NumberType_Signed, .i = -1L },
                        .max = { .type = NumberType_Signed,
                                 .i = atom->enumDefinition.members.used }
                    };
                    const char* c = CTypeToTypeString(RangeToCType(&range));
                    if (result) {
                        TemLangStringAppendFormat(
                          (*output),
                          "bool continueLoop = true;\nfor(%s item = %s_%s; "
                          "continueLoop && item < %u; ++item){\n %s %s \n}",
                          c,
                          newValue.enumValue.name.buffer,
                          newValue.enumValue.value.buffer,
                          atom->enumDefinition.members.used,
                          s.buffer,
                          cleanupString.buffer);
                    }
                    TemLangStringFree(&cleanupString);
                    TemLangStringFree(&name);
                    TemLangStringFree(&s);
                } break;
                default:
                    TemLangError("Cannot run iterate on value type '%s'",
                                 ValueTypeToString(newValue.type));
                    break;
            }
            ValueFree(&newValue);
            TemLangStringFree(&listName);
            TemLangStringAppendChars(output, "\n}");
        } break;
        case InstructionType_DefineFunction: {
            TemLangStringAppendFormat((*output),
                                      "\n//Function: %s\n",
                                      instruction->functionName.buffer);
        } break;
        case InstructionType_ListModify: {
            TemLangString s =
              CompileListModify(state, allocator, &instruction->listModify);
            TemLangStringAppend(output, &s);
            TemLangStringFree(&s);
        } break;
        case InstructionType_Return: {
            TemLangString s = CompilerGetExpression(
              state, allocator, target, &value, &instruction->expression);
            TemLangStringAppend(output, &s);
            TemLangStringFree(&s);
        } break;
        case InstructionType_IfReturn: {
            GET_RETURN_VALUE_SCOPE({ break; });
            TemLangString name = TemLangStringCreate("doReturn", allocator);

            TemLangStringAppendChars(output, "{bool doReturn = false;{");
            const VariableTarget tempTarget = { .type = VariableTarget_Variable,
                                                .name = &name };
            TemLangString condition = CompilerGetExpression(
              state, allocator, tempTarget, &value, &instruction->ifCondition);
            {
                TemLangStringAppendFormat((*output), "%s}", condition.buffer);
            }
            TemLangStringFree(&condition);

            if (target.type == VariableTarget_None) {
                TemLangStringAppendFormat(
                  (*output), "if(doReturn){ goto scope%zu; }}", returnScope);
            } else {
                condition = CompilerGetExpression(
                  state, allocator, target, &value, &instruction->ifResult);
                TemLangStringAppendFormat((*output),
                                          "if(doReturn){ %s goto scope%zu; }}",
                                          condition.buffer,
                                          returnScope);
            }
            TemLangStringFree(&condition);
            TemLangStringFree(&name);
        } break;
        case InstructionType_Verify: {
            const StateFindArgs args = { .log = true, .searchParent = true };
            const Atom* atom =
              StateFindAnyAtomConst(state, &instruction->verifyName, args);
            if (atom == NULL) {
                return false;
            }
            TemLangString s = AtomToString(atom, allocator);
            TemLangStringAppendFormat(
              (*output), "\n//Verified atom: %s\n", s.buffer);
            TemLangStringFree(&s);
        } break;
        case InstructionType_Print:
        case InstructionType_Error:
        case InstructionType_Format:
        case InstructionType_NoCompile: {
            TemLangStringAppendFormat(
              (*output),
              "\n//%s\n",
              InstructionTypeToString(instruction->type));
        } break;
        case InstructionType_NoCleanup: {
            State temp = { 0 };
            StateCopy(&temp, state, allocator);
            TemLangStringAppendChars(output, "\n//Start of No Cleanup scope\n");
            ++noCleanupState;
            result = CompileInstructions(
              &instruction->instructions, allocator, target, &temp, output);
            TemLangStringAppendChars(output, "\n//End of No Cleanup scope\n");
            --noCleanupState;
            StateFree(&temp);
        } break;
        case InstructionType_NumberRound: {
            if (!EvaluateExpression(
                  &instruction->numberRoundTarget, state, &value, allocator)) {
                break;
            }
            if (value.type != ValueType_Number) {
                break;
            }
            bool isFloat;
            {
                const CType cType =
                  value.rangedNumber.hasRange
                    ? RangeToCType(&value.rangedNumber.range)
                    : NumberToCType(&value.rangedNumber.number);
                switch (cType) {
                    case CType_f32:
                        isFloat = true;
                        break;
                    case CType_f64:
                        isFloat = false;
                        break;
                    default:
                        goto cleanup;
                }
            }
            const char* c = NULL;
            switch (instruction->numberRound) {
                case NumberRound_Ceil:
                    c = isFloat ? "ceilf" : "ceil";
                    break;
                case NumberRound_Floor:
                    c = isFloat ? "floorf" : "floor";
                    break;
                case NumberRound_Round:
                    c = isFloat ? "roundf" : "round";
                    break;
                default:
                    goto cleanup;
            }
            TemLangString targetName = CompilerGetFullVariableName(
              &instruction->numberRoundTarget, state, allocator);
            TemLangString typeName = StructMemberToTypeName(
              &instruction->numberRoundMember, allocator);
            TemLangStringAppendFormat((*output),
                                      "%s %s = %s(%s);",
                                      typeName.buffer,
                                      instruction->numberRoundName.buffer,
                                      c,
                                      targetName.buffer);
            TemLangStringFree(&targetName);
            TemLangStringFree(&typeName);
        } break;
        default:
            TemLangStringAppendFormat(
              (*output),
              "\n// Did not compile instruction '%s'\n",
              InstructionTypeToString(instruction->type));
            TemLangError("Instruction '%s' cannot be compiled",
                         InstructionTypeToString(instruction->type));
            break;
    }
cleanup:
    ValueFree(&value);
    if (!result) {
        TemLangError("Failed to complie instruction (%s:%zu)",
                     instruction->source.source.buffer,
                     instruction->source.lineNumber);
    }
    return result;
}

static inline bool
ExpressionIsSimple(const Expression* e)
{
    switch (e->type) {
        case ExpressionType_UnaryVariable:
            return true;
        case ExpressionType_UnaryValue:
            switch (e->value.type) {
                case ValueType_Number:
                case ValueType_Boolean:
                case ValueType_Enum:
                    return true;
                default:
                    break;
            }
            break;
        case ExpressionType_Binary:
            if (ExpressionIsSimple(e->left) && ExpressionIsSimple(e->right)) {
                switch (e->op.type) {
                    case OperatorType_Number:
                    case OperatorType_Boolean:
                    case OperatorType_Comparison:
                        return true;
                    default:
                        break;
                }
            }
            break;
        default:
            break;
    }
    return false;
}

static inline bool
GetSimpleExpressionName(const State* state,
                        const Allocator* allocator,
                        const Expression* e,
                        pTemLangString s)
{
    switch (e->type) {
        case ExpressionType_UnaryValue: {
            switch (e->value.type) {
                case ValueType_Number:
                case ValueType_Boolean:
                case ValueType_Enum: {
                    TemLangString s1 = CompilerGetValue(&e->value, allocator);
                    TemLangStringAppend(s, &s1);
                    TemLangStringFree(&s1);
                    return true;
                } break;
                default:
                    break;
            }
        } break;
        case ExpressionType_UnaryVariable: {
            return TemLangStringCopy(s, &e->identifier, allocator);
        } break;
        case ExpressionType_Binary: {
            bool result = true;
            TemLangString s1 = { .allocator = allocator };
            TemLangString s2 = { .allocator = allocator };
            Value left = { 0 };
            Value right = { 0 };

            result = EvaluateExpression(e->left, state, &left, allocator) &&
                     EvaluateExpression(e->right, state, &right, allocator);
            if (!result) {
                goto end;
            }

            if (e->left->op.type == OperatorType_Get &&
                TryCompilerGetFullVariableName(
                  e->left, state, allocator, &s1)) {
            } else if (ExpressionIsSimple(e->left)) {
                GetSimpleExpressionName(state, allocator, e->left, &s1);
            } else {
                result = false;
                goto end;
            }

            if (e->right->op.type == OperatorType_Get &&
                TryCompilerGetFullVariableName(
                  e->right, state, allocator, &s2)) {
            } else if (ExpressionIsSimple(e->right)) {
                GetSimpleExpressionName(state, allocator, e->right, &s2);
            } else {
                result = false;
                goto end;
            }

            switch (e->op.type) {
                case OperatorType_Number: {
                    if (e->op.numberOperator == NumberOperator_Modulo &&
                        (left.rangedNumber.number.type == NumberType_Float ||
                         right.rangedNumber.number.type == NumberType_Float)) {
                        TemLangStringAppendFormat(
                          (*s), "fmod(%s,%s);", s1.buffer, s2.buffer);
                        break;
                    }
                    TemLangStringAppendFormat(
                      (*s),
                      "(%s %c %s)",
                      s1.buffer,
                      NumberOperatorToChar(e->op.numberOperator),
                      s2.buffer);
                } break;
                case OperatorType_Boolean: {
                    if (e->op.booleanOperator == BooleanOperator_Not) {
                        const Value* value = getUnaryValue(&left, &right);
                        if (value != NULL) {
                            TemLangStringAppendFormat(
                              (*s),
                              "!(%s)",
                              value == &left ? s1.buffer : s2.buffer);
                        }
                    } else {
                        TemLangStringAppendFormat(
                          (*s),
                          "(%s %s %s)",
                          s1.buffer,
                          BooleanOperatorToChars(e->op.booleanOperator),
                          s2.buffer);
                    }
                } break;
                case OperatorType_Comparison: {
                    TemLangStringAppendFormat(
                      (*s),
                      "(%s %s %s)",
                      s1.buffer,
                      ComparisonOperatorToChars(e->op.comparisonOperator),
                      s2.buffer);
                } break;
                default:
                    TemLangError(
                      "Compiler error! Check (%s:%zu)", __FILE__, __LINE__);
                    break;
            }
        end:
            TemLangStringFree(&s1);
            TemLangStringFree(&s2);
            ValueFree(&left);
            ValueFree(&right);
            return result;
        } break;
        default:
            break;
    }
    return false;
}

static inline TemLangString
CompilerGetNumberExpression(const State* state,
                            const Allocator* allocator,
                            const VariableTarget target,
                            const Value* left,
                            const Value* right,
                            const Value* realValue,
                            const Expression* e)
{
    TemLangString s = TemLangStringCreate("", allocator);
    if (left->type == ValueType_Flag && right->type == ValueType_Flag) {
        const size_t total =
          left->flagValue.members.used + right->flagValue.members.used;
        if (total > 0) {
            TemLangString temp = { .allocator = allocator };
            size_t j = 0;
            for (size_t i = 0; i < left->flagValue.members.used; ++i, ++j) {
                TemLangStringAppendFormat(
                  temp,
                  "%s_%s",
                  left->flagValue.name.buffer,
                  left->flagValue.members.buffer[i].buffer);
                if (i + j != total - 1) {
                    TemLangStringAppendChar(&temp, '|');
                }
            }
            for (size_t i = 0; i < right->flagValue.members.used; ++i, ++j) {
                TemLangStringAppendFormat(
                  temp,
                  "%s_%s",
                  right->flagValue.name.buffer,
                  right->flagValue.members.buffer[i].buffer);
                if (i + j != total - 1) {
                    TemLangStringAppendChar(&temp, '|');
                }
            }
            switch (target.type) {
                case VariableTarget_ReturnValue:
                    TemLangStringAppendFormat(s, "return %s;", temp.buffer);
                    break;
                case VariableTarget_Variable:
                    TemLangStringAppendFormat(
                      s, "%s = %s;", target.name->buffer, temp.buffer);
                    break;
                default:
                    TemLangStringAppendFormat(s, "%s", temp.buffer);
                    break;
            }
            TemLangStringFree(&temp);
        }
    } else if (left->type == ValueType_Number &&
               right->type == ValueType_Number) {
        TemLangString s1 = { .allocator = allocator };
        TemLangString s2 = { .allocator = allocator };

        if (!GetSimpleExpressionName(state, allocator, e->left, &s1) ||
            !GetSimpleExpressionName(state, allocator, e->right, &s2)) {
            MAKE_LEFT_AND_RIGHT((*left), (*right));
            TemLangStringCopy(&s1, &ls, allocator);
            TemLangStringCopy(&s2, &rs, allocator);
            TemLangStringFree(&ls);
            TemLangStringFree(&rs);
        }

        if (e->op.numberOperator == NumberOperator_Modulo &&
            (left->rangedNumber.number.type == NumberType_Float ||
             right->rangedNumber.number.type == NumberType_Float)) {
            switch (target.type) {
                case VariableTarget_ReturnValue: {
                    TemLangStringAppendFormat(
                      s, "return fmod(%s, %s);", s1.buffer, s2.buffer);
                } break;
                case VariableTarget_Variable: {
                    TemLangStringAppendFormat(s,
                                              "%s = fmod(%s,%s);",
                                              target.name->buffer,
                                              s1.buffer,
                                              s2.buffer);
                } break;
                case VariableTarget_None:
                default: {
                    TemLangStringAppendFormat(
                      s, "fmod(%s,%s)", s1.buffer, s2.buffer);
                } break;
            }
        } else {
            switch (target.type) {
                case VariableTarget_ReturnValue: {
                    TemLangStringAppendFormat(
                      s,
                      "return %s %c %s;",
                      s1.buffer,
                      NumberOperatorToChar(e->op.numberOperator),
                      s2.buffer);
                } break;
                case VariableTarget_Variable: {
                    TemLangStringAppendFormat(
                      s,
                      "%s = %s %c %s;",
                      target.name->buffer,
                      s1.buffer,
                      NumberOperatorToChar(e->op.numberOperator),
                      s2.buffer);
                } break;
                case VariableTarget_None:
                default: {
                    TemLangStringAppendFormat(
                      s,
                      "%s %c %s",
                      s1.buffer,
                      NumberOperatorToChar(e->op.numberOperator),
                      s2.buffer);
                } break;
            }
        }
        TemLangStringFree(&s1);
        TemLangStringFree(&s2);
    } else if (left->type == ValueType_String &&
               right->type == ValueType_String) {
        MAKE_LEFT_AND_RIGHT((*left), (*right));
        switch (target.type) {
            case VariableTarget_ReturnValue: {
                TemLangStringAppendFormat(
                  s,
                  "return TemLangStringCombine(&%s, &%s, currentAllocator);",
                  ls.buffer,
                  rs.buffer);
            } break;
            case VariableTarget_Variable: {
                TemLangStringAppendFormat(
                  s,
                  "%s.used = 0; TemLangStringAppend(&%s, "
                  "&%s);TemLangStringAppend(&%s, &%s);",
                  target.name->buffer,
                  target.name->buffer,
                  ls.buffer,
                  target.name->buffer,
                  rs.buffer);
            } break;
            case VariableTarget_None:
            default: {
                TemLangStringAppendFormat(
                  s,
                  "TemLangStringCombine(&%s, &%s, currentAllocator)",
                  ls.buffer,
                  rs.buffer);
            } break;
        }
        TemLangStringFree(&ls);
        TemLangStringFree(&rs);
    } else if (left->type == ValueType_Null &&
               right->type == ValueType_Number &&
               e->op.numberOperator == NumberOperator_Subtract) {
        TemLangString s1 =
          NumberToString(&right->rangedNumber.number, allocator);
        switch (target.type) {
            case VariableTarget_ReturnValue:
                TemLangStringAppendFormat(s, "return -%s;", s1.buffer);
                break;
            case VariableTarget_Variable:
                TemLangStringAppendFormat(
                  s, "%s = -%s;", target.name->buffer, s1.buffer);
                break;
            case VariableTarget_None:
            default:
                TemLangStringAppendFormat(s, "-%s", s1.buffer);
                break;
        }
        TemLangStringFree(&s1);
    } else if (e->op.numberOperator == NumberOperator_Multiply) {
        if (target.type != VariableTarget_Variable) {
            goto expressionCompileError;
        }
        uint64_t count = 0;
        const Value* value = NULL;
        const Expression* realE = NULL;
        if (left->type == ValueType_Number) {
            if (!NumberTryToUInt(&left->rangedNumber.number, &count)) {
                goto expressionCompileError;
            }
            value = right;
            realE = e->right;
        } else if (right->type == ValueType_Number) {
            if (!NumberTryToUInt(&right->rangedNumber.number, &count)) {
                goto expressionCompileError;
            }
            value = left;
            realE = e->left;
        } else {
            goto expressionCompileError;
        }
        for (size_t i = 0; i < count; ++i) {
            TemLangStringCreateFormat(
              name, allocator, "%s[%zu]", target.name->buffer, i);
            TemLangString o =
              CompilerAssignValue(state, &name, allocator, value, realE, false);
            TemLangStringAppend(&s, &o);
            TemLangStringFree(&o);
            TemLangStringFree(&name);
        }
    } else {
        {
            const Value* numberValue = NULL;
            const Value* enumValue = NULL;
            if (ValueTypesTryMatch(left,
                                   ValueType_Enum,
                                   right,
                                   ValueType_Number,
                                   &enumValue,
                                   &numberValue)) {
                const StateFindArgs args = { .log = true,
                                             .searchParent = true };
                const Atom* atom = StateFindAtomConst(
                  state, &enumValue->enumValue.name, AtomType_Enum, args);
                if (atom == NULL) {
                    goto expressionCompileError;
                }
                TemLangString s1 = CompilerGetFullVariableName(
                  enumValue == left ? e->left : e->right, state, allocator);
                TemLangStringCreateFormat(s3, allocator, "temp%zu", variableId);
                ++variableId;
                TemLangString s2 =
                  CompilerAssignValue(state,
                                      &s3,
                                      allocator,
                                      numberValue,
                                      numberValue == left ? e->left : e->right,
                                      true);
                TemLangStringAppendFormat(s, "{ %s", s2.buffer);
                switch (target.type) {
                    case VariableTarget_Variable:
                        TemLangStringAppendFormat(
                          s,
                          "%s = (%s + %s + %u) %% %u;}",
                          target.name->buffer,
                          s1.buffer,
                          s3.buffer,
                          atom->enumDefinition.members.used,
                          atom->enumDefinition.members.used);
                        break;
                    case VariableTarget_ReturnValue:
                        TemLangStringAppendFormat(
                          s,
                          "return (%s + %s + %u) %% %u;}",
                          s1.buffer,
                          s3.buffer,
                          atom->enumDefinition.members.used,
                          atom->enumDefinition.members.used);
                        break;
                    default:
                        TemLangStringAppendFormat(
                          s,
                          "(%s + %s + %u) %% %u;}",
                          s1.buffer,
                          s3.buffer,
                          atom->enumDefinition.members.used,
                          atom->enumDefinition.members.used);
                        break;
                }
                TemLangStringFree(&s1);
                TemLangStringFree(&s2);
                TemLangStringFree(&s3);
            }
        }
        const Value* value = NULL;
        const Value* typeValue = NULL;
        if (ValueTypesTryMatch(left,
                               ValueType_Enum,
                               right,
                               ValueType_Type,
                               &value,
                               &typeValue)) {
            const Expression* targetE = value == left ? e->left : e->right;
            if (targetE->type == ExpressionType_UnaryValue) {
                switch (typeValue->fakeValue->type) {
                    case ValueType_Number: {
                        const char* c = CTypeToTypeString(RangeToCType(
                          &typeValue->fakeValue->rangedNumber.range));
                        switch (target.type) {
                            case VariableTarget_ReturnValue:
                                TemLangStringAppendFormat(
                                  s,
                                  " return (%s)%s_%s;",
                                  c,
                                  value->enumValue.name.buffer,
                                  value->enumValue.value.buffer);
                                break;
                            case VariableTarget_Variable:
                                TemLangStringAppendFormat(
                                  s,
                                  "%s = (%s)%s_%s;",
                                  target.name->buffer,
                                  c,
                                  value->enumValue.name.buffer,
                                  value->enumValue.value.buffer);
                                break;
                            default:
                                TemLangStringAppendFormat(
                                  s,
                                  " (%s)%s_%s",
                                  c,
                                  value->enumValue.name.buffer,
                                  value->enumValue.value.buffer);
                                break;
                        }
                    } break;
                    case ValueType_String: {
                        switch (target.type) {
                            case VariableTarget_ReturnValue:
                                TemLangStringAppendFormat(
                                  s,
                                  "return \"%s\";",
                                  value->enumValue.value.buffer);
                                break;
                            case VariableTarget_Variable:
                                TemLangStringAppendFormat(
                                  s,
                                  "%s.used = 0; TemLangStringAppendChars(&%s, "
                                  "\"%s\");",
                                  target.name->buffer,
                                  target.name->buffer,
                                  value->enumValue.value.buffer);
                                break;
                            default:
                                TemLangStringAppendFormat(
                                  s, "\"%s\"", value->enumValue.value.buffer);
                                break;
                        }
                    } break;
                    default: {
                        goto expressionCompileError;
                    } break;
                }
            } else {
                TemLangStringCreateFormat(
                  temp, allocator, "temp%zu", variableId);
                ++variableId;
                TemLangString s1 = CompilerAssignValue(
                  state, &temp, allocator, value, targetE, true);
                switch (typeValue->fakeValue->type) {
                    case ValueType_Number: {
                        const char* c = CTypeToTypeString(RangeToCType(
                          &typeValue->fakeValue->rangedNumber.range));
                        switch (target.type) {
                            case VariableTarget_ReturnValue:
                                TemLangStringAppendFormat(s,
                                                          " %s return (%s)%s;",
                                                          s1.buffer,
                                                          c,
                                                          temp.buffer);
                                break;
                            case VariableTarget_Variable:
                                TemLangStringAppendFormat(s,
                                                          " %s %s = (%s)%s;",
                                                          s1.buffer,
                                                          target.name->buffer,
                                                          c,
                                                          temp.buffer);
                                break;
                            default:
                                break;
                        }
                    } break;
                    case ValueType_String: {
                        switch (target.type) {
                            case VariableTarget_Variable:
                                TemLangStringAppendFormat(
                                  s,
                                  "%s %s = %sToString(%s);",
                                  s1.buffer,
                                  target.name->buffer,
                                  value->enumValue.name.buffer,
                                  temp.buffer);
                                break;
                            case VariableTarget_ReturnValue:
                                TemLangStringAppendFormat(
                                  s,
                                  "%s return %sToString(%s);",
                                  s1.buffer,
                                  value->enumValue.name.buffer,
                                  temp.buffer);
                                break;
                            default:
                                TemLangStringAppendFormat(
                                  s,
                                  "%s %sToString(%s)",
                                  s1.buffer,
                                  value->enumValue.name.buffer,
                                  temp.buffer);
                                break;
                        }

                    } break;
                    default: {
                        goto expressionCompileError;
                    } break;
                }
                TemLangStringFree(&s1);
                TemLangStringFree(&temp);
            }
        } else if (ValueTypesTryMatch(left,
                                      ValueType_Variant,
                                      right,
                                      ValueType_Type,
                                      &value,
                                      &typeValue)) {
            TemLangString s1 = CompilerGetFullVariableName(
              value == left ? e->left : e->right, state, allocator);
            switch (typeValue->fakeValue->type) {
                case ValueType_String: {
                    switch (target.type) {
                        case VariableTarget_ReturnValue:
                            TemLangStringAppendFormat(
                              s,
                              "return %sTagToString(%s.tag, currentAllocator);",
                              value->variantValue.name.buffer,
                              s1.buffer);
                            break;
                        case VariableTarget_Variable:
                            TemLangStringAppendFormat(
                              s,
                              "%s.used = 0; TemLangStringAppendChars(&%s, "
                              "%sTagToCharString(%s.tag));",
                              target.name->buffer,
                              target.name->buffer,
                              value->variantValue.name.buffer,
                              s1.buffer);
                            break;
                        default:
                            TemLangStringAppendFormat(
                              s,
                              "%sTagToString(%s.tag, currentAllocator);",
                              value->variantValue.name.buffer,
                              s1.buffer);
                            break;
                    }
                } break;
                default:
                    break;
            }
            TemLangStringFree(&s1);
        } else if (ValueTypesTryMatch(left,
                                      ValueType_Number,
                                      right,
                                      ValueType_Type,
                                      &value,
                                      &typeValue) ||
                   ValueTypesTryMatch(left,
                                      ValueType_Struct,
                                      right,
                                      ValueType_Type,
                                      &value,
                                      &typeValue)) {
            bool done = false;
            if (value->type == ValueType_Number) {
                const Expression* targetE = value == left ? e->left : e->right;
                switch (typeValue->fakeValue->type) {
                    case ValueType_String: {
                        if (targetE->type == ExpressionType_UnaryValue) {
                            TemLangString s1 = NumberToString(
                              &value->rangedNumber.number, allocator);
                            switch (target.type) {
                                case VariableTarget_ReturnValue:
                                    TemLangStringAppendFormat(
                                      s,
                                      "return TemLangStringCreate(\"%s\", "
                                      "currentAllocator);",
                                      s1.buffer);
                                    break;
                                case VariableTarget_Variable:
                                    TemLangStringAppendFormat(
                                      s,
                                      "%s.used = 0; "
                                      "TemLangStringAppendChars(&%s, "
                                      "\"%s\");",
                                      target.name->buffer,
                                      target.name->buffer,
                                      s1.buffer);
                                    break;
                                default:
                                    TemLangStringAppendFormat(
                                      s,
                                      "TemLangStringCreate(\"%s\", "
                                      "currentAllocator);",
                                      s1.buffer);
                                    break;
                            }
                            TemLangStringFree(&s1);
                        } else {
                            State temp = { 0 };
                            temp.parent = state;
                            temp.atoms.allocator = allocator;
                            TemLangStringCreateFormat(
                              name, allocator, "temp%zu", variableId);
                            ++variableId;
                            if (!StateAddValue(&temp, &name, value)) {
                                TemLangError("Compiler error! Check (%s:%zu)",
                                             __FILE__,
                                             __LINE__);
                            }
                            {
                                TemLangString s1 = CompilerAssignValue(
                                  &temp,
                                  &name,
                                  allocator,
                                  value,
                                  value == left ? e->left : e->right,
                                  true);
                                TemLangStringAppend(&s, &s1);
                                TemLangStringFree(&s1);
                            }
                            switch (
                              typeValue->fakeValue->rangedNumber.number.type) {
                                case NumberType_Signed:
                                    TemLangStringAppendFormat(
                                      s,
                                      "Number n = "
                                      "NumberFromInt((int64_t)%s);",
                                      name.buffer);
                                    break;
                                case NumberType_Unsigned:
                                    TemLangStringAppendFormat(
                                      s,
                                      "Number n = "
                                      "NumberFromUInt((uint64_t)%s);",
                                      name.buffer);
                                    break;
                                default:
                                    TemLangStringAppendFormat(
                                      s,
                                      "Number n = "
                                      "NumberFromDouble((double)%s);",
                                      name.buffer);
                                    break;
                            }
                            switch (target.type) {
                                case VariableTarget_Variable:
                                    TemLangStringAppendFormat(
                                      s,
                                      "{TemLangString temp2 = "
                                      "NumberToString(&n, "
                                      "currentAllocator); %s.used = 0; "
                                      "TemLangStringAppend(&%s, &temp2); "
                                      "TemLangStringFree(&temp2);}",
                                      target.name->buffer,
                                      target.name->buffer);
                                    break;
                                case VariableTarget_ReturnValue:
                                    TemLangStringAppendFormat(
                                      s,
                                      "return NumberToString(&n, "
                                      "currentAllocator);");
                                    break;
                                default:
                                    TemLangStringAppendFormat(
                                      s,
                                      "NumberToString(&n, "
                                      "currentAllocator);");
                                    break;
                            }
                            StateFree(&temp);
                            TemLangStringFree(&name);
                        }
                        done = true;
                    } break;
                    case ValueType_Enum: {
                        if (targetE->type == ExpressionType_UnaryValue) {
                            const size_t index =
                              NumberToUInt(&value->rangedNumber.number);
                            switch (target.type) {
                                case VariableTarget_Variable:
                                    TemLangStringAppendFormat(
                                      s,
                                      "%s = %zu;",
                                      target.name->buffer,
                                      index);
                                    break;
                                case VariableTarget_ReturnValue:
                                    TemLangStringAppendFormat(
                                      s, "return %zu;", index);
                                    break;
                                default:
                                    TemLangStringAppendFormat(s, "%zu", index);
                                    break;
                            }
                        } else {
                            State temp = { 0 };
                            temp.parent = state;
                            temp.atoms.allocator = allocator;
                            TemLangStringCreateFormat(
                              name, allocator, "temp%zu", variableId);
                            ++variableId;
                            if (!StateAddValue(&temp, &name, value)) {
                                TemLangError("Compiler error! Check (%s:%zu)",
                                             __FILE__,
                                             __LINE__);
                            }
                            {
                                TemLangString s1 = CompilerAssignValue(
                                  &temp,
                                  &name,
                                  allocator,
                                  value,
                                  value == left ? e->left : e->right,
                                  true);
                                TemLangStringAppend(&s, &s1);
                                TemLangStringFree(&s1);
                            }
                            switch (target.type) {
                                case VariableTarget_Variable:
                                    TemLangStringAppendFormat(
                                      s,
                                      "%s = %s;",
                                      target.name->buffer,
                                      name.buffer);
                                    break;
                                case VariableTarget_ReturnValue:
                                    TemLangStringAppendFormat(
                                      s, "return %s;", name.buffer);
                                    break;
                                default:
                                    TemLangStringAppendFormat(
                                      s, "%s", name.buffer);
                                    break;
                            }
                            StateFree(&temp);
                            TemLangStringFree(&name);
                        }
                        done = true;
                    } break;
                    default:
                        break;
                }
            }
            if (!done) {
                TemLangString a =
                  CompilerGetExpression(state,
                                        allocator,
                                        target,
                                        value,
                                        value == left ? e->left : e->right);
                TemLangStringAppend(&s, &a);
                TemLangStringFree(&a);
            }
        } else if (ValueTypesTryMatch(left,
                                      ValueType_String,
                                      right,
                                      ValueType_Type,
                                      &value,
                                      &typeValue)) {
            const Value* fakeValue = typeValue->fakeValue;
            const Expression* valueE = value == left ? e->left : e->right;
            // Check if constant string
            if (valueE->type == ExpressionType_UnaryValue) {
                switch (fakeValue->type) {
                    case ValueType_Enum:
                        switch (target.type) {
                            case VariableTarget_Variable:
                                TemLangStringAppendFormat(
                                  s,
                                  "%s = %s_%s;",
                                  target.name->buffer,
                                  fakeValue->enumValue.name.buffer,
                                  value->string.buffer);
                                break;
                            case VariableTarget_ReturnValue:
                                TemLangStringAppendFormat(
                                  s,
                                  "return %s_%s;",
                                  fakeValue->enumValue.name.buffer,
                                  value->string.buffer);
                                break;
                            default:
                                TemLangStringAppendFormat(
                                  s,
                                  "%s_%s",
                                  fakeValue->enumValue.name.buffer,
                                  value->string.buffer);
                                break;
                        }
                        break;
                    case ValueType_Number: {
                        switch (target.type) {
                            case VariableTarget_Variable:
                                TemLangStringAppendFormat(s,
                                                          "%s = %s;",
                                                          target.name->buffer,
                                                          value->string.buffer);
                                break;
                            case VariableTarget_ReturnValue:
                                TemLangStringAppendFormat(
                                  s, "return %s;", value->string.buffer);
                                break;
                            default:
                                TemLangStringAppendFormat(
                                  s, "%s", value->string.buffer);
                                break;
                        }
                    } break;
                    default:
                        break;
                }
            } else {
                State temp = { 0 };
                temp.parent = state;
                temp.atoms.allocator = allocator;
                {
                    TemLangString name =
                      TemLangStringCreate("string", allocator);
                    if (!StateAddValue(&temp, &name, value)) {
                        TemLangError(
                          "Compiler error! Check (%s:%zu)", __FILE__, __LINE__);
                    }
                    TemLangString s1 =
                      CompilerAssignValue(&temp,
                                          &name,
                                          allocator,
                                          value,
                                          value == left ? e->left : e->right,
                                          true);
                    TemLangStringAppend(&s, &s1);
                    TemLangStringFree(&s1);
                    TemLangStringFree(&name);
                }
                switch (fakeValue->type) {
                    case ValueType_Enum: {
                        switch (target.type) {
                            case VariableTarget_None: {
                                TemLangStringAppendFormat(
                                  s,
                                  "%sFromString(&string)",
                                  fakeValue->enumValue.name.buffer);
                            } break;
                            case VariableTarget_Variable: {
                                TemLangStringAppendFormat(
                                  s,
                                  "%s = %sFromString(&string);",
                                  target.name->buffer,
                                  fakeValue->enumValue.name.buffer);
                            } break;
                            case VariableTarget_ReturnValue: {
                                TemLangStringAppendFormat(
                                  s,
                                  "return %sFromString(&string);",
                                  fakeValue->enumValue.name.buffer);
                            } break;
                            default:
                                break;
                        }
                    } break;
                    case ValueType_Number: {
                        const char* c = CTypeToTypeString(
                          fakeValue->rangedNumber.hasRange
                            ? RangeToCType(&fakeValue->rangedNumber.range)
                            : NumberToCType(&fakeValue->rangedNumber.number));
                        switch (target.type) {
                            case VariableTarget_None: {
                                TemLangStringAppendFormat(
                                  s, "%sFromString(&string)", c);
                            } break;
                            case VariableTarget_Variable: {
                                TemLangStringAppendFormat(
                                  s,
                                  "%s = %sFromString(&string);",
                                  target.name->buffer,
                                  c);
                            } break;
                            case VariableTarget_ReturnValue: {
                                TemLangStringAppendFormat(
                                  s, "return %sFromString(&string);", c);
                            } break;
                            default:
                                break;
                        }
                    } break;
                    default: {
                        TemLangString s1 =
                          CompilerGetValue(realValue, allocator);
                        switch (target.type) {
                            case VariableTarget_None: {
                                TemLangStringAppendFormat(s, "%s", s1.buffer);
                            } break;
                            case VariableTarget_ReturnValue: {
                                TemLangStringAppendFormat(
                                  s, "return %s;", s1.buffer);
                            } break;
                            case VariableTarget_Variable: {
                                Expression tempE = {
                                    .type = ExpressionType_UnaryValue,
                                    .value = *fakeValue
                                };
                                TemLangString s2 =
                                  CompilerAssignValue(&temp,
                                                      target.name,
                                                      allocator,
                                                      realValue,
                                                      &tempE,
                                                      false);
                                TemLangStringAppend(&s, &s2);
                                TemLangStringFree(&s2);
                            } break;
                            default:
                                break;
                        }
                        TemLangStringFree(&s1);
                    } break;
                }
                COMPILE_STATE_CLEANUP(temp, s);
            }
        }
    }

    return s;
expressionCompileError : {
    TemLangString s1 = ValueToString(left, allocator);
    TemLangString s2 = OperatorString(&e->op, allocator);
    TemLangString s3 = ValueToString(right, allocator);
    TemLangError("Failed to compile expression '%s' '%s' '%s'",
                 s1.buffer,
                 s2.buffer,
                 s3.buffer);
    TemLangStringFree(&s1);
    TemLangStringFree(&s2);
    TemLangStringFree(&s3);
    return s;
}
}
