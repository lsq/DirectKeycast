import os


def normpath(path):
    return path.replace("\\", "/")


cur_file_path = os.path.dirname(os.path.abspath(__file__))
project_root_path = os.path.dirname(cur_file_path)

Directkeycast_root_path = normpath(project_root_path)

#
# project_root/.clangd
#
dot_clangd_file = os.path.join(
    Directkeycast_root_path, "scripts", "config_files", ".clangd"
)
dot_clangd_output_file = os.path.join(Directkeycast_root_path, ".clangd")
with open(dot_clangd_file, "r", encoding="utf-8") as f:
    lines = f.readlines()
lines[6] = f'    "-I{Directkeycast_root_path}/includes",\n'
with open(dot_clangd_output_file, "w", encoding="utf-8") as f:
    f.writelines(lines)

#
# project_root/tests/CMakeLists.txt
#
CMakeLists_file = os.path.join(
    Directkeycast_root_path, "scripts", "config_files", "CMakeLists.txt"
)
CMakeLists_output_file = os.path.join(Directkeycast_root_path, "CMakeLists.txt")
with open(CMakeLists_file, "r", encoding="utf-8") as f:
    lines = f.readlines()
with open(CMakeLists_output_file, "w", encoding="utf-8") as f:
    f.writelines(lines)
