#!/usr/bin/env python3
"""tools/const_returns.py [header]  (S9-R1, D-034)

Counts the functions declared in matrix.hpp whose return type is a top-level const value (`T const f()`,
`const T f()`, `auto const f()`, `auto f() -> T const`, `T* const f()`); references and pointers to const
(`T const&`, `T const*`) do not count. Route 1, the clang AST: it runs
`clang++ <flags> -fsyntax-only -Xclang -ast-dump=json -Xclang -ast-dump-filter=feng` on a TU that includes the
header, once per configuration in CONFIGS (-std=c++20; -std=c++26 with FENG_MATRIX_CHECKED_ITERATORS and
FENG_MATRIX_PARALLEL, so the __cpp_lib_mdspan / submdspan / expected and checked-iterator blocks are parsed),
walks FunctionDecl / CXXMethodDecl / CXXConversionDecl nodes located in the header (multi-line declarations are
one node), reads the return type from the function type's qualType and unions the hits. Route 2, text: OpenCV is
not installed, so the `#ifdef FENG_MATRIX_OPENCV` regions are scanned as text for a declaration line with a
top-level const before the function name (`T const f(`, `const T f(`) or after a trailing `->`. Prints one
line per hit, `matrix.hpp:<line>: <name>: <return type>`, then `CONST_RETURNS <n>`. Exit 0 when the scan ran
(whatever n is), 2 when clang++ fails. Environment: CHECK_CLANG (default clang++). Stdlib only.
"""
import json
import os
import re
import subprocess
import sys
import tempfile

FUNC_KINDS = {"FunctionDecl", "CXXMethodDecl", "CXXConversionDecl"}
CONFIGS = [["-std=c++20"],
           ["-std=c++26", "-DFENG_MATRIX_CHECKED_ITERATORS", "-DFENG_MATRIX_PARALLEL"]]
GATE = "FENG_MATRIX_OPENCV"
# A declaration line: optional specifiers, then a return type holding a top-level const, then `name(`.
TYPE = r"[A-Za-z_][\w:]*(?:\s*<[^;{}]*>)?(?:\s*\*)*"
SPEC = r"(?:(?:template\s*<[^;{}]*>|\[\[[^\]]*\]\]|static|inline|constexpr|consteval|virtual|friend|explicit)\s+)*"
DECL_CONST = [re.compile(r"^\s*" + SPEC + r"const\s+" + TYPE + r"\s+(?P<name>[A-Za-z_]\w*|operator\s*\S+?)\s*\("),
              re.compile(r"^\s*" + SPEC + TYPE + r"\s+const\s+(?P<name>[A-Za-z_]\w*|operator\s*\S+?)\s*\("),
              re.compile(r"^\s*" + SPEC + r"auto\s+(?P<name>[A-Za-z_]\w*)\s*\(.*\)[^;{]*->\s*[^;{&]*\bconst\s*(?:\{|$)")]


def split_function_type(qual):
    """Return type text of a clang function qualType such as `const T (int) const noexcept` or
    `auto () -> const T`; None when it is not recognisable."""
    depth_angle = depth_paren = 0
    start = -1
    for i, ch in enumerate(qual):
        if ch == "<":
            depth_angle += 1
        elif ch == ">":
            depth_angle -= 1
        elif ch == "(":
            # The parameter list is the first top-level '(' preceded by a space (decltype( is not).
            if depth_angle == 0 and depth_paren == 0 and i > 0 and qual[i - 1] == " ":
                start = i
                break
            depth_paren += 1
        elif ch == ")":
            depth_paren -= 1
    if start < 0:
        return None
    ret = qual[:start].strip()
    depth = 0
    end = -1
    for j in range(start, len(qual)):
        if qual[j] == "(":
            depth += 1
        elif qual[j] == ")":
            depth -= 1
            if depth == 0:
                end = j
                break
    if end >= 0:
        rest = qual[end + 1:]
        arrow = rest.find("->")
        if arrow >= 0:
            ret = rest[arrow + 2:].strip()
    return ret


def top_level_const(ret):
    """True when the type text is const-qualified at top level (not a reference, not a pointer to const)."""
    t = ret.strip()
    # Drop template arguments so `const std::vector<T *> ` and `foo<const T>` read correctly.
    flat, depth = [], 0
    for ch in t:
        if ch in "<(":
            depth += 1
        elif ch in ">)":
            depth -= 1
        elif depth == 0:
            flat.append(ch)
    f = "".join(flat).strip()
    if f.endswith("&"):
        return False
    if f.endswith("const") and (f == "const" or not f[-6].isalnum() and f[-6] != "_"):
        return True  # `T *const`, `T const`
    if "*" in f:
        return False  # pointer whose pointee is const (`const T *`)
    words = f.replace("*", " ").split()
    return "const" in words


