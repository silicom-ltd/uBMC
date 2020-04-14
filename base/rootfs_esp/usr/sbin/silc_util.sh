

attrib_get()
{
  echo $(eval echo \$$(eval echo ${1}_$2)) >&2
}

