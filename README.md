# Windows proof-of-concept keylogger

### **Introduction**
Please note that this project was for educational purposes and should be tested/executed in a secure environment ONLY.   

### **How to Build:**
1. If you would like to compile this malware yourself you can do so by utilising the `Makefile`, this builds the executable using the apropriate command and includes the required compile-time linked libraries.
   * However, you may notice that I have hard-coded the path to my 32-bit mingw gcc compiler and the `-m32` compiler flag is used. I have done this to ensure it is compiled to a 32-bit executable allowing it to run in both 64 and 32-bit Windows environments. As such, you will likely have to replace the compiler path with that of you own 32-bit compiler if you wish to use the make file.
   * Otherwise, simply construct the compiler command to be specific to you environment.
2. Once compiled, you can then pack the executable using upx by simply navigating to the location of the generated `donotexecute.exe` file and executing `upx donotexecute.exe` (if you have upx installed).

### **How to Setup/Test/Run and Interact With the C2 Using LocalHost in a Secure Environment:**
1. Firstly, ensure you have spun up a virtual machine, or are in a secure environment before going any further. 
2. Then, to begin testing open a new command prompt/powershell window and navigate to the `c2` directory and run the Flask server using `py server.py`.
3. Then, open another window and run `controller.py` which is also located in the `c2` directory. This script allows you to send commands to registered clients via the C2 server, view captured key logs, and see all client information.
4. Then, open a third command prompt as admin (this is important) and navigate to the directory that stores the malware executable (called `donotexecute.exe`), and execute it (sorry for the mixed signals).
5. Once running, the server window should recieve an HTTP GET request from the client, registering them with the C2 server. At this point everything is set up and ready to go, to test the various capabiltiies you can follow the below instructions:
   * `slp`, `shd`, and `pwn`: use the `controller` window to send these commands in the specified format to the client machine. If you need help simply enter the `help` command. Once sent the result of these commands should be visible in the malware window, there will likely be a delay before they are executed as the commands are polled at regular intervals of 20 seconds.
   * Listing clients: simply enter the command `clients` to list all clients including their id, how long it has been since their last beacon, and whether they are currently active.
   * Viewing log files: to view captured log files simply enter the command `logs` followed by the client id. If the client id is unknown you can list all clients using the `clients` commands as described earlier.

