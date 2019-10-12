################################################################################
#
# perl-text-template
#
################################################################################

PERL_TEXT_TEMPLATE_VERSION = 1.52
PERL_TEXT_TEMPLATE_SOURCE = Text-Template-$(PERL_TEXT_TEMPLATE_VERSION).tar.gz
PERL_TEXT_TEMPLATE_SITE = $(BR2_CPAN_MIRROR)/authors/id/M/MS/MSCHOUT
PERL_TEXT_TEMPLATE_LICENSE = Artistic or GPL-1.0+
PERL_TEXT_TEMPLATE_LICENSE_FILES = LICENSE

$(eval $(perl-package))
