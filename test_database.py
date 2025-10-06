import os
import subprocess
import sys
import signal

# ------------------------------------------------------------
# Locate project root (same level as this script)
# ------------------------------------------------------------
CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.abspath(os.path.join(CURRENT_DIR))

CQLITE_PATH = os.path.join(ROOT_DIR, "cqlite")
ALT_PATH = os.path.join(ROOT_DIR, "cmake-build-debug", "cqlite")

if os.path.exists(CQLITE_PATH):
    BINARY_PATH = CQLITE_PATH
elif os.path.exists(ALT_PATH):
    BINARY_PATH = ALT_PATH
else:
    raise FileNotFoundError(f"❌ Could not find 'cqlite' binary at {CQLITE_PATH} or {ALT_PATH}")


# ------------------------------------------------------------
# Helper to run commands in cqlite process
# ------------------------------------------------------------
def run_script(commands, args=None, timeout=5):
    """
    Run a list of commands in the cqlite REPL and capture output safely.
    Prints last few lines if the process crashes.
    """
    if args is None:
        args = []

    full_cmd = [BINARY_PATH] + args
    script = "\n".join(commands) + "\n"

    process = subprocess.Popen(
        full_cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    try:
        stdout, stderr = process.communicate(script, timeout=timeout)
        exit_code = process.returncode
        stdout_lines = stdout.strip().split("\n") if stdout.strip() else []

        if exit_code != 0:
            print("\n💥 Database exited unexpectedly!")
            print("🧩 Last 10 lines of stdout before crash:")
            for line in stdout_lines[-10:]:
                print("   ", line)
            print(f"\n⚠️ stderr:\n{stderr.strip() or '(empty)'}\n")
            raise RuntimeError(f"❌ Database exited with code {exit_code}")

        if stderr.strip():
            print(f"⚠️ stderr from {BINARY_PATH}:\n{stderr.strip()}")

        return stdout_lines

    except subprocess.TimeoutExpired:
        process.kill()
        raise TimeoutError("⏰ Database process timed out.")


# ------------------------------------------------------------
# Test utilities
# ------------------------------------------------------------
TEST_DB_PATH = os.path.join(ROOT_DIR, "test.db")

def cleanup_db():
    if os.path.exists(TEST_DB_PATH):
        os.remove(TEST_DB_PATH)
        print("🧹 Removed old test.db")


# ------------------------------------------------------------
# Bulk insert with detailed tracing
# ------------------------------------------------------------
def test_bulk_insert(n):
    """
    Insert N rows automatically with increasing IDs and demo data.
    Shows progress and captures crash point.
    """
    cleanup_db()
    print(f"🚀 Starting bulk insert of {n} rows...")

    commands = []
    for i in range(1, n + 1):
        username = f"user{i}"
        email = f"user{i}@example.com"
        commands.append(f"insert {i} {username} {email}")

    # Add select + exit for verification
    commands += ["select", ".exit"]

    try:
        result = run_script(commands, args=["test.db"])

        print(f"✅ Successfully inserted {n} rows into test.db")
        # Optional: print last few output lines for inspection
        print("\n📄 Last output lines:")
        for line in result[-8:]:
            print("   ", line)

    except RuntimeError as e:
        print("\n❌ Bulk insert failed.")
        print(f"Crash occurred while inserting up to ID {len(commands)-2}")
        raise


# ------------------------------------------------------------
# Main
# ------------------------------------------------------------
if __name__ == "__main__":
    print(f"🧩 Using binary: {BINARY_PATH}")
    test_bulk_insert(14)
