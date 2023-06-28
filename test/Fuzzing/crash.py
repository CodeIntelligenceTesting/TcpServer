import random

# Function to fuzz-test
def process_input(input_string):
    if input_string == "crash":
        # Simulate a crash or error condition
        raise Exception("Crash detected!")
    else:
        # Process the input normally
        print("Processing input:", input_string)

# Fuzz test the process_input function
def fuzz_test():
    while True:
        # Generate a random input
        input_length = random.randint(1, 10)
        input_chars = [chr(random.randint(32, 126)) for _ in range(input_length)]
        input_string = "".join(input_chars)
        
        # Call the process_input function with the generated input
        try:
            process_input(input_string)
        except Exception as e:
            print("Exception:", str(e))
        
# Run the fuzz test
fuzz_test()
