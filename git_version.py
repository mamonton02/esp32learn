Import("env")
from subprocess import check_output

out = check_output("git rev-parse --short HEAD", shell=True)
sha = out.decode().strip()
env.Append(CPPDEFINES=[("GIT_HASH", env.StringifyMacro(sha))])