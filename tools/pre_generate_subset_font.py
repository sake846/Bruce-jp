import os
import subprocess
import sys

Import("env")

proj_dir = env.subst("$PROJECT_DIR")
script = os.path.join(proj_dir, "tools", "generate_subset_font.py")
target_cpp = os.path.join(proj_dir, "lib", "misaki", "subset_font.cpp")

# Only regenerate if target doesn't exist or script/i18n is newer
i18n_path = os.path.join(proj_dir, "include", "i18n.h")
need_gen = False

if not os.path.exists(target_cpp):
    need_gen = True
elif os.path.exists(i18n_path) and os.path.getmtime(i18n_path) > os.path.getmtime(target_cpp):
    need_gen = True
elif os.path.exists(script) and os.path.getmtime(script) > os.path.getmtime(target_cpp):
    need_gen = True

if need_gen:
    print("[SUBSET FONT] Updating Japanese font subset...")
    try:
        # Try running with python from PATH (which has Pillow)
        subprocess.check_call(["python", script])
    except Exception as e:
        print(f"[SUBSET FONT] Warning: Failed to run generate_subset_font.py ({e}). Using existing subset.")

