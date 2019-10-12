################################################################################
#
# perl-file-readbackwards
#
################################################################################

PERL_FILE_READBACKWARDS_VERSION = 1.05
PERL_FILE_READBACKWARDS_SOURCE = File-ReadBackwards-$(PERL_FILE_READBACKWARDS_VERSION).tar.gz
PERL_FILE_READBACKWARDS_SITE = $(BR2_CPAN_MIRROR)/authors/id/U/UR/URI
PERL_FILE_READBACKWARDS_LICENSE = Artistic or GPL-1.0+
PERL_FILE_READBACKWARDS_LICENSE_FILES = README

$(eval $(perl-package))
