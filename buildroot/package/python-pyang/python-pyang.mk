################################################################################
#
# python-pyang
#
################################################################################

PYTHON_PYANG_VERSION = 1.7.4
PYTHON_PYANG_SOURCE = pyang-$(PYTHON_PYANG_VERSION).tar.gz
PYTHON_PYANG_SITE = https://pypi.python.org/packages/source/p/pyang
PYTHON_PYANG_SETUP_TYPE = setuptools
PYTHON_PYANG_LICENSE = BSD
PYTHON_PYANG_LICENSE_FILES = LICENSE

$(eval $(python-package))
