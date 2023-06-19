import socket

# Connect to the server
server_address = ('localhost', 65123)
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(server_address)

# Define the payload to be sent repeatedly
payload = "A" * 1000  # Adjust the payload length as needed

try:
    # Send the payload repeatedly
    while True:
        client_socket.sendall(payload.encode())
except KeyboardInterrupt:
    # Interrupt the loop on keyboard interrupt
    pass

# Close the connection
client_socket.close()
