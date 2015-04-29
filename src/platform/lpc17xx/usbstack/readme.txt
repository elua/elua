USB stack for LPC17xx. Based on the library distributed by Code Red
Technologies, which in turn is based on the LPCUSB project for the
LPC2148, originally written by Bertrik Sikken.

I'm taking the liberty to just edit the code without annotating
edits, even removing Code Red annotations, as git will preserve
all this history for us.

Original README follows.



Code Red Technologies RDB1768 Board - USB Stack Example
=======================================================

The USB stack and associated examples provided with RDB1768 board are
based on the open source LPCUSB stack, originally written for the NXP
LPC214x microcontrollers. The original sources for the USB stack and
and examples can be found at:

  http://sourceforge.net/projects/lpcusb/

with additional information at:

  http://wiki.sikken.nl/index.php?title=LPCUSB

We have tried to keep the changes to the codebase down to a minimum, and
have commented where changes have been made as much as possible
 
Support
-------
This example project is based on the open source  LPCUSB stack.
Code Red can provide support on using this project as supplied to 
users with a valid support contract with us. But if you require 
assistance on the general use of the stack or on extending the 
example, then we would recommend that you raise your questions
to an appropriate community forum, such as....

http://tech.groups.yahoo.com/group/lpc2000/messages

USBstack
--------
This is the main project which builds the LPCUSB stack as a library.
This is then used in building the various RDB1768 USB examples projects.
	
	
 
