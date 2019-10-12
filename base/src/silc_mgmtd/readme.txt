
1. Introduction
Mgmtd Framework is a serials of source codes and scripts for 
eveloping a management system, which can be used to manage a 
network device or something like. 
Mgmtd Framework include Mgmtd, Library, CLI, and some scripts.

2. Create a new mgmtd
a. create a xml file to config the nodes.
b. run silc_mgmtd_inst_code_gen.py in mgmtd/scripts, and follow the guide.
c. add the functions in directory mgmtd/<your mgmtd name>/funcs.

3. Create a new cli
a. create a xml file to config the commands.
b. run silc_cli_inst_code_gen.py in cli/scripts, and follow the guide.
c. add the command functions in directory cli/<your cli name>/cmd_funcs.
d. add the dynamic functions(for dynamic parameters) in directory cli/<your cli name>/dync_funcs.

4. Add the new directorys into the configure.ac and Makefile.am.
configure.ac
mgmtd/Makefile.am
cli/Makefile.am
