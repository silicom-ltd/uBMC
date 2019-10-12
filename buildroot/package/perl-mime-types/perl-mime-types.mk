################################################################################
#
# perl-mime-types
#
################################################################################

PERL_MIME_TYPES_VERSION = 2.17
PERL_MIME_TYPES_SOURCE = MIME-Types-$(PERL_MIME_TYPES_VERSION).tar.gz
PERL_MIME_TYPES_SITE = $(BR2_CPAN_MIRROR)/authors/id/M/MA/MARKOV
PERL_MIME_TYPES_LICENSE = Artistic or GPL-1.0+
PERL_MIME_TYPES_LICENSE_FILES = README

$(eval $(perl-package))
