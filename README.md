# C473-A3

### Introduction
This repository is used to keep track of my submission for the final CYBR473 assignment, with the goal of this assignment being to develop a proof-of-concept "toy" malware. 

### How to Run/Test:
1) Navigate to the `server` directory and execute the `server.py` file.
2) Open a new command prompt and run `controller.py` aslo from `server`.
3) Open another command prompt, go to the `src` directory and run the make file. For me that is `mingW32-make` but yours may be different.
4) Once compiled run the generated executable `.\run.exe`. This should immediately start sending requests to the server, you can go back to your `controller` window and start executing commands to the client!

### How to setup and interact with C2


### Defenses Bypassed 
Each number refers to the order at which the defense appear in the assignment brief.
   5) *"Reboot the client machines so that the malware will be forced to shut down."*  
   This will be very doable, check comment on plans document

   11) *"The malware analyst will eventually find the hard-coded key and will be able to decipher the communications of malware with the C2."*  
   I bypassed this defence by generating a random single-byte encoding key for each new client on the server-side. As such, when a client registers with the server the server responds with said key, ensuring that if the analyst discovers the key on one infected machine it cannot be used to decipher communication of other infected machines.   

   12) *"The malware analyst will spoof the C&C server itself to send the shutdown command to all hosts on the network to try and deactivate the malware ("kill-switch")."*
   This defense is bypassed as all commands sent to the malware are encoded using the previously mentioned random encoding key. As such, the spoofed C2 server might send '*shd <user_id>*', however, when this is decoded by the malware it will no longer be a valid command. As such, the spoofed C2 would need to correctly encode each of the shutdown messags for every infected client in order to be able to shut them down. 