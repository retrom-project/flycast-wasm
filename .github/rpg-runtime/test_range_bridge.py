"""Exercise the shipped EM_JS read body, including the cache-hit suspension boundary."""
import json
from pathlib import Path
import re
import subprocess
import unittest

BODY = re.search(r"EM_JS\(int, retrom_flycast_range_read_js,.*?\{\n(.*?)\n\}\);",
                 (Path(__file__).parents[2] / ".github/rpg-runtime/bridge.c").read_text(), re.S).group(1)


class RangeBridgeTests(unittest.TestCase):
    def test_cached_read_does_not_suspend_and_async_read_balances_ownership(self):
        script = """
const assert = require('node:assert/strict');
const HEAPU8 = new Uint8Array(32);
let begins = 0, ends = 0, wakes = [], synchronous = true;
const bridge = {begin: () => begins++, end: () => ends++, fail: () => {},
    read: () => Uint8Array.of(7, 8, 9)};
globalThis.RETROM_FLYCAST_RANGE = bridge;
const Asyncify = {handleSleep: callback => callback(n => wakes.push([n, synchronous]))};
const read = new Function('offset', 'length', 'destination', 'Asyncify', 'HEAPU8', BODY);
read(0, 3, 4, Asyncify, HEAPU8);
assert.deepEqual(wakes, [[3, true]]);
assert.deepEqual(Array.from(HEAPU8.slice(4, 7)), [7, 8, 9]);
assert.equal(begins, ends);
bridge.read = () => Promise.resolve(Uint8Array.of(4, 5, 6));
read(0, 3, 8, Asyncify, HEAPU8);
synchronous = false;
setImmediate(() => {
 assert.deepEqual(wakes, [[3, true], [3, false]]);
 assert.equal(begins, ends);
 assert.deepEqual(Array.from(HEAPU8.slice(8, 11)), [4, 5, 6]);
 read(0, 262145, 0, Asyncify, HEAPU8);
 assert.deepEqual(wakes.at(-1), [-1, false]);
 assert.equal(begins, ends);
});
"""
        completed = subprocess.run(["node", "-"], input="const BODY = " + json.dumps(BODY) + ";\n" + script,
                                   text=True, capture_output=True)
        self.assertEqual(completed.returncode, 0, completed.stderr)


if __name__ == "__main__":
    unittest.main()
