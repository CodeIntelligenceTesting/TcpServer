import socket

# Connect to the server
server_address = ('localhost', 65123)
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(server_address)

# Craft a test case for input validation
# Modify the client IP and message to simulate different scenarios
# Example 1: Valid input
client_ip = '127.0.0.1'
message = 'Hello, server!'
# Example 2: Invalid input (containing a semicolon)
# client_ip = '127.0.0.1;'
# message = 'Malicious payload'
# Example 3: Invalid input (empty client IP)
# client_ip = ''
# message = 'Hello, server!'

# Craft the payload by combining client IP and message
payload = f'{client_ip}\n{message}'

# Send the payload to the server
client_socket.sendall(payload.encode())

# Receive and print the server's response
response = client_socket.recv(1024).decode()
print(response)

# Close the connection
client_socket.close()

