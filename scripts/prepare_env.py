"""Generate machine-specific editor config (.clangd) for this checkout."""

import os
import sys

PLACEHOLDER = "{{PROJECT_ROOT}}"


def normpath(path):
    return path.replace("\\", "/")


scripts_dir = os.path.dirname(os.path.abspath(__file__))
project_root = normpath(os.path.dirname(scripts_dir))

#
# project_root/.clangd
#
template_file = os.path.join(scripts_dir, "config_files", ".clangd")
output_file = os.path.join(project_root, ".clangd")
with open(template_file, "r", encoding="utf-8") as f:
    content = f.read()
with open(output_file, "w", encoding="utf-8", newline="\n") as f:
    f.write(content.replace(PLACEHOLDER, project_root))
print(f"Generated {normpath(output_file)}")

msimeui_cmake = os.path.join(project_root, "third_party", "msimeui", "CMakeLists.txt")
if not os.path.exists(msimeui_cmake):
    print(
        "warning: third_party/msimeui is empty, run: git submodule update --init --recursive",
        file=sys.stderr,
    )
