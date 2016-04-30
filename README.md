# OSproject

This is a multichannel chat developed by Roberto Dessì and Jacopo Carlini.
It is a project for the Operating Systems class at Sapienza University of Rome.
It is written in ANSI-C and has a server application developed for Linux OS and a client application for Windows OS.
Feel free to contact us for any further information at roberto.dessi11@gmail.com or jack1395@gmail.com


##Instruction

To compile and launch in Linux with GNOME:

1. Open terminal in the project directory 

2. `make `

3. `make run `

To compile and launch in MAC OSX:

1. Open the launcher.c file and replace the following line
  `ret = system("open -a Terminal.app ./client_prova");`
  with
  `ret = system("open -a Terminal.app ./client_prova");`

2. Save the file

3. Open terminal in the project directory

4. `make `

5. `make run `
