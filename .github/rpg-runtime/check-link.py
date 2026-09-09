"""Fail before packaging if an Emscripten lazy missing-function stub remains."""
from pathlib import Path
import sys
source = Path(sys.argv[1]).read_text()
if "missing function:" in source:
    raise SystemExit("FLYCAST_UNRESOLVED_SYMBOL")
for name in ("HEAPU8", "HEAPU32", "HEAP16", "EJS_Runtime"):
    if name not in source:
        raise SystemExit("FLYCAST_RUNTIME_EXPORT_MISSING:" + name)
