 # Define the vulnerable function
def vulnerable_function(user_input):
    if user_input == "admin":
        print("Access granted!")
    else:
        print("Access denied!")

# Fuzzing with AFL
import subprocess

# Define the AFL command
afl_command = ["afl-fuzz", "-i", "input_dir", "-o", "output_dir", "--", "python", "-c",
               "from main import vulnerable_function; import sys; vulnerable_function(sys.stdin.readline().strip())"]

# Start the fuzzing process
subprocess.run(afl_command)
