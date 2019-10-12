AUTOMAKE_OPTIONS = foreign 1.9

bin_PROGRAMS=dying_gasp

ACLOCAL_AMFLAGS = -I m4

dying_gasp_CFLAGS = -I${top_builddir}/inc \
           -Ulinux -Dlinux=linux -D_REENTRANT -D_GNU_SOURCE -fno-strict-aliasing -pipe -Wdeclaration-after-statement 
           

dying_gasp_LDADD = \
           -lnetsnmpmibs -lnetsnmpagent -lnetsnmp \
           -lsilc \
           -lpthread\
           -lsilc_mgmtd

dying_gasp_SOURCES= \
         dying_gasp_main.c \
         dying_gasp.c
