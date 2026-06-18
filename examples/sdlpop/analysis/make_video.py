#!/usr/bin/env python3
"""Render a JaffarPlus/SDLPoP solution to an animated GIF for visual analysis.

Drives jaffar-player headlessly (SDL offscreen driver) to dump the rendered frames as
BMPs, then assembles them into a GIF with PIL. No ffmpeg/imagemagick needed -- PoP's
limited palette suits GIF well. Use it on a full solution or a short [--start,--end]
window where a trick is being attempted.

Example:
  make_video.py 0300/script.jaffar 0300/solution.sol /tmp/climb.gif --start 840 --end 920 --scale 3
"""
import argparse, subprocess, os, glob, tempfile, shutil, sys
from PIL import Image

DEFAULT_PLAYER = os.path.join(os.path.dirname(__file__), "..", "..", "..", "build-sdlpop", "jaffar-player")

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("config"); ap.add_argument("solution"); ap.add_argument("output")
    ap.add_argument("--player", default=os.path.abspath(DEFAULT_PLAYER))
    ap.add_argument("--start", type=int, default=0, help="first frame (default 0)")
    ap.add_argument("--end", type=int, default=None, help="last frame (default: whole solution)")
    ap.add_argument("--fps", type=float, default=24.0, help="playback rate (default 24)")
    ap.add_argument("--scale", type=int, default=2, help="integer upscale, nearest-neighbour (default 2)")
    ap.add_argument("--workdir", default=None, help="run the player from here (so config/data paths resolve)")
    args = ap.parse_args()

    tmp = tempfile.mkdtemp(prefix="jaffar_frames_")
    try:
        # Only capture the needed window; the player still advances every frame but writes only these.
        steps = ""
        if args.end is not None:
            steps = ",".join(str(s) for s in range(args.start, args.end + 1))
        config_abs, solution_abs = os.path.abspath(args.config), os.path.abspath(args.solution)
        cmd = [args.player, config_abs, solution_abs, "--reproduce", "--unattended",
               "--exitOnEnd", "--screenshotDir", tmp]
        if steps: cmd += ["--screenshotSteps", steps]
        env = dict(os.environ, SDL_VIDEODRIVER="offscreen")
        # Run from the config's directory so its internal relative paths (Root Path, data) resolve.
        cwd = args.workdir or os.path.dirname(config_abs) or "."
        print(f"[make_video] capturing frames -> {tmp}\n[make_video] cwd={cwd}")
        r = subprocess.run(cmd, env=env, cwd=cwd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        if r.returncode != 0:
            sys.stderr.write(r.stderr.decode()[-2000:]); sys.exit(f"player failed ({r.returncode})")

        bmps = sorted(glob.glob(os.path.join(tmp, "step_*.bmp")))
        if args.end is None and args.start > 0:
            bmps = [b for b in bmps if int(os.path.basename(b)[5:11]) >= args.start]
        if not bmps: sys.exit("no frames captured")

        frames = []
        for b in bmps:
            im = Image.open(b).convert("RGB")
            if args.scale != 1:
                im = im.resize((im.width * args.scale, im.height * args.scale), Image.NEAREST)
            frames.append(im)

        dur = max(20, int(round(1000.0 / args.fps)))
        frames[0].save(args.output, save_all=True, append_images=frames[1:], duration=dur, loop=0,
                       optimize=True, disposal=2)
        print(f"[make_video] wrote {args.output}: {len(frames)} frames, {frames[0].size[0]}x{frames[0].size[1]}, {dur}ms/frame")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

if __name__ == "__main__":
    main()
