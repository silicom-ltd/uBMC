#!/usr/bin/env python
# this script is used to monitor user input during boot
# if user input is correct, it will generate a flag to reset system config later
import sys, os, select, crypt

def read_input(f, timeout):
	rrdy, wrdy, xrdy = select.select([f], [], [], timeout)
	if rrdy == []:
		return ""

	input = os.read(f.fileno(), 10).rstrip()
	return input

if __name__ == "__main__":
	shadow_reset = "$6$Uou6dRAibApEg1or$kOsyfsxF1Q.6Az4fvskfd6pAgw2wOXNUx1FmVkJVxPJckKra5q8IwFFCMgiHhJnBLIAtRjVywvgKToTSkB9kl."

	f = open("/dev/ttyS0","r")

	password = read_input(f, 15)
	shadow = crypt.crypt(password, shadow_reset)
	if shadow == shadow_reset:
		print("System recovery options:")
		print("[1] Reset system administrator password to default.")
		print("[2] Reset system configuration to default.")
		print("Please select your choice: ")
		choice = read_input(f, 10)
		print(choice)
		if choice == "1" or choice == "2":
			print("Please wait for the request to be processed.")
			os.system("echo "+choice+" > /etc/boot_action_flag")
		else:
			print("Continue normal startup.")

	f.close()

