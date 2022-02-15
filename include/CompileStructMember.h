#pragma once

#include "Allocator.h"
#include "StructMember.h"

static bool useCGLM = false;

static inline StructMember
TryGetCGLMName(const StructMember* m,
               const char** c,
               const Allocator* allocator)
{
    StructMember newM = { 0 };
    StructMemberCopy(&newM, m, allocator);
    if (useCGLM && newM.isKeyword && newM.keyword == Keyword_f32) {
        switch (newM.quantity) {
            case 2:
                *c = "vec2";
                break;
            case 3:
                *c = "vec3";
                break;
            case 4:
                *c = "vec4";
                break;
            case 9:
                *c = "mat3";
                break;
            case 16:
                *c = "mat4";
                break;
            default:
                goto end;
        }
        StructMemberFree(&newM);
        newM.quantity = 1;
        newM.isKeyword = false;
        TemLangStringCopy(&newM.name, &m->name, allocator);
        newM.typeName = TemLangStringCreate(*c, allocator);
    }
end:
    return newM;
}

#define CompileStructMember(isVariant, endOfStructCheck)                       \
    const char* c = NULL;                                                      \
    StructMember newM = TryGetCGLMName(m, &c, allocator);                      \
    m = &newM;                                                                 \
    if (m->isKeyword) {                                                        \
        const CType t = KeywordToCType(m->keyword);                            \
        c = CTypeToTypeString(t);                                              \
        switch (m->quantity) {                                                 \
            case 0: {                                                          \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(cleanups,                    \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      cleanups, "%sListFree(&value->%s);", c, m->name.buffer); \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&cleanups, "break;");         \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(copies,                      \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      copies,                                                  \
                      "%sListCopy(&a->%s,&b->%s, p);if(a->%s.used != "         \
                      "b->%s.used){ return false; }",                          \
                      c,                                                       \
                      m->name.buffer,                                          \
                      m->name.buffer,                                          \
                      m->name.buffer,                                          \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&copies, "break;");           \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(serialization,               \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      serialization,                                           \
                      "total += uint32_tSerialize(&value->%s.used, bytes, "    \
                      "e); for(size_t i = 0; i < value->%s.used; ++i){total "  \
                      "+= %sSerialize(&value->%s.buffer[i],bytes, e);}",       \
                      m->name.buffer,                                          \
                      m->name.buffer,                                          \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&serialization, "break;");    \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(deserialization,             \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      deserialization,                                         \
                      "{if(value->%s.allocator == NULL){ if(bytes->allocator " \
                      "== NULL){ return total; } memset(&value->%s, 0, "       \
                      "sizeof(value->%s)); value->%s.allocator = "             \
                      "bytes->allocator;} uint32_t count = 0UL; total += "     \
                      "uint32_tDeserialize(&count, bytes, offset + total, "    \
                      "e); for(size_t i =0; i < count; ++i) {%s temp = {0}; "  \
                      "const size_t l = %sDeserialize(&temp, bytes, offset + " \
                      "total, e); if(l == 0UL){break;} total += l; "           \
                      "%sListAppend(&value->%s, &temp); %sFree(&temp); }}",    \
                      m->name.buffer,                                          \
                      m->name.buffer,                                          \
                      m->name.buffer,                                          \
                      m->name.buffer,                                          \
                      c,                                                       \
                      c,                                                       \
                      c,                                                       \
                      m->name.buffer,                                          \
                      c);                                                      \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&deserialization, "break;");  \
                    }                                                          \
                }                                                              \
            } break;                                                           \
            case 1: {                                                          \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(cleanups,                    \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      cleanups, "%sFree(&value->%s);", c, m->name.buffer);     \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&cleanups, "break;");         \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(copies,                      \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      copies,                                                  \
                      "if(!%sCopy(&a->%s, &b->%s, p)){return false;}",         \
                      c,                                                       \
                      m->name.buffer,                                          \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&copies, "break;");           \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(serialization,               \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      serialization,                                           \
                      "total += %sSerialize(&value->%s, "                      \
                      "bytes, e);",                                            \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&serialization, "break;");    \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(deserialization,             \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      deserialization,                                         \
                      "total += %sDeserialize(&value->%s, "                    \
                      "bytes, offset + total, e);",                            \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&deserialization, "break;");  \
                    }                                                          \
                }                                                              \
            } break;                                                           \
            default: {                                                         \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(cleanups,                    \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      cleanups,                                                \
                      "for(size_t i = 0; i < %zu; ++i){ "                      \
                      "%sFree(&value->%s[i]);}",                               \
                      m->quantity,                                             \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&cleanups, "break;");         \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(copies,                      \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      copies,                                                  \
                      "for(size_t i = 0; i < %zu; ++i){ "                      \
                      "if(!%sCopy(&a->%s[i], &b->%s[i], "                      \
                      "p)){return false;}}",                                   \
                      m->quantity,                                             \
                      c,                                                       \
                      m->name.buffer,                                          \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&copies, "break;");           \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(serialization,               \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      serialization,                                           \
                      "for(size_t i = 0; i < %zu; ++i){total += "              \
                      "%sSerialize(&value->%s[i],bytes, e);}",                 \
                      m->quantity,                                             \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&serialization, "break;");    \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(deserialization,             \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      deserialization,                                         \
                      "for(size_t i =0; i < %zu; ++i){total "                  \
                      "+= %sDeserialize(&value->%s[i], "                       \
                      "bytes, offset + total, e);}",                           \
                      m->quantity,                                             \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&deserialization, "break;");  \
                    }                                                          \
                }                                                              \
            } break;                                                           \
        }                                                                      \
    } else {                                                                   \
        c = m->typeName.buffer;                                                \
        switch (m->quantity) {                                                 \
            case 0: {                                                          \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(cleanups,                    \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      cleanups, "%sListFree(&value->%s);", c, m->name.buffer); \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&cleanups, "break;");         \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(copies,                      \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      copies,                                                  \
                      "if(!%sListCopy(&a->%s,&b->%s, p)){return false;}",      \
                      c,                                                       \
                      m->name.buffer,                                          \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&copies, "break;");           \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(serialization,               \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      serialization,                                           \
                      "total += uint32_tSerialize(&value->%s.used, bytes, "    \
                      "e);for(size_t i = 0; i < value->%s.used; ++i){total "   \
                      "+= %sSerialize(&value->%s.buffer[i],bytes, e);}",       \
                      m->name.buffer,                                          \
                      m->name.buffer,                                          \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&serialization, "break;");    \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(deserialization,             \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      deserialization,                                         \
                      "{ if(value->%s.allocator == NULL){ "                    \
                      "if(bytes->allocator == NULL) {return total;} "          \
                      "memset(&value->%s, 0, sizeof(value->%s)); "             \
                      "value->%s.allocator=bytes->allocator; } uint32_t "      \
                      "count = 0;total += uint32_tDeserialize(&count, bytes, " \
                      "offset+total,e); for(size_t i =0; i < count; ++i){%s "  \
                      "temp={0};const size_t l= %sDeserialize(&temp,bytes, "   \
                      "offset + total, e); if(l == 0UL) {break;} total += l;"  \
                      "%sListAppend(&value->%s, &temp);%sFree(&temp);}}",      \
                      m->name.buffer,                                          \
                      m->name.buffer,                                          \
                      m->name.buffer,                                          \
                      m->name.buffer,                                          \
                      c,                                                       \
                      c,                                                       \
                      c,                                                       \
                      m->name.buffer,                                          \
                      c);                                                      \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&deserialization, "break;");  \
                    }                                                          \
                }                                                              \
            } break;                                                           \
            case 1: {                                                          \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(cleanups,                    \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      cleanups, "%sFree(&value->%s);", c, m->name.buffer);     \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&cleanups, "break;");         \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(copies,                      \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(copies,                          \
                                              "if(!%sCopy(&a->%s, &b->%s, "    \
                                              "p)){return false;}",            \
                                              c,                               \
                                              m->name.buffer,                  \
                                              m->name.buffer);                 \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&copies, "break;");           \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(serialization,               \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      serialization,                                           \
                      "total += %sSerialize(&value->%s, "                      \
                      "bytes, e);",                                            \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&serialization, "break;");    \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(deserialization,             \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      deserialization,                                         \
                      "total += %sDeserialize(&value->%s, "                    \
                      "bytes, offset + total, e);",                            \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&deserialization, "break;");  \
                    }                                                          \
                }                                                              \
            } break;                                                           \
            default: {                                                         \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(cleanups,                    \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      cleanups,                                                \
                      "for(size_t i = 0; i < %zu; ++i){ "                      \
                      "%sFree(&value->%s[i]);}",                               \
                      m->quantity,                                             \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&cleanups, "break;");         \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(copies,                      \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      copies,                                                  \
                      "for(size_t i = 0; i < %zu; ++i){ "                      \
                      "if(!%sCopy(&a->%s[i], "                                 \
                      "&b->%s[i], p)){return false;}}",                        \
                      m->quantity,                                             \
                      c,                                                       \
                      m->name.buffer,                                          \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&copies, "break;");           \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(serialization,               \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      serialization,                                           \
                      "for(size_t i = 0; i < %zu; ++i){total += "              \
                      "%sSerialize(&value->%s[i],bytes, e);}",                 \
                      m->quantity,                                             \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&serialization, "break;");    \
                    }                                                          \
                }                                                              \
                {                                                              \
                    if (isVariant) {                                           \
                        TemLangStringAppendFormat(deserialization,             \
                                                  "case %sTag_%s:",            \
                                                  atom->name.buffer,           \
                                                  m->name.buffer);             \
                    }                                                          \
                    TemLangStringAppendFormat(                                 \
                      deserialization,                                         \
                      "for(size_t i =0; i < %zu; ++i){total "                  \
                      "+= %sDeserialize(&value->%s[i], "                       \
                      "bytes, offset + total, e);}",                           \
                      m->quantity,                                             \
                      c,                                                       \
                      m->name.buffer);                                         \
                    if (isVariant) {                                           \
                        TemLangStringAppendChars(&deserialization, "break;");  \
                    }                                                          \
                }                                                              \
            } break;                                                           \
        }                                                                      \
    }                                                                          \
    switch (m->quantity) {                                                     \
        case 0: {                                                              \
            TemLangStringAppendFormat(                                         \
              (*output), "%sList %s;", c, m->name.buffer);                     \
        } break;                                                               \
        case 1: {                                                              \
            TemLangStringAppendFormat((*output), "%s %s;", c, m->name.buffer); \
        } break;                                                               \
        default: {                                                             \
            if (useCGLM && m->isKeyword && m->keyword == Keyword_f32) {        \
                switch (m->quantity) {                                         \
                    case 2:                                                    \
                        TemLangStringAppendFormat(                             \
                          (*output), "vec2 %s;", m->name.buffer);              \
                        goto endOfStructCheck;                                 \
                    case 3:                                                    \
                        TemLangStringAppendFormat(                             \
                          (*output), "vec3 %s;", m->name.buffer);              \
                        goto endOfStructCheck;                                 \
                    case 4:                                                    \
                        TemLangStringAppendFormat(                             \
                          (*output), "vec4 %s;", m->name.buffer);              \
                        goto endOfStructCheck;                                 \
                    case 9:                                                    \
                        TemLangStringAppendFormat(                             \
                          (*output), "mat3 %s;", m->name.buffer);              \
                        goto endOfStructCheck;                                 \
                    case 16:                                                   \
                        TemLangStringAppendFormat(                             \
                          (*output), "mat4 %s;", m->name.buffer);              \
                        goto endOfStructCheck;                                 \
                    default:                                                   \
                        break;                                                 \
                }                                                              \
            }                                                                  \
            TemLangStringAppendFormat(                                         \
              (*output), "%s %s[%zu];", c, m->name.buffer, m->quantity);       \
        } break;                                                               \
    }                                                                          \
    StructMemberFree(&newM);