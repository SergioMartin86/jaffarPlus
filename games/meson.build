# Per-game auxiliary tools. Game folders that ship .cpp tool sources provide their own meson.build
# building each tool as an executable, guarded so it only builds for that game's single-game target
# (-Dgame=<stem> with the game's emulator). Meson has no directory glob, so tool-bearing folders are
# listed here explicitly -- add a folder when it gains a .cpp tool.
gameToolDirs = [
  'arkbot/arkanoid',
  'nes/arkanoid2',
  'nes/lunarBall',
  'genesis/shoveit',
]

foreach d : gameToolDirs
  subdir(d)
endforeach
