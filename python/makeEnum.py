import subprocess

headerCode = """
#pragma once
#include <stddef.h>
#include <memory.h>
#include <ctype.h>

typedef enum {enumName} {{
    {enumName}_Invalid = -1,
    {members}
}} {enumName}, *p{enumName};

#define {enumName}Count {memberCount}
#define {enumName}LongestString {longestName}

static const {enumName} {enumName}Members[] = {{ {members} }};

static inline {enumName} {enumName}FromIndex(size_t index){{
    if(index >= {enumName}Count){{
        return {enumName}_Invalid;
    }}
    return {enumName}Members[index];
}}
"""


def makeEnum(name, members):
    with open('include/{0}.h'.format(name), 'w') as f:
        s = headerCode.format(enumName=name, memberCount=len(members),
                              longestName=max(map(lambda x: len(x), members)),
                              members=','.join(map(lambda x: name + "_" + x, members)))
        f.write(s)

        s = "static inline {enumName} {enumName}FromString(const void* c, const size_t size){{ if(size > {enumName}LongestString){{ return {enumName}_Invalid;}}".format(
            enumName=name)
        f.write(s)

        for m in members:
            f.write('if(size == {size} && memcmp("{member}", c, {size}) == 0){{ return {enumName}_{member}; }}'.format(
                enumName=name, member=m, size=len(m)))
        f.write('return {enumName}_Invalid; }}'.format(enumName=name))

        s = """
        static inline {enumName} {enumName}FromCaseInsensitiveString
        (const char* original, const size_t size){{ if(size > {enumName}LongestString){{ return {enumName}_Invalid;}}
        char c[{enumName}LongestString] = {{0}}; 
        for(size_t i =0 ; i < size; ++i){{ c[i] = tolower(original[i]); }}
        """.format(
            enumName=name)
        f.write(s)

        for m in members:
            f.write('if(size == {size} && memcmp("{lowMember}", c, {size}) == 0){{ return {enumName}_{member}; }}'.format(
                enumName=name, member=m, size=len(m), lowMember=str.lower(m)))
        f.write('return {enumName}_Invalid; }}'.format(enumName=name))

        s = "static inline const char* {enumName}ToString(const {enumName} e){{".format(
            enumName=name)
        f.write(s)
        for m in members:
            f.write(
                'if(e == {enumName}_{member}){{ return "{member}"; }}'.format(member=m, enumName=name))
        f.write('return "Invalid"; }')

    subprocess.run(['clang-format', '-i', 'include/{0}.h'.format(name)])