class Walker:
    def __init__(self, header):
        self.header = os.path.realpath(header)
        self.file = None
        self.line = None
        self.hits = {}

    def track(self, loc):
        """Clang's JSON omits `file` and `line` when unchanged since the last printed location (document order)."""
        if not isinstance(loc, dict):
            return
        for key in ("spellingLoc", "expansionLoc"):
            if key in loc:
                self.track(loc[key])
        if "file" in loc:
            self.file = loc["file"]
        if "line" in loc:
            self.line = loc["line"]

    def loc_of(self, loc):
        if "expansionLoc" in loc:
            self.track(loc.get("spellingLoc", {}))
            self.track(loc["expansionLoc"])
        else:
            self.track(loc)
        return self.file, self.line

    def walk(self, node):
        if isinstance(node, list):
            for x in node:
                self.walk(x)
            return
        if not isinstance(node, dict):
            return
        kind = node.get("kind")
        here = None
        for key, val in node.items():
            if key == "loc":
                here = self.loc_of(val)
            elif key == "range":
                self.track(val.get("begin", {}))
                self.track(val.get("end", {}))
            elif isinstance(val, (dict, list)) and key != "type":
                self.walk(val)
        if kind in FUNC_KINDS:
            self.check(node, here)

    def check(self, node, here):
        if node.get("isImplicit") or not here or here[0] is None:
            return
        if os.path.realpath(here[0]) != self.header:
            return
        qual = node.get("type", {}).get("qualType", "")
        ret = split_function_type(qual)
        if ret is None or not top_level_const(ret):
            return
        self.hits.setdefault((here[1], node.get("name", "?")), ret)


def gated_regions(lines, macro):
    """1-based line numbers inside `#ifdef macro` / `#if defined( macro )` blocks, nested #if counted."""
    opener = re.compile(r"^\s*#\s*(?:ifdef\s+%s\b|if\s+defined\s*\(?\s*%s\b)" % (macro, macro))
    inside, depth = set(), 0
    for no, text in enumerate(lines, 1):
        if depth == 0:
            if opener.match(text) and "!" not in text:
                depth = 1
            continue
        if re.match(r"^\s*#\s*if", text):
            depth += 1
        elif re.match(r"^\s*#\s*endif", text):
            depth -= 1
            continue
        if depth > 0:
            inside.add(no)
    return inside


def text_scan(header, macro):
    """Hits {(line, name): line text} for const-returning declarations inside the macro's regions."""
    with open(header) as fh:
        lines = fh.read().split("\n")
    hits = {}
    for no in sorted(gated_regions(lines, macro)):
        text = lines[no - 1].split("//")[0]
        if text.rstrip().endswith(";"):
            continue  # a local variable such as `T const x( a );`, not a definition
        for rx in DECL_CONST:
            m = rx.match(text)
            if m:
                hits[(no, m.group("name"))] = text.strip()
                break
    return hits


def ast_scan(clang, header, flags):
    """Hits {(line, name): return type} of one clang AST pass; None when clang fails."""
    with tempfile.TemporaryDirectory() as tmp:
        tu = os.path.join(tmp, "const_returns.cc")
        with open(tu, "w") as fh:
            fh.write('#include "%s"\n' % os.path.realpath(header))
        proc = subprocess.run([clang] + flags + ["-fsyntax-only", "-Xclang", "-ast-dump=json",
                                                 "-Xclang", "-ast-dump-filter=feng", tu],
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr)
        return None
    walker = Walker(header)
    dec = json.JSONDecoder()
    text, pos = proc.stdout, 0
    while True:
        while pos < len(text) and text[pos] != "{":
            pos = text.find("\n", pos)
            if pos < 0:
                pos = len(text)
                break
            pos += 1
        if pos >= len(text):
            break
        obj, pos = dec.raw_decode(text, pos)
        walker.walk(obj)
    return walker.hits


def main():
    root = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
    header = sys.argv[1] if len(sys.argv) > 1 else os.path.join(root, "matrix.hpp")
    clang = os.environ.get("CHECK_CLANG", "clang++")
    hits = {}
    for flags in CONFIGS:
        found = ast_scan(clang, header, flags)
        if found is None:
            print("CONST_RETURNS error: %s %s failed" % (clang, " ".join(flags)))
            return 2
        for key, ret in found.items():
            hits.setdefault(key, ret)
    for key, ret in text_scan(header, GATE).items():
        hits.setdefault(key, ret)
    base = os.path.basename(header)
    for (line, name), ret in sorted(hits.items()):
        print("%s:%s: %s: %s" % (base, line, name, ret))
    print("CONST_RETURNS %d" % len(hits))
    return 0


if __name__ == "__main__":
    sys.exit(main())
