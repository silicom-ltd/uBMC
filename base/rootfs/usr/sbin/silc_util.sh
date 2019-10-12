

attrib_get()
{
    echo $(eval echo \$$(eval echo ${1}_$2))
}

error_quit()
{
    echo -e "\nError: $1" >&2
    exit 1
}

get_product_name()
{
    cat /etc/product.txt 2>/dev/null
}

get_product_sub_name()
{
    cat /etc/product_sub.txt 2>/dev/null
}

get_vendor_name()
{
	/usr/sbin/is_upgrade.sh -o
}

