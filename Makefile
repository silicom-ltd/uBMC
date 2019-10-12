
.DEFAULT_GOAL = all

.PHONY : all prod_set prod_change prod_clean config_diff config_save

prodfile = buildroot/output/.product buildroot/output/.product_cfg

prod_set:
	scripts/__is_prepare_prod.sh

$(prodfile):
	make prod_set

prod_clean:
	rm -rf buildroot/output/.product
	rm -rf buildroot/output/.product_cfg
	rm -rf buildroot/product

prod_change:
	make prod_clean
	make prod_set

config_save:$(prodfile)
	scripts/__is_save_config.sh buildroot/product/configs/$(shell cat buildroot/output/.product_cfg).config

config_diff:$(prodfile)
	diff -uNr buildroot/product/configs/$(shell cat buildroot/output/.product_cfg).config buildroot/.config
	
all:$(prodfile)
	@echo "Building $(shell cat buildroot/output/.product)."
	@echo "Building image $(shell cat buildroot/output/.product_cfg)"
	make -C buildroot

# the below Makefile target is to prevent the % rule to match Makefile, and then to regenerate Makefile
# by calling buildroot makefiles
Makefile:
	;

%:$(prodfile)
	@make $@ -C buildroot

