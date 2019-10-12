################################################################################
#
# perl-crypt-openssl-x509
#
################################################################################

PERL_CRYPT_OPENSSL_X509_VERSION = 1.808
PERL_CRYPT_OPENSSL_X509_SOURCE = Crypt-OpenSSL-X509-$(PERL_CRYPT_OPENSSL_X509_VERSION).tar.gz
PERL_CRYPT_OPENSSL_X509_SITE = $(BR2_CPAN_MIRROR)/authors/id/J/JO/JONASBN
PERL_CRYPT_OPENSSL_X509_DEPENDENCIES = perl-crypt-openssl-random
PERL_CRYPT_OPENSSL_X509_LICENSE = Artistic or GPL-1.0+
PERL_CRYPT_OPENSSL_X509_LICENSE_FILES = LICENSE

$(eval $(perl-package))
