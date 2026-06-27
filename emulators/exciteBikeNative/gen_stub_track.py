#!/usr/bin/env python3
"""Generate SYNTHETIC, all-flat stub track headers so ExciteBikeNative can build & run WITHOUT the
copyrighted ROM (e.g. in CI). The real ROM-derived track_layout.hpp / track_sections.hpp (from the
exciteBot tools -- see README.md) take precedence: this only writes a header if it is MISSING.

The stub is flat terrain (every tile 0x3B = ground, no section markers), so the engine accelerates on
flat ground and never enters a track section -- exactly what the ROM-free flat-baseline tests exercise.
NOT game data: it contains no extracted ROM content.

Usage: python3 gen_stub_track.py [outDir]   (default: this script's directory)
"""
import os, sys
outDir = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.abspath(__file__))
LANES, COLS = 6, 1376

layout = outDir + '/track_layout.hpp'
if not os.path.exists(layout):
    row = ','.join(['0x3B'] * COLS)
    body = ',\n  '.join('{' + row + '}' for _ in range(LANES))
    open(layout, 'w').write(
        '#pragma once\n// SYNTHETIC flat stub (no ROM content) for ROM-free builds. See gen_stub_track.py.\n'
        '#include <cstdint>\n#include <cstddef>\nnamespace excitebike {\n'
        'static constexpr size_t kTrackLanes = %d;\nstatic constexpr size_t kTrackColumns = %d;\n'
        'static constexpr uint8_t kTrack[kTrackLanes][kTrackColumns] = {\n  %s\n};\n} // namespace excitebike\n'
        % (LANES, COLS, body))
    print('wrote stub', layout)

sections = outDir + '/track_sections.hpp'
if not os.path.exists(sections):
    # 21 sections, each a single 0xFF-style sentinel feat that never fires on flat ground (never reached
    # because no flat tile >= 0x40 ever starts a section). Just enough to compile/link.
    secs = ''.join('static constexpr Feat kSec%d[] = {{255,0x00}};\n' % i for i in range(21))
    ptrs = ','.join('kSec%d' % i for i in range(21))
    lens = ','.join('1' for _ in range(21))
    open(sections, 'w').write(
        '#pragma once\n// SYNTHETIC stub (no ROM content) for ROM-free builds. See gen_stub_track.py.\n'
        '#include <cstdint>\n#include <cstddef>\nnamespace excitebike {\n'
        'struct Feat { uint8_t pos; uint8_t type; };\n%s'
        'static constexpr const Feat* kSections[21] = {%s};\n'
        'static constexpr uint8_t kSectionLen[21] = {%s};\n'
        'static constexpr size_t kNumSections = 21;\n} // namespace excitebike\n'
        % (secs, ptrs, lens))
    print('wrote stub', sections)
