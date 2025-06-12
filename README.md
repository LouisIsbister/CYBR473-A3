# C473-A3

### Introduction
This repository is used to keep track of my submission for the final CYBR473 (Malware Analysis) assignment, with the goal of this assignment being to develop a proof-of-concept malware. 

### How to Build/Run/Test Using LocalHost in a Secure Environment:
1. If you would like to compile this malware yourself you can do so using my `Makefile`, this builds the executable using the apropriate command and includes the required linked compile-time libraries
   * However, you may notice that I have hard-coded the path to my 32-bit mingw gcc compiler and utilise the `-m32` compiler flag. I have done this to ensure it is compile to a 32-bit executable allowing it to be exectuabel in both 64 and 32-bit Windows environments. As such, you will have to replace the compiler path with you own 32-bit compiler.

### How to Setup and Interact With C2
1. Firstly, open a new command prompt/powershell window and navigate to the `server` directory and run the Flask server using `py server.py`.
2. Then, open another window and run `controller.py` which is also located in the `server` directory. This script allows you to send commands to the C2 server, view captured key logs, and see all client information.
3. Then, open a third command prompt as admin (this is important) and navigate to the directory that stores the malware executable (called `donotexecute.exe`), and execute it (sorry for the mixed signals).
4. Once running the server window should recieve an HTTP GET request from the client, registering them with the C2 server. At this point everything is set up and ready to go, to test the various capabiltiies you can follow the below instructions:
   * `SLP`, `SHD`, and `PWN`: use the `controller` window to send these commands in the specified format to the client machine. If you need help simply enter the `help` command. Once sent the result of these commands should be visible in the malware window, there will likely be a delay before they are executed as the commands are polled at regular intervals of 20 seconds.
   * Listing clients: simply enter the command `clients` to list all clients including their id, how long it has been since their last beacon, and whether they are currently active.
   * Viewing log files: to view captured log files simply enter the command `logs` followed by the client id. If the client id is unknown you can the list all clients using the command described before.


### Defenses Bypassed 
Each number refers to the order at which the defense appear in the assignment brief.
   5) *"Reboot the client machines so that the malware will be forced to shut down."*  
   This will be very doable, check comment on plans document

   11) *"The malware analyst will eventually find the hard-coded key and will be able to decipher the communications of malware with the C2."*  
   I bypassed this defence by generating a random single-byte encoding key for each new client on the server-side. As such, when a client registers with the server the server responds with said key, ensuring that if the analyst discovers the key on one infected machine it cannot be used to decipher communication of other infected machines.   

   12) *"The malware analyst will spoof the C&C server itself to send the shutdown command to all hosts on the network to try and deactivate the malware ("kill-switch")."*
   This defense is bypassed as all commands sent to the malware are encoded using the previously mentioned random encoding key. As such, the spoofed C2 server might send '*shd <user_id>*', however, when this is decoded by the malware it will no longer be a valid command. As such, the spoofed C2 would need to correctly encode each of the shutdown messags for every infected client in order to be able to shut them down.



### Dependencies and Assumptions
The only dependecy for my malware is that when it is first executed it must be done so with administrator privileges.