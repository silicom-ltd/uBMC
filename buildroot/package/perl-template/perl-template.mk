################################################################################
#
# perl-template
#
################################################################################

PERL_TEMPLATE_VERSION = 2.27
PERL_TEMPLATE_SOURCE = Template-Toolkit-$(PERL_TEMPLATE_VERSION).tar.gz
PERL_TEMPLATE_SITE = $(BR2_CPAN_MIRROR)/authors/id/A/AB/ABW
PERL_TEMPLATE_LICENSE = Artistic or GPL-1.0+
PERL_TEMPLATE_LICENSE_FILES = LICENSE
PERL_TEMPLATE_CONF_OPTS += TT_XS_ENABLE=n TT_XS_DEFAULT=n

$(eval $(perl-package))
