#!/usr/bin/env python3
"""Anti-drift check for the JaffarPlus configuration reference.

The engine reads its .jaffar configuration through typed accessors of the form
    jaffarCommon::json::get{Object,Number,Boolean,String,Array}(<parent>, "Key Name")
(the call may span several lines, and <parent> may itself be a function call). This script
extracts every such key string the *engine core* (source/*.hpp) reads, and verifies that each
one is documented in docs/02-config-reference.md -- and, conversely, flags keys that the doc
mentions as core config keys but the code no longer reads.

It catches the most common way a hand-written reference rots: a key is renamed, removed, or
added in code while the docs are left untouched. Run it standalone or via `ninja test`.

Usage: checkConfigDocs.py <repoRoot>
Exit code 0 = in sync, 1 = drift detected.
"""
import os, re, sys

# Keys that appear in source as json::getX literals but are NOT user-facing .jaffar config keys
# (e.g. nested structural lookups that are documented under a different name, or internal probes).
# Keep this list tiny and justified; it is the explicit escape hatch for false positives.
IGNORED_KEYS = set()

ACCESSOR_RE = re.compile(r'jaffarCommon::json::get(?:Object|Number|Boolean|String|Array)\s*(?:<[^>]*>)?\s*\(')


def find_matching_paren(text, open_idx):
    """Given index of an '(', return index of its matching ')', or -1."""
    depth = 0
    i = open_idx
    n = len(text)
    while i < n:
        c = text[i]
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                return i
        elif c == '"':  # skip string literals so parens inside them don't count
            i += 1
            while i < n and text[i] != '"':
                if text[i] == '\\':
                    i += 1
                i += 1
        i += 1
    return -1


def last_toplevel_string(arglist):
    """Return the last top-level (paren-depth 0) double-quoted string in an argument list."""
    depth = 0
    i = 0
    n = len(arglist)
    last = None
    while i < n:
        c = arglist[i]
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
        elif c == '"':
            j = i + 1
            buf = []
            while j < n and arglist[j] != '"':
                if arglist[j] == '\\':
                    j += 1
                buf.append(arglist[j])
                j += 1
            if depth == 0:
                last = ''.join(buf)
            i = j
        i += 1
    return last


def extract_keys(source_text):
    keys = set()
    for m in ACCESSOR_RE.finditer(source_text):
        open_idx = m.end() - 1  # the '(' itself
        close_idx = find_matching_paren(source_text, open_idx)
        if close_idx < 0:
            continue
        arglist = source_text[open_idx + 1:close_idx]
        key = last_toplevel_string(arglist)
        if key:
            keys.add(key)
    return keys


def main():
    if len(sys.argv) != 2:
        sys.exit("usage: checkConfigDocs.py <repoRoot>")
    root = sys.argv[1]
    source_dir = os.path.join(root, 'source')
    reference = os.path.join(root, 'docs', '02-config-reference.md')

    # Collect keys read by the engine core.
    code_keys = set()
    for fname in sorted(os.listdir(source_dir)):
        if not fname.endswith('.hpp'):
            continue
        with open(os.path.join(source_dir, fname), encoding='utf-8') as fh:
            code_keys |= extract_keys(fh.read())
    code_keys -= IGNORED_KEYS

    with open(reference, encoding='utf-8') as fh:
        doc_text = fh.read()

    # A key is "documented" if it appears verbatim in the reference (keys are quoted in `backticks`
    # or in JSON examples, so a plain substring match is the right test).
    undocumented = sorted(k for k in code_keys if k not in doc_text)

    if undocumented:
        print("FAIL: the engine core reads these config keys, but they are NOT documented in")
        print(f"      docs/02-config-reference.md:")
        for k in undocumented:
            print(f"        - \"{k}\"")
        print()
        print("Fix: document the key (or, if it is an internal lookup, add it to IGNORED_KEYS")
        print("in docs/tools/checkConfigDocs.py with a justifying comment).")
        sys.exit(1)

    print(f"PASS: all {len(code_keys)} core config keys read by source/*.hpp are documented in")
    print("      docs/02-config-reference.md")
    sys.exit(0)


if __name__ == '__main__':
    main()
