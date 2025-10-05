import os
import subprocess
import sys

# ------------------------------------------------------------
# Locate project root (same level as this script)
# ------------------------------------------------------------
CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.abspath(os.path.join(CURRENT_DIR))

# Paths to look for the cqlite binary
CQLITE_PATH = os.path.join(ROOT_DIR, "cqlite")
ALT_PATH = os.path.join(ROOT_DIR, "cmake-build-debug", "cqlite")

if os.path.exists(CQLITE_PATH):
    BINARY_PATH = CQLITE_PATH
elif os.path.exists(ALT_PATH):
    BINARY_PATH = ALT_PATH
else:
    raise FileNotFoundError(
        f"❌ Could not find 'cqlite' binary.\n"
        f"Checked:\n  {CQLITE_PATH}\n  {ALT_PATH}\n"
        f"Make sure you built it with `cmake --build .` or CLion."
    )

# ------------------------------------------------------------
# Helper to run commands in cqlite process
# ------------------------------------------------------------
def run_script(commands, args=None):
    """
    Run a list of commands in the cqlite REPL and capture output.
    """
    if args is None:
        args = []

    full_cmd = [BINARY_PATH] + args

    process = subprocess.Popen(
        full_cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    script = "\n".join(commands) + "\n"
    stdout, stderr = process.communicate(script)

    if stderr.strip():
        print(f"⚠️ stderr from {BINARY_PATH}:\n{stderr}")

    return stdout.strip().split("\n")

# ------------------------------------------------------------
# Test utilities
# ------------------------------------------------------------
TEST_DB_PATH = os.path.join(ROOT_DIR, "test.db")

def cleanup_db():
    """Delete test database file if it exists."""
    if os.path.exists(TEST_DB_PATH):
        os.remove(TEST_DB_PATH)
        print("🧹 Removed old test.db")

# ------------------------------------------------------------
# Example test - insert + select
# ------------------------------------------------------------
def test_insert_and_select():
    cleanup_db()

    result = run_script([
        "insert 1 user1 person1@example.com",
        "select",
        ".exit",
    ], args=["test.db"])

    expected = [
        "CQlite.....",
        "This is just a database built to learn....",
        "cqlite > Executed.",
        "cqlite > (1 user1 person1@example.com)",
        "Executed.",
        "cqlite >",
    ]

    assert result == expected, f"\nExpected:\n{expected}\nGot:\n{result}"
    cleanup_db()

# ------------------------------------------------------------
# Insert single row test helper
# ------------------------------------------------------------
def test_insert(data):
    """
    Insert a single row into the test database.
    :param data: tuple or list like (id, username, email)
    """
    cleanup_db()

    id_, username, email = data
    result = run_script([
        f"insert {id_} {username} {email}",
        ".exit",
    ], args=["test.db"])

    expected = [
        "CQlite.....",
        "This is just a database built to learn....",
        "cqlite > Executed.",
        "cqlite >",
    ]

    assert result == expected, f"\nExpected:\n{expected}\nGot:\n{result}"
    cleanup_db()
    print(f"✅ Inserted row ({id_}, {username}, {email}) successfully!")

# ------------------------------------------------------------
# Insert multiple rows with auto data
# ------------------------------------------------------------
def test_bulk_insert(n):
    """
    Insert N rows automatically with increasing IDs and demo data.
    e.g., (1, user1, user1@example.com), (2, user2, user2@example.com)
    """
    cleanup_db()

    commands = []
    for i in range(1, n + 1):
        username = f"user{i}"
        email = f"user{i}@example.com"
        commands.append(f"insert {i} {username} {email}")
    commands.append(".exit")

    result = run_script(commands, args=["test.db"])

    # Verify the first and last inserts at least executed successfully
    assert "Executed." in result[-2], "❌ Bulk insert failed!"
    print(f"✅ Successfully inserted {n} rows into test.db")


# ------------------------------------------------------------
# Main entry
# ------------------------------------------------------------
if __name__ == "__main__":
    print(f"🧩 Using binary: {BINARY_PATH}")

    # Examples
    test_insert_and_select()
    test_insert((2, "user2", "user2@example.com"))
    test_bulk_insert(13)
    # print_btree()
    print("✅ All tests passed successfully!")
